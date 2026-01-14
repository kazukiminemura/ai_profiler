#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QVector>
#include <QtCore/QVariantMap>
#include "../common/datamodels.h"

class TimelineModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        CpuRole,
        RssRole,
        ThreadsRole
    };

    explicit TimelineModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap get(int index) const;
    void addFrame(const CounterFrame &frame);

    const QVector<CounterFrame> &frames() const { return m_frames; }

signals:
    void lastFrameChanged(const CounterFrame &frame);
    void countChanged();

private:
    QVector<CounterFrame> m_frames;
    int m_maxFrames = 300;
};
