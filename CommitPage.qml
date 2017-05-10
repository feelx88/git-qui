import QtQuick 2.7
import de.feelx88.GitFile 1.0

CommitForm {
  unstagedArea.delegate: row
  stagedArea.delegate: row
  unstagedArea.model: unstagedModel
  stagedArea.model: stagedModel

  Component {
    id: row
    FileRow {
      onUpdated: init();
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
    stagedModel.clear();
    unstagedModel.clear();
    var status = gitManager.statusVariant();
    for(var x = 0; x < status.length; ++x) {
      var file = status[x],
          model = file.staged ? stagedModel : unstagedModel;
      model.append(file);
    }
  }
}
