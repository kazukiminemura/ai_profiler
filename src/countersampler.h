#pragma once

#include <QtCore/QObject>
#include <QtCore/QAtomicInteger>
#include <memory>
#include "common/datamodels.h"

struct GpuQuery;

class CounterSampler : public QObject {
    Q_OBJECT
public:
    explicit CounterSampler(QObject *parent = nullptr);
    ~CounterSampler() override;

public slots:
    void startSampling(quint32 pid);
    void stopSampling();

signals:
    void frameReady(const CounterFrame &frame);
    void error(const QString &message);
    void targetExited();

private:
    QAtomicInteger<bool> m_stop {false};
    std::unique_ptr<GpuQuery> m_gpu;
};
