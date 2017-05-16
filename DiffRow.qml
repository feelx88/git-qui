import QtQuick 2.7
import QtQuick.Controls 2.1
import de.feelx88.GitDiffLine 1.0

Row {
  id: row

  property color _color: '#000'
  property string _typeText: ' '

  CheckBox {
    opacity: (type == GitDiffLine.ADD || type == GitDiffLine.REMOVE) ? 1 : 0
  }
  Text {
    text: _typeText
    font.family: 'monospace'
    color: _color
    anchors.verticalCenter: parent.verticalCenter
  }
  Text {
    text: content
    font.family: 'monospace'
    color: _color
    verticalAlignment: Text.AlignVCenter
    anchors.verticalCenter: parent.verticalCenter
  }

  Component.onCompleted: {
    if (type == GitDiffLine.ADD) {
      _typeText = '+';
      _color = '#0a0';
    } else if (type == GitDiffLine.REMOVE) {
      _typeText = '-';
      _color = '#a00';
    } else if (type == GitDiffLine.HEADER) {
      _color = '#aaa';
    }
  }
}
