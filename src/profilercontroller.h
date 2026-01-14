#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>

#include "models/timelinemodel.h"
#include "models/hotspotmodel.h"

class CounterSampler;
class CpuSampler;
class Aggregator;

class ProfilerController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit ProfilerController(TimelineModel *timeline, HotspotModel *hotspots, QObject *parent = nullptr);
    ~ProfilerController();

    Q_INVOKABLE void start(quint32 pid);
    Q_INVOKABLE void stop();

    bool running() const { return m_running; }
    QString status() const { return m_status; }

signals:
    void runningChanged();
    void statusChanged();
    void error(const QString &message);

private slots:
    void onCounterFrame(const CounterFrame &frame);
    void onHotspotFrame(const HotspotFrame &frame);
    void onWorkerError(const QString &message);
    void onTargetExited();

private:
    void setStatus(const QString &status);
    void stopWorkers();

    TimelineModel *m_timeline = nullptr;
    HotspotModel *m_hotspots = nullptr;

    QThread m_counterThread;
    QThread m_cpuThread;
    QThread m_aggThread;

    CounterSampler *m_counterSampler = nullptr;
    CpuSampler *m_cpuSampler = nullptr;
    Aggregator *m_aggregator = nullptr;

    bool m_running = false;
    QString m_status = "Idle";
};
