import QtQuick 2.7
import QtQuick.Controls.Material 2.1
import Qt.labs.settings 1.0
import de.feelx88.GitFile 1.0
import de.feelx88.GitDiffLine 1.0

CommitForm {
  unstagedArea.delegate: row
  stagedArea.delegate: row
  diffView.delegate: diffRow

  unstagedArea.model: unstagedModel
  stagedArea.model: stagedModel
  diffView.model: diffModel

  diffViewInactive.opacity: (diffModel.count === 0) ? 1 : 0;

  unstagedArea.highlight: selected == unstagedArea ? highlight : null
  stagedArea.highlight: selected == stagedArea ? highlight : null

  property ListView selected: unstagedArea

  Settings {
    id: settings
    category: 'CommitForm'
    property real stagedAreaHeight: stagedArea.height
    property real splitViewLeftWidth: splitViewLeft.width
    property string commitText: commitMessage.text

    Component.objectName: {
      stagedArea.height = settings.stagedAreaHeight;
      splitViewLeft.width = settings.splitViewLeftWidth;
      commitMessage.text = settings.commitText;
    }
  }

  Shortcut {
    sequence: "Ctrl+Return"
    onActivated: commit()
  }

  buttonCommit.onClicked: commit()
  buttonRefresh.onClicked: init();

  Component {
    id: highlight
    Rectangle {
      color: Qt.lighter(Material.accent)
    }
  }

  Component {
    id: row
    FileRow {
      onUpdated: init()
      onClicked: {
        selected = listView;
        loadDiff(path, listView == stagedArea);
      }
    }
  }

  Component {
    id: diffRow
    DiffRow {
      onUpdated: init()
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

  Component.onCompleted: init()

  function init() {
    unstagedModel.clear();
    stagedModel.clear();
    var status = gitManager.statusVariant();
    for(var x = 0; x < status.length; ++x) {
      var file = status[x];
      if (file.unstaged) {
        file.buttonLabel = '+';
        unstagedModel.append(file);
      }
      if (file.staged) {
        file.buttonLabel = '-';
        stagedModel.append(file);
      }
    }
    unstagedArea.focus = true;

    var path = '';
    var staged = false;
    if (unstagedModel.count > 0) {
      path = unstagedModel.get(0).path;
    } else if (unstagedModel.count > 0) {
      path = stagedModel.get(0).path;
      staged = true;
    }

    loadDiff(path, staged);
  }

  function loadDiff(path, staged) {
    diffModel.clear();
    var diff = gitManager.diffPathVariant(path, staged);
    for(var x = 0; x < diff.length; ++x) {
      if (diff[x].type != GitDiffLine.FILE_HEADER) {
        diff[x].staged = staged;
        diffModel.append(diff[x]);
      }
    }
  }

  function commit() {
    if (stagedModel.count === 0) {
      errorDialog.title = 'Nothing has been staged yet';
      errorDialog.visible = true;
      return;
    }

    gitManager.commit(commitMessage.text);
    commitMessage.text = '';
    init();
  }
}
