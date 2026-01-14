import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string title: ""
    property var value: ""

    radius: 12
    color: "#111722"
    border.color: "#1d2633"

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: root.title
            color: "#8aa1b5"
            font.pixelSize: 12
        }
        Text {
            text: root.value
            color: "#f3f6fb"
            font.pixelSize: 22
            font.weight: Font.DemiBold
        }
    }
}
