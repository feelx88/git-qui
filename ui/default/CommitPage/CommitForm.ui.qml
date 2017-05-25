import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

SplitView {
  width: 800
  height: 600
  property alias unstagedContainer: unstagedContainer
  property alias stagedContainer: stagedContainer
  property alias buttonRefresh: buttonRefresh
  property alias splitViewLeft: splitViewLeft
  property alias splitViewRight: splitViewRight
  property alias diffViewInactive: diffViewInactive
  property alias buttonCommit: buttonCommit
  property alias commitMessage: commitMessage
  property alias diffView: diffView
  property alias stagedArea: stagedArea
  property alias unstagedArea: unstagedArea

  SplitView {
    id: splitViewLeft
    width: 320
    height: 480
    orientation: Qt.Vertical


    ColumnLayout {
      id: unstagedContainer
      width: 100
      height: 100
      Layout.fillWidth: true
      Layout.fillHeight: true

      Rectangle {
        id: rectangle1
        width: 200
        height: text2.height
        color: "#cdcdcd"
        Layout.fillWidth: true

        Text {
          id: text2
          text: qsTr("Unstaged files")
          rightPadding: 5
          leftPadding: 5
          bottomPadding: 10
          topPadding: 10
          font.pixelSize: 12
        }
      }

      ListView {
        id: unstagedArea
        width: 320
        height: 240
        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
      }

    }

    ColumnLayout {
      id: stagedContainer
      width: 100
      height: 100
      Layout.fillWidth: true
      Layout.fillHeight: true

      Rectangle {
        id: rectangle2
        width: 200
        height: text3.height
        color: "#cdcdcd"
        Text {
          id: text3
          text: qsTr("Staged files")
          rightPadding: 5
          leftPadding: 5
          topPadding: 10
          font.pixelSize: 12
          bottomPadding: 10
        }
        Layout.fillWidth: true
      }

      ListView {
        id: stagedArea
        width: 320
        height: 240
        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
      }

    }
  }

  SplitView {
    id: splitViewRight
    x: 320
    width: 320
    height: 480
    orientation: Qt.Vertical

    ColumnLayout {
      id: column
      width: 200
      height: 400
      Layout.fillWidth: true
      Layout.fillHeight: true


      RowLayout {
        id: row
        height: buttonStageLines.height
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
          id: rectangle
          height: row.height
          color: "#cdcdcd"
          border.width: 0
          anchors.fill: parent
        }


      }

      ScrollView {
        anchors.top: row.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.topMargin: 0
        Layout.fillWidth: true
        Layout.fillHeight: true

        ListView {
          id: diffView
          x: 0
          y: 0
          width: 110
          height: 160
          flickableDirection: Flickable.HorizontalAndVerticalFlick
          interactive: true
          clip: true

          Rectangle {
            id: diffViewInactive
            color: "#a2a2a2"
            opacity: 0
            anchors.fill: parent
            z: -10

            Text {
              id: text1
              text: qsTr("No diff selected")
              anchors.horizontalCenter: parent.horizontalCenter
              anchors.verticalCenter: parent.verticalCenter
              font.pixelSize: 12
            }
          }
        }
      }
    }

    RowLayout {
      id: rowLayout
      y: 240
      width: 320
      height: 240

      ColumnLayout {
        id: columnLayout
        width: 320
        height: 240

        Button {
          id: buttonCommit
          text: qsTr("Commit")
        }

        Button {
          id: buttonRefresh
          text: qsTr("Refresh")
        }

        Button {
          id: button2
          text: qsTr("Button")
        }

        Button {
          id: button3
          text: qsTr("Button")
        }

        Button {
          id: button4
          text: qsTr("Button")
        }
      }

      ColumnLayout {
        id: columnLayout1
        width: 100
        height: 100

        SwitchDelegate {
          id: switchDelegate
          text: qsTr("Amend last commit")
          Layout.fillWidth: false
        }

        TextEdit {
          id: commitMessage
          width: 80
          height: 20
          text: qsTr("")
          clip: true
          Layout.fillHeight: true
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
          font.pixelSize: 12
        }
      }
    }
  }
}
