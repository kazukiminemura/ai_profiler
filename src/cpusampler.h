#pragma once

#include <QtCore/QObject>
#include <QtCore/QAtomicInteger>
#include <QtCore/QHash>
#include "common/datamodels.h"

class CpuSampler : public QObject {
    Q_OBJECT
public:
    explicit CpuSampler(QObject *parent = nullptr);

public slots:
    void startSampling(quint32 pid);
    void stopSampling();

signals:
    void sampleReady(const CpuSample &sample);
    void error(const QString &message);
    void targetExited();

private:
    QAtomicInteger<bool> m_stop {false};
};
