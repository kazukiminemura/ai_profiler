import QtQuick 2.15

Canvas {
    id: canvas
    property var model
    property string role: "cpu"
    property color lineColor: "#3ddc97"
    property color gridColor: "#1d2633"
    property real maxValue: 0
    property bool showGrid: true

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        ctx.clearRect(0, 0, width, height)
        ctx.fillStyle = "#0b111a"
        ctx.fillRect(0, 0, width, height)

        var count = model ? model.count : 0
        if (count <= 1) return

        if (showGrid) {
            ctx.strokeStyle = gridColor
            ctx.lineWidth = 1
            for (var i = 1; i <= 4; i++) {
                var y = (height / 5) * i
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
                ctx.stroke()
            }
        }

        var maxVal = maxValue
        if (role === "rss" && maxVal <= 0) {
            maxVal = 1
            for (var j = 0; j < count; j++) {
                var item = model.get(j)
                if (item.rss > maxVal) maxVal = item.rss
            }
        } else if (maxVal <= 0) {
            maxVal = 100
        }

        ctx.beginPath()
        for (var k = 0; k < count; k++) {
            var it = model.get(k)
            var v = (role === "cpu") ? it.cpu : (role === "gpu") ? it.gpu : it.rss
            var x = (k / (count - 1)) * width
            var y = height - (v / maxVal) * height
            if (k === 0) ctx.moveTo(x, y)
            else ctx.lineTo(x, y)
        }
        ctx.lineTo(width, height)
        ctx.lineTo(0, height)
        ctx.closePath()
        ctx.fillStyle = lineColor
        ctx.globalAlpha = 0.25
        ctx.fill()
        ctx.globalAlpha = 1.0
        ctx.strokeStyle = lineColor
        ctx.lineWidth = 2
        ctx.stroke()
    }

    Connections {
        target: model
        function onRowsInserted() { canvas.requestPaint() }
        function onModelReset() { canvas.requestPaint() }
    }

    Component.onCompleted: requestPaint()
}
