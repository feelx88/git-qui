import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

import "CommitPage"
import "TreePage"

ApplicationWindow {
  id: mainWindow
  visible: true
  width: 800
  height: 600
  title: qsTr("Git QUI")

  Settings {
    category: 'mainWindow'
    property alias x: mainWindow.x
    property alias y: mainWindow.y
    property alias width: mainWindow.width
    property alias height: mainWindow.height
  }

  Connections {
    target: gitManager
    onGitError: {
      errorDialog.text = message;
      errorDialog.visible = true;
    }
  }

  MessageDialog {
      id: messageDialogNothingStaged
      standardButtons: StandardButton.Ok
      title: 'Warning'
      text: 'Nothing added to staging area yet.'
  }

  MessageDialog {
      id: errorDialog
      standardButtons: StandardButton.Ok
      title: 'Error'
      text: ''
  }

  SwipeView {
    id: swipeView
    anchors.fill: parent
    currentIndex: tabBar.currentIndex
    interactive: false

    CommitPage {
    }

    TreePage {
    }
  }

  footer: TabBar {
    id: tabBar
    currentIndex: swipeView.currentIndex
    TabButton {
      text: qsTr("Commit")
    }
    TabButton {
      text: qsTr("History")
    }
  }
}
