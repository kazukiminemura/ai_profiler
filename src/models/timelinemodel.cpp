#include "timelinemodel.h"

TimelineModel::TimelineModel(QObject *parent) : QAbstractListModel(parent) {}

int TimelineModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_frames.size();
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_frames.size()) return {};
    const auto &f = m_frames[index.row()];
    switch (role) {
    case TimeRole: return f.epochMs;
    case CpuRole: return f.cpuPercent;
    case RssRole: return f.rssMB;
    case ThreadsRole: return f.threadCount;
    default: return {};
    }
}

QHash<int, QByteArray> TimelineModel::roleNames() const {
    return {
        {TimeRole, "t"},
        {CpuRole, "cpu"},
        {RssRole, "rss"},
        {ThreadsRole, "threads"}
    };
}

void TimelineModel::clear() {
    beginResetModel();
    m_frames.clear();
    endResetModel();
    emit countChanged();
}

QVariantMap TimelineModel::get(int index) const {
    QVariantMap map;
    if (index < 0 || index >= m_frames.size()) return map;
    const auto &f = m_frames[index];
    map.insert(\"t\", f.epochMs);
    map.insert(\"cpu\", f.cpuPercent);
    map.insert(\"rss\", f.rssMB);
    map.insert(\"threads\", f.threadCount);
    return map;
}

void TimelineModel::addFrame(const CounterFrame &frame) {
    if (m_frames.size() >= m_maxFrames) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_frames.pop_front();
        endRemoveRows();
        emit countChanged();
    }
    const int row = m_frames.size();
    beginInsertRows(QModelIndex(), row, row);
    m_frames.push_back(frame);
    endInsertRows();
    emit countChanged();
    emit lastFrameChanged(frame);
}
