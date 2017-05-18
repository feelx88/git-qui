import QtQuick 2.7
import QtQuick.Controls 2.1

Row {
  id: row
  signal updated();
  signal clicked(ListView listView, string path);
  width: parent.width
  height: 25

  CheckBox {
    id: checkbox
    scale: 0.7
    anchors.verticalCenter: parent.verticalCenter
  }

  ToolButton {
    anchors.verticalCenter: parent.verticalCenter
    text: staged ? '-' : '+'
    onClicked: {
      if (staged) {
        gitManager.unstagePath(path);
      } else {
        gitManager.stagePath(path);
      }
      row.updated();
    }
  }

  Text {
    height: parent.height
    width: parent.width
    verticalAlignment: Text.AlignVCenter
    text: path
    color: modified ? '#0000ff' : '#000000';
    font.strikeout: deleted;

    MouseArea {
      anchors.fill: parent
      onClicked: {
        row.clicked(row.ListView.view, path);
        row.ListView.view.currentIndex = index
      }
    }
  }
}
