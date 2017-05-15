import QtQuick 2.7
import de.feelx88.GitFile 1.0
import de.feelx88.GitDiffLine 1.0
import QtQuick.Controls.Material 2.1

CommitForm {
  unstagedArea.delegate: row
  stagedArea.delegate: row

  unstagedArea.model: unstagedModel
  stagedArea.model: stagedModel

  unstagedArea.highlight: selected == unstagedArea ? highlight : null
  stagedArea.highlight: selected == stagedArea ? highlight : null

  property ListView selected: unstagedArea

  Component {
    id: highlight
    Rectangle {
      color: Qt.lighter(Material.accent)
    }
  }

  Component {
    id: row
    FileRow {
      onUpdated: init();
      onClicked: {
        selected = listView;
        loadDiff(path);
      }
    }
  }

   ListModel {
     id: unstagedModel
  }

   ListModel {
     id: stagedModel
  }

  Component.onCompleted: init();

  function init() {
    unstagedModel.clear();
    stagedModel.clear();
    var status = gitManager.statusVariant();
    for(var x = 0; x < status.length; ++x) {
      var file = status[x],
          model = file.staged ? stagedModel : unstagedModel;
      model.append(file);
    }
    unstagedArea.focus = true;

    var path = '';
    if (unstagedModel.count > 0) {
      path = unstagedModel.get(0).path;
    } else if (unstagedModel.count > 0) {
      path = stagedModel.get(0).path;
    }

    loadDiff(path);
  }

  function loadDiff(path) {
    diffView.text = '';
    var diff = gitManager.diffPathVariant(path);
    for(var x = 0; x < diff.length; ++x) {
      var color = '#000',
          curDiff = diff[x];
      if (curDiff.type == GitDiffLine.REMOVE) {
        color = '#a00';
        curDiff.content = '-' + curDiff.content;
      } else if (curDiff.type == GitDiffLine.ADD) {
        color = '#0a0';
        curDiff.content = '+' + curDiff.content;
      } else if(curDiff.type == GitDiffLine.HEADER || curDiff.type == GitDiffLine.FILE_HEADER) {
        color = '#aaa';
      }

      diffView.text += '<div style="color: ' + color + '">' + curDiff.content + '</div>';
    }
  }
}
