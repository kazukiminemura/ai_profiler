#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QVector>
#include "../common/datamodels.h"

class HotspotModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        RankRole = Qt::UserRole + 1,
        StackIdRole,
        ThreadIdRole,
        CpuMsRole,
        CpuPercentRole
    };

    explicit HotspotModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    void setFrame(const HotspotFrame &frame);

signals:
    void lastFrameChanged(const HotspotFrame &frame);

private:
    HotspotFrame m_frame;
};
