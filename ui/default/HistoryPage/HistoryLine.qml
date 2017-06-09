import QtQuick 2.7
import QtQuick.Layouts 1.1

Item {
  property var branchModel
  height: 20
  width: parent.width
  Row {
    id: label
    x: x
    Repeater {
      model: Object.keys(branchModel).length
      Layout.rightMargin: 5
      delegate: Text {
        text: branchModel[index]
        leftPadding: 5
        rightPadding: 5
        Rectangle {
          color: 'yellow'
          anchors.fill: parent
          border.color: 'black'
          z: -1
        }
      }
    }
  }

  RowLayout {
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: label.width + label.x + 5
    anchors.rightMargin: 10
    Text {
      text: message
    }
    Text {
      text: id
      font.family: 'monospace'
      anchors.right: parent.right
    }
  }
  MouseArea {
    anchors.fill: parent
    onClicked: parent.ListView.view.currentIndex = index
  }
}
