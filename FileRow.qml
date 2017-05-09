import QtQuick 2.7
import QtQuick.Controls 2.1

Row {
  id: row
  CheckBox {
    id: checkbox
    anchors.verticalCenter: parent.verticalCenter
  }

  ToolButton {
    anchors.verticalCenter: parent.verticalCenter
    text: staged ? '-' : '+'
  }

  Text {
    anchors.verticalCenter: parent.verticalCenter
    text: path
    color: modified ? '#0000ff' : '#000000';
  }
}
