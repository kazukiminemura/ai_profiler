#include "aggregator.h"

#include <QtCore/QDateTime>
#include <algorithm>

Aggregator::Aggregator(QObject *parent) : QObject(parent) {
    m_timer.setInterval(2000);
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &Aggregator::flush);
}

void Aggregator::start() {
    m_acc.clear();
    m_timer.start();
}

void Aggregator::stop() {
    m_timer.stop();
    m_acc.clear();
}

void Aggregator::onSample(const CpuSample &sample) {
    auto &entry = m_acc[sample.stackId];
    entry.threadId = sample.threadId;
    entry.total100ns += sample.cpuDelta100ns;
}

void Aggregator::flush() {
    HotspotFrame frame;
    frame.epochMs = QDateTime::currentMSecsSinceEpoch();

    quint64 total100ns = 0;
    for (const auto &e : m_acc) total100ns += e.total100ns;

    QVector<HotspotEntry> entries;
    entries.reserve(m_acc.size());
    for (auto it = m_acc.constBegin(); it != m_acc.constEnd(); ++it) {
        const auto &e = it.value();
        HotspotEntry he;
        he.stackId = it.key();
        he.threadId = e.threadId;
        he.cpuMs = double(e.total100ns) / 10000.0;
        he.cpuPercent = total100ns > 0 ? (double(e.total100ns) / double(total100ns)) * 100.0 : 0.0;
        entries.push_back(he);
    }

    std::sort(entries.begin(), entries.end(), [](const HotspotEntry &a, const HotspotEntry &b) {
        return a.cpuMs > b.cpuMs;
    });

    if (entries.size() > m_topN) entries.resize(m_topN);
    frame.entries = entries;

    emit frameReady(frame);
    m_acc.clear();
}
