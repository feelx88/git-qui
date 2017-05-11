import QtQuick 2.7
import de.feelx88.GitFile 1.0
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
        textEdit.text = '';
        var diff = gitManager.diffPath(path);
        for(var x = 0; x < diff.length; ++x) {
          textEdit.text += diff[x];
        }
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
  }
}
