import QtQuick 2.7

CommitForm {
  unstagedArea.delegate: row
  stagedArea.delegate: row
  unstagedArea.model: model
  stagedArea.model: model

  Component {
    id: row
    FileRow {
    }
  }

   ListModel {
     id: model
     ListElement { value: 'a' }
     ListElement { value: 'b' }
  }
}
