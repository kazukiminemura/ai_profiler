import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "components"

Item {
    id: root

    property double cpuValue: 0
    property double rssValue: 0
    property int threadsValue: 0

    Connections {
        target: timelineModel
        function onLastFrameChanged(frame) {
            cpuValue = frame.cpuPercent
            rssValue = frame.rssMB
            threadsValue = frame.threadCount
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Card {
                Layout.fillWidth: true
                title: "CPU %"
                value: cpuValue.toFixed(1)
            }
            Card {
                Layout.fillWidth: true
                title: "RSS MB"
                value: rssValue.toFixed(1)
            }
            Card {
                Layout.fillWidth: true
                title: "Threads"
                value: threadsValue
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "Timeline (1s buckets)"

            TimelineGraph {
                anchors.fill: parent
                anchors.margins: 12
                model: timelineModel
            }
        }
    }
}
