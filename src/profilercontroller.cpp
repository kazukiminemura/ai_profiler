#include "profilercontroller.h"

#include "countersampler.h"
#include "cpusampler.h"
#include "aggregator.h"

#include <QtCore/QMetaObject>

ProfilerController::ProfilerController(TimelineModel *timeline, HotspotModel *hotspots, QObject *parent)
    : QObject(parent), m_timeline(timeline), m_hotspots(hotspots) {
}

ProfilerController::~ProfilerController() {
    stop();
}

void ProfilerController::start(quint32 pid) {
    if (m_running) return;

    m_timeline->clear();
    m_hotspots->clear();

    m_counterSampler = new CounterSampler();
    m_cpuSampler = new CpuSampler();
    m_aggregator = new Aggregator();

    m_counterSampler->moveToThread(&m_counterThread);
    m_cpuSampler->moveToThread(&m_cpuThread);
    m_aggregator->moveToThread(&m_aggThread);

    connect(&m_counterThread, &QThread::started, [this, pid]() {
        QMetaObject::invokeMethod(m_counterSampler, "startSampling", Qt::QueuedConnection, Q_ARG(quint32, pid));
    });
    connect(&m_cpuThread, &QThread::started, [this, pid]() {
        QMetaObject::invokeMethod(m_cpuSampler, "startSampling", Qt::QueuedConnection, Q_ARG(quint32, pid));
    });
    connect(&m_aggThread, &QThread::started, m_aggregator, &Aggregator::start);

    connect(m_counterSampler, &CounterSampler::frameReady, this, &ProfilerController::onCounterFrame);
    connect(m_counterSampler, &CounterSampler::error, this, &ProfilerController::onWorkerError);
    connect(m_counterSampler, &CounterSampler::targetExited, this, &ProfilerController::onTargetExited);

    connect(m_cpuSampler, &CpuSampler::sampleReady, m_aggregator, &Aggregator::onSample, Qt::QueuedConnection);
    connect(m_cpuSampler, &CpuSampler::error, this, &ProfilerController::onWorkerError);
    connect(m_cpuSampler, &CpuSampler::targetExited, this, &ProfilerController::onTargetExited);

    connect(m_aggregator, &Aggregator::frameReady, this, &ProfilerController::onHotspotFrame);

    m_counterThread.start();
    m_cpuThread.start();
    m_aggThread.start();

    m_running = true;
    setStatus(QStringLiteral("Running (pid=%1)").arg(pid));
    emit runningChanged();
}

void ProfilerController::stop() {
    if (!m_running) return;
    stopWorkers();
    m_running = false;
    setStatus("Stopped");
    emit runningChanged();
}

void ProfilerController::stopWorkers() {
    // stopSampling is thread-safe; call directly so it works even if the worker
    // thread is busy and its event loop is blocked by the sampling loop.
    if (m_counterSampler) m_counterSampler->stopSampling();
    if (m_cpuSampler) m_cpuSampler->stopSampling();
    if (m_aggregator) QMetaObject::invokeMethod(m_aggregator, "stop", Qt::QueuedConnection);

    m_counterThread.quit();
    m_cpuThread.quit();
    m_aggThread.quit();

    m_counterThread.wait();
    m_cpuThread.wait();
    m_aggThread.wait();

    delete m_counterSampler; m_counterSampler = nullptr;
    delete m_cpuSampler; m_cpuSampler = nullptr;
    delete m_aggregator; m_aggregator = nullptr;
}

void ProfilerController::onCounterFrame(const CounterFrame &frame) {
    m_timeline->addFrame(frame);
}

void ProfilerController::onHotspotFrame(const HotspotFrame &frame) {
    m_hotspots->setFrame(frame);
}

void ProfilerController::onWorkerError(const QString &message) {
    emit error(message);
    setStatus("Error: " + message);
}

void ProfilerController::onTargetExited() {
    setStatus("Target exited");
    stop();
}

void ProfilerController::setStatus(const QString &status) {
    if (m_status == status) return;
    m_status = status;
    emit statusChanged();
}
