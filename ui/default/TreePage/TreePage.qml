import QtQuick 2.5
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
        ctx.fill();
      }
      ctx.stroke();
    }
  }

  ListView {
    id: list
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
                   message: commits[x].id
                 });
        }
      }
    }
    delegate: Item {
      height: 20
      width: parent.width
        x: 30
      }

      RowLayout {
        x: label.width + label.x
        Text {
          text: id
        }
        Text {
          text: message
        }
        MouseArea {
          anchors.fill: parent
          onClicked: parent.parent.ListView.view.currentIndex = index
        }
      }
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
