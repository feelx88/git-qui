import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.0
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
      errorDialog.title = message;
      errorDialog.visible = true;
    }
  }

  Dialog {
      id: errorDialog
      x: parent.width / 2 - width / 2
      y: parent.height / 2 - height / 2
      dim: true
      modal: true
      closePolicy: Popup.NoAutoClose
      standardButtons: Dialog.Ok
      title: ''
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
