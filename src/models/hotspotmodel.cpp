#include "hotspotmodel.h"

HotspotModel::HotspotModel(QObject *parent) : QAbstractListModel(parent) {}

int HotspotModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_frame.entries.size();
}

QVariant HotspotModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_frame.entries.size()) return {};
    const auto &e = m_frame.entries[index.row()];
    switch (role) {
    case RankRole: return index.row() + 1;
    case StackIdRole: return static_cast<qulonglong>(e.stackId);
    case ThreadIdRole: return static_cast<qulonglong>(e.threadId);
    case CpuMsRole: return e.cpuMs;
    case CpuPercentRole: return e.cpuPercent;
    default: return {};
    }
}

QHash<int, QByteArray> HotspotModel::roleNames() const {
    return {
        {RankRole, "rank"},
        {StackIdRole, "stackId"},
        {ThreadIdRole, "threadId"},
        {CpuMsRole, "cpuMs"},
        {CpuPercentRole, "cpuPct"}
    };
}

void HotspotModel::clear() {
    beginResetModel();
    m_frame.entries.clear();
    endResetModel();
}

void HotspotModel::setFrame(const HotspotFrame &frame) {
    beginResetModel();
    m_frame = frame;
    endResetModel();
    emit lastFrameChanged(frame);
}
