import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Item {
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
                   branches: commits[x].branches,
                   parents: commits[x].parents
                 });
        }
        canvas.requestPaint();
      }
    }
    delegate: HistoryLine {
      branchModel: branches
    }

    highlight: Component {
      Rectangle {
        color: Qt.lighter(Material.accent)
      }
    }
    Canvas {
      id: canvas
      z: 1
      height: parent.contentHeight
      width: 50
      y: -parent.contentY
      renderTarget: Canvas.FramebufferObject
      onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        draw(ctx, list.model.get(0), 10, 5);
        ctx.stroke();
      }
      function draw(ctx, commit, x, y) {
        while(commit) {
          ctx.fillStyle = Material.accent;
          ctx.strokeStyle = Qt.rgba(0, 0, 0, 0);
          ctx.ellipse(x, y, 10, 8);
          if (Object.keys(commit.parents).length) {
            ctx.rect(x + 4, y + 6, 2, 15);
          }
          ctx.fill();

          commit = commit.parents[0];
          y += 20;

          for (var pi = 1; pi < Object.keys(commit.parents).length - 1; ++pi) {
            draw(ctx, commit, x +10, y + 10)
          }

        }
      }
    }
  }
}
