import QtQuick 2.7
import QtQuick.Controls 2.1
import de.feelx88.GitDiffLine 1.0

Rectangle {
  id: row

  property color _color: '#55ffffff'
  property string _typeText: ' '
  width: parent.width
  height: 20
  color: _color

  CheckBox {
    id: _checkBox
    anchors.left: parent.left
    anchors.leftMargin: -10
    scale: 0.7
    anchors.verticalCenter: parent.verticalCenter
    opacity: (type == GitDiffLine.ADD || type == GitDiffLine.REMOVE) ? 1 : 0
  }
  Text {
    id: _type
    text: _typeText
    anchors.verticalCenterOffset: 0
    anchors.left: _checkBox.right
    anchors.leftMargin: -5
    font.family: 'monospace'
    anchors.verticalCenter: _checkBox.verticalCenter
  }
  Text {
    id: _content
    text: content
    anchors.left: _type.right
    anchors.leftMargin: 10
    font.family: 'monospace'
    verticalAlignment: Text.AlignVCenter
    anchors.verticalCenter: _checkBox.verticalCenter
  }

  Component.onCompleted: {
    if (type == GitDiffLine.ADD) {
      _typeText = '+';
      _color = '#0f0';
    } else if (type == GitDiffLine.REMOVE) {
      _typeText = '-';
      _color = '#f00';
    } else if (type == GitDiffLine.HEADER) {
      _color = '#aaa';
    }
  }
}
