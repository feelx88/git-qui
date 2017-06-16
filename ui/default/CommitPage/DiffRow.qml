import QtQuick 2.7
import QtQuick.Controls 2.1
import de.feelx88.GitDiffLine 1.0

Rectangle {
  id: row

  signal updated();

  property color _color: '#55ffffff'
  property string _typeText: ' '
  width: parent.width
  height: 20
  color: _color

  MouseArea {
    anchors.fill: parent
    onClicked: {
      if(!(type == GitDiffLine.ADD || type == GitDiffLine.REMOVE)) {
        return;
      }
      _checkBox.checked = !_checkBox.checked;
    }
    onDoubleClicked: {
      if(!(type == GitDiffLine.ADD || type == GitDiffLine.REMOVE)) {
        return;
      }

      gitManager.stageLinesVariant([{
                                      header: header,
                                      content: content,
                                      type: type,
                                      oldLine: oldLine,
                                      newLine: newLine,
                                      index: index
                                   }], staged);
      updated();
    }
  }

  Text {
    id: _oldLine
    text: oldLine > 0 ? oldLine : ''
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 0
    font.pixelSize: 12
    width: 40
  }

  Text {
    id: _newLine
    text: newLine > 0 ? newLine : ''
    anchors.verticalCenterOffset: 0
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: _oldLine.right
    anchors.leftMargin: 10
    font.pixelSize: 12
    width: 40
  }

  CheckBox {
    id: _checkBox
    anchors.left: _newLine.right
    anchors.leftMargin: 0
    scale: 0.7
    anchors.verticalCenter: parent.verticalCenter
    visible: (type == GitDiffLine.ADD || type == GitDiffLine.REMOVE)
    enabled: (type == GitDiffLine.ADD || type == GitDiffLine.REMOVE)
    onCheckedChanged: {
      selected = checked;
    }
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
    textFormat: Text.PlainText
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
