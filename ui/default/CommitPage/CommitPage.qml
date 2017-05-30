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
  unstagedArea.highlightFollowsCurrentItem: true
  stagedArea.highlightFollowsCurrentItem: true

  property ListView selected: unstagedArea
  property int selectedIndex: 0

  Settings {
    id: settings
    category: 'CommitForm'
    property real stagedAreaHeight: stagedContainer.height
    property real splitViewLeftWidth: splitViewLeft.width
    property string commitText: commitMessage.text

    Component.objectName: {
      stagedContainer.height = settings.stagedAreaHeight;
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
  buttonPush.onClicked: {
    progressDialog.open();
    gitManager.push(gitManager.headName(), 'origin', gitManager.headName());
  }

  buttonStageLines.onClicked: {
    var lines = [];
    for (var x = 0; x < diffModel.count; ++x) {
      var diff = diffModel.get(x);
      if (diff.selected) {
        lines.push({
                     header: diff.header,
                     content: diff.content,
                     type: diff.type,
                     oldLine: diff.oldLine,
                     newLine: diff.newLine,
                     index: diff.index
                  });
      }
    }

    if (lines.length > 0) {
      gitManager.stageLinesVariant(lines, selected == stagedArea);
      init();
    }
  }

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
        selectedIndex = index
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

  Connections  {
    target: gitManager
    onRepositoryChanged: init()
  }

  Connections {
    target: watcher
    onFileChanged: init()
    onDirectoryChanged: init()
  }

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

    if (selected.model.count == 0) {
      diffModel.clear();
      return;
    } if (selectedIndex >= selected.model.count) {
      selectedIndex = 0;
    }

    selected.currentIndex = selectedIndex;

    loadDiff(selected.model.get(selectedIndex).path, selected == stagedArea);
  }

  function loadDiff(path, staged) {
    diffModel.clear();
    var diff = gitManager.diffPathVariant(path, staged);
    for(var x = 0; x < diff.length; ++x) {
      if (diff[x].type != GitDiffLine.FILE_HEADER) {
        diff[x].staged = staged;
        diff[x].selected = false;
        diffModel.append(diff[x]);
      }
    }
  }

  function commit() {
    if (stagedModel.count === 0) {
      errorDialog.title = 'Nothing has been staged yet';
      errorDialog.open();
      return;
    }

    gitManager.commit(commitMessage.text);
    commitMessage.text = '';
    init();
  }
}
