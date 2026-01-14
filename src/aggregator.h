#pragma once

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include "common/datamodels.h"

class Aggregator : public QObject {
    Q_OBJECT
public:
    explicit Aggregator(QObject *parent = nullptr);

public slots:
    void start();
    void stop();
    void onSample(const CpuSample &sample);

signals:
    void frameReady(const HotspotFrame &frame);

private slots:
    void flush();

private:
    struct AggEntry {
        quint32 threadId = 0;
        quint64 total100ns = 0;
    };

    QHash<quint64, AggEntry> m_acc;
    QTimer m_timer;
    int m_topN = 10;
};
