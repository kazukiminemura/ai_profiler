import QtQuick 2.15

Canvas {
    id: canvas
    property var model
    property color cpuColor: "#3ddc97"
    property color rssColor: "#58a6ff"
    property color gridColor: "#1d2633"

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        ctx.clearRect(0, 0, width, height)
        ctx.fillStyle = "#0b111a"
        ctx.fillRect(0, 0, width, height)

        var count = model ? model.count : 0
        if (count <= 1) return

        // grid
        ctx.strokeStyle = gridColor
        ctx.lineWidth = 1
        for (var i = 1; i <= 4; i++) {
            var y = (height / 5) * i
            ctx.beginPath()
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
            ctx.stroke()
        }

        var maxRss = 1
        for (var j = 0; j < count; j++) {
            var item = model.get(j)
            if (item.rss > maxRss) maxRss = item.rss
        }

        function drawLine(role, maxValue, color) {
            ctx.strokeStyle = color
            ctx.lineWidth = 2
            ctx.beginPath()
            for (var i = 0; i < count; i++) {
                var it = model.get(i)
                var v = (role === "cpu") ? it.cpu : it.rss
                var x = (i / (count - 1)) * width
                var y = height - (v / maxValue) * height
                if (i === 0) ctx.moveTo(x, y)
                else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }

        drawLine("cpu", 100, cpuColor)
        drawLine("rss", maxRss, rssColor)
    }

    Connections {
        target: model
        function onRowsInserted() { canvas.requestPaint() }
        function onModelReset() { canvas.requestPaint() }
    }

    Component.onCompleted: requestPaint()
}
