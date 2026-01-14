import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "components"

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "Hotspots (Top N)"

            ListView {
                anchors.fill: parent
                anchors.margins: 12
                model: hotspotModel
                clip: true
                delegate: Rectangle {
                    width: ListView.view ? ListView.view.width : 0
                    height: 40
                    radius: 6
                    color: index % 2 === 0 ? "#0f1622" : "#101b2a"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12

                        Text { text: "#" + rank; color: "#e8eef7"; font.pixelSize: 12; width: 40 }
                        Text { text: "Thread " + threadId; color: "#9cb1c7"; font.pixelSize: 12; width: 120 }
                        Text { text: "StackId " + stackId; color: "#7f96ac"; font.pixelSize: 12; width: 180 }
                        Text { text: cpuMs.toFixed(2) + " ms"; color: "#c2d2e1"; font.pixelSize: 12; width: 100 }
                        Item { Layout.fillWidth: true }
                        Text { text: cpuPct.toFixed(1) + "%"; color: "#e8eef7"; font.pixelSize: 12 }
                    }
                }
            }
        }
    }
}
