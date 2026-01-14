import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 1200
    height: 720
    visible: true
    title: "AI Profiler"
    color: "#0c0f14"

    property string statusText: profilerController.status

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            height: 64
            radius: 12
            color: "#111722"
            border.color: "#1d2633"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    text: "AI Profiler"
                    color: "#e8eef7"
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Rectangle {
                    width: 1
                    height: 28
                    color: "#263241"
                    Layout.leftMargin: 8
                    Layout.rightMargin: 8
                }

                TextField {
                    id: pidField
                    placeholderText: "PID"
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: IntValidator { bottom: 1; top: 2147483647 }
                    width: 120
                    color: "#e8eef7"
                    placeholderTextColor: "#58667a"
                    background: Rectangle { color: "#0b111a"; radius: 6; border.color: "#263241" }
                }

                Button {
                    text: profilerController.running ? "Stop" : "Start"
                    onClicked: {
                        if (profilerController.running) {
                            profilerController.stop()
                        } else if (pidField.text.length > 0) {
                            profilerController.start(parseInt(pidField.text))
                        }
                    }
                    background: Rectangle { color: profilerController.running ? "#7a1f2b" : "#1c6b45"; radius: 6 }
                    contentItem: Text {
                        text: parent.text
                        color: "#f3f6fb"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: statusText
                    color: "#8aa1b5"
                    font.pixelSize: 12
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            background: Rectangle { color: "#0c0f14" }

            TabButton { text: "Overview" }
            TabButton { text: "Hotspots" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            OverviewPage { }
            HotspotsPage { }
        }
    }

    Connections {
        target: profilerController
        function onError(message) {
            statusText = "Error: " + message
        }
    }
}
