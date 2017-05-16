import QtQuick 2.7
import de.feelx88.GitFile 1.0
import de.feelx88.GitDiffLine 1.0
import QtQuick.Controls.Material 2.1

CommitForm {
  unstagedArea.delegate: row
  stagedArea.delegate: row
  diffView.delegate: diffRow

  unstagedArea.model: unstagedModel
  stagedArea.model: stagedModel
  diffView.model: diffModel

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

  Component {
    id: diffRow
    DiffRow {
    }
  }

  ListModel {
     id: unstagedModel
  }

  ListModel {
     id: stagedModel
  }

  ListModel {
    id: diffModel
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
    diffModel.clear();
    var diff = gitManager.diffPathVariant(path);
    for(var x = 0; x < diff.length; ++x) {
      diffModel.append(diff[x]);
    }
  }
}
