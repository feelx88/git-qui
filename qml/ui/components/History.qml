import QtQuick 2.0
import QtQuick.Controls 1.4

Item {
  id: container
  anchors.fill: parent
  Component.onCompleted: gitInterface.log()

  property var _logs: []

  Connections {
    target: gitInterface
    onLogChanged: {
      _logs = logs;
      canvas.requestPaint();
    }
  }

  ScrollView {
    anchors.fill: parent
    Canvas {
      id: canvas
      width: container.width
      height: _logs.length * 20
      renderStrategy: Canvas.Threaded
      onPaint: {
        var y = 15,
            branches = [_logs[0]],
            ctx = getContext("2d");
        ctx.reset();
        ctx.fillStyle = Qt.rgba(0, 0, 1, 1);

        for (var index in _logs) {
          var x = branches.length * 15;
          ctx.beginPath();
          ctx.ellipse(x - 10, y - 10, 10, 10);
          ctx.text(_logs[index].message, x + 15, y);
          ctx.fill();

          if (_logs[index].parents.length === 2) {
            branches.push({});
          }
          branches.splice(1, _logs[index].children.length - 1);

          y += 20;
        }
      }
    }
  }
}
