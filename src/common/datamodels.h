#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QVector>
#include <QtCore/QString>

struct CounterFrame {
    qint64 epochMs = 0;
    double cpuPercent = 0.0;
    double rssMB = 0.0;
    int threadCount = 0;
};

struct CpuSample {
    qint64 epochMs = 0;
    quint32 threadId = 0;
    quint64 cpuDelta100ns = 0;
    quint64 stackId = 0;
};

struct HotspotEntry {
    quint64 stackId = 0;
    quint32 threadId = 0;
    double cpuMs = 0.0;
    double cpuPercent = 0.0;
};

struct HotspotFrame {
    qint64 epochMs = 0;
    QVector<HotspotEntry> entries;
};

Q_DECLARE_METATYPE(CounterFrame)
Q_DECLARE_METATYPE(CpuSample)
Q_DECLARE_METATYPE(HotspotEntry)
Q_DECLARE_METATYPE(HotspotFrame)
