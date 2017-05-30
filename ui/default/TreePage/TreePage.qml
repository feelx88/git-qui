import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Item {
  Canvas {
    id: canvas
    z: 1
    property real contentY: 0
    anchors.fill: parent
    onPaint: {
      var ctx = getContext("2d");
      ctx.reset();
      for (var x = 0; x < list.model.count; ++x) {
        ctx.fillStyle = Material.accent;
        ctx.strokeStyle = Qt.rgba(0, 0, 0, 0);
        ctx.ellipse(10, 5 + contentY + x * 20, 10, 10);
        if (x > 0) {
          ctx.rect(14, 5 + contentY + x * 20 - 12.5, 2, 15);
        }
        ctx.fill();
      }
      ctx.stroke();
    }
  }

  ListView {
    id: list
    property var listModel: model
    anchors.fill: parent
    currentIndex: -1
    highlightMoveDuration: 250
    model: ListModel {
      Component.onCompleted: {
        var commits = gitManager.logVariant();
        for (var x = 0; x < commits.length; ++x) {
          append({
                   id: commits[x].id,
                   message: commits[x].message,
                   branches: commits[x].branches
                 });
        }
      }
    }
    delegate: HistoryLine {
      branchModel: branches
    }

    onContentYChanged: {
      canvas.contentY = -contentY;
      canvas.requestPaint();
    }
    highlight: Component {
      Rectangle {
        color: Qt.lighter(Material.accent)
      }
    }
  }
}
