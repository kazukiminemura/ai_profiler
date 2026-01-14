import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "components"

Item {
    id: root

    property double cpuValue: 0
    property double rssValue: 0
    property double gpuValue: 0
    property int threadsValue: 0
    property double startEpochMs: 0
    property string elapsedText: "0:00"

    function formatElapsed(seconds) {
        var s = Math.max(0, Math.floor(seconds))
        var h = Math.floor(s / 3600)
        var m = Math.floor((s % 3600) / 60)
        var sec = s % 60
        function pad(v) { return (v < 10 ? "0" : "") + v }
        if (h > 0) {
            return h + ":" + pad(m) + ":" + pad(sec)
        }
        return m + ":" + pad(sec)
    }

    Connections {
        target: timelineModel
        function onLastFrameChanged(frame) {
            if (!frame || frame.cpu === undefined) {
                return
            }
            cpuValue = frame.cpu
            rssValue = frame.rss
            gpuValue = frame.gpu
            threadsValue = frame.threads
            if (startEpochMs === 0) {
                startEpochMs = frame.t
            }
            elapsedText = formatElapsed((frame.t - startEpochMs) / 1000.0)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 20
                    spacing: 12

                    Rectangle { width: 10; height: 10; radius: 3; color: "#3ddc97" }
                    Text { text: "CPU"; color: "#8aa1b5"; font.pixelSize: 12 }
                    Rectangle { width: 10; height: 10; radius: 3; color: "#f6c453" }
                    Text { text: "GPU"; color: "#8aa1b5"; font.pixelSize: 12 }
                    Rectangle { width: 10; height: 10; radius: 3; color: "#58a6ff" }
                    Text { text: "RSS"; color: "#8aa1b5"; font.pixelSize: 12 }
                    Item { Layout.fillWidth: true }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: "#0b111a"
                    border.color: "#1d2633"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "CPU %"; color: "#8aa1b5"; font.pixelSize: 12 }
                            Item { Layout.fillWidth: true }
                            Text { text: cpuValue.toFixed(1); color: "#e8eef7"; font.pixelSize: 12 }
                        }
                        TimelineGraph {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            role: "cpu"
                            lineColor: "#3ddc97"
                            maxValue: 100
                            model: timelineModel
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: "#0b111a"
                    border.color: "#1d2633"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "GPU %"; color: "#8aa1b5"; font.pixelSize: 12 }
                            Item { Layout.fillWidth: true }
                            Text { text: gpuValue.toFixed(1); color: "#e8eef7"; font.pixelSize: 12 }
                        }
                        TimelineGraph {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            role: "gpu"
                            lineColor: "#f6c453"
                            maxValue: 100
                            model: timelineModel
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: "#0b111a"
                    border.color: "#1d2633"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "RSS MB"; color: "#8aa1b5"; font.pixelSize: 12 }
                            Item { Layout.fillWidth: true }
                            Text { text: rssValue.toFixed(1); color: "#e8eef7"; font.pixelSize: 12 }
                        }
                        TimelineGraph {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            role: "rss"
                            lineColor: "#58a6ff"
                            maxValue: 0
                            model: timelineModel
                        }
                    }
                }
            }
        }
    }
}
