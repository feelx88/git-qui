import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import "CommitPage"
import "TreePage"

ApplicationWindow {
  visible: true
  width: 800
  height: 600
  title: qsTr("Git QUI")

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
