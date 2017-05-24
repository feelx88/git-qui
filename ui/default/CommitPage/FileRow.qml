import QtQuick 2.7
import QtQuick.Controls 2.1

Rectangle {
  id: row
  signal updated();
  signal clicked(ListView listView, string path);
  width: parent.width
  height: 25
  color: "#00000000"
  opacity: 1

  Menu {
    id: contextMenu
    MenuItem {
      text: 'Reset file'
      onClicked: {
        gitManager.checkout(path)
        updated();
      }
    }
  }

  CheckBox {
    id: _checkBox
    anchors.left: parent.left
    anchors.leftMargin: -5
    scale: 0.7
    anchors.verticalCenter: parent.verticalCenter
  }

  ToolButton {
    id: _buttonModifiyIndex
    anchors.verticalCenter: parent.verticalCenter
    text: buttonLabel
    anchors.left: _checkBox.right
    anchors.leftMargin: -20
    onClicked: {
      if (staged) {
        gitManager.unstagePath(path);
      } else {
        gitManager.stagePath(path);
      }
      updated();
    }
  }

  Text {
    width: 360
    height: parent.height
    verticalAlignment: Text.AlignVCenter
    text: path
    anchors.left: _buttonModifiyIndex.right
    anchors.leftMargin: -5
    color: modified ? '#0000ff' : '#000000';
    font.strikeout: deleted;

    MouseArea {
      anchors.fill: parent
      acceptedButtons: Qt.LeftButton | Qt.RightButton
      onClicked: {
        row.clicked(row.ListView.view, path);
        row.ListView.view.currentIndex = index
        if (!staged && mouse.button == Qt.RightButton) {
          contextMenu.x = mouse.x;
          contextMenu.y = mouse.y;
          contextMenu.visible = true;
        }
      }
    }
  }
}