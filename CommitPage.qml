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
    }
  }

   ListModel {
     id: unstagedModel
     Component.onCompleted: {
       var status = gitManager.statusVariant();
       console.log(status);
       for(var x = 0; x < status.length; ++x) {
         unstagedModel.append({
                        value: status[x].path,
                        modified: status[x].modified
                      });
       }
     }
  }

   ListModel {
     id: stagedModel
  }
}
