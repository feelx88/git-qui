import QtQuick 2.7
import QtQuick.Controls 2.1

Row {
  id: row
  CheckBox {
    id: checkbox
    anchors.verticalCenter: parent.verticalCenter
  }
  Text {
    text: value
    anchors.verticalCenter: parent.verticalCenter
    color: modified ? '#0000ff' : '#000000';
  }
}
