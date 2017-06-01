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
      x: canvas.width
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
      width: 500
      y: -parent.contentY
      renderTarget: Canvas.FramebufferObject
      onPaint: {
        var ctx = getContext("2d");
        ctx.reset();

        var index = 0,
            commits = [{
                         x: 10,
                         commit: list.model.get(index)
                       }],
            y = 5,
            w = 0;

        while(index < list.model.count) {
          var newCommits = [];
          for (var cx = 0; cx < commits.length; ++cx) {
            var commit = commits[cx].commit,
                x = commits[cx].x;

            var currentCommit = false;

            if (commit.id === list.model.get(index).id) {
              currentCommit = true;
              ctx.beginPath();
              if (Object.keys(commit.parents).length > 1) {
                ctx.fillStyle = '#00f';
              } else {
                ctx.fillStyle = Material.accent;
              }

              ctx.strokeStyle = Qt.rgba(0, 0, 0, 0);
              ctx.ellipse(x, y, 10, 10);
              ctx.fill();
            } else {
              newCommits.push({
                                x: x,
                                commit: commit
                              });
            }

            ctx.beginPath();
            ctx.fillStyle = Qt.rgba(0, 0, 0, 0);
            ctx.strokeStyle = Material.accent;
            ctx.lineWidth = 2;
            for (var pi = 0; pi < Object.keys(commit.parents).length; ++pi) {

              ctx.moveTo(x + 5, y + 5);
              ctx.bezierCurveTo(x + 5, y + 25,
                                x + 5 + 20 * pi, y + 5,
                                x + 5 + 20 * pi, y + 25);
              if (currentCommit) {
                newCommits.push({
                                  x: x + 20 * pi,
                                  commit: commit.parents[pi]
                                });
              }
            }
            ctx.stroke();

            w = Math.max(w, x + 10);
          }
          ++index;
          y += 20;
          commits = [];
          for (var newCommitsIndex = 0; newCommitsIndex < newCommits.length; ++newCommitsIndex) {
            var found = false;
            for (var commitsIndex = 0; commitsIndex < commits.length; ++commitsIndex) {
              if (commits[commitsIndex].commit.id === newCommits[newCommitsIndex].commit.id) {
                found = true;
                break;
              }
            }
            if (!found) {
              commits.push(newCommits[newCommitsIndex]);
            }
          }
        }
          width = w + 20;
      }
    }
  }
}
