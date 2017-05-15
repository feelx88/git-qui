import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

SplitView {
    width: 800
    height: 600
    property alias diffView: diffView
    property alias stagedArea: stagedArea
    property alias unstagedArea: unstagedArea

    SplitView {
        id: splitView
        width: 320
        height: 480
        orientation: Qt.Vertical

        ListView {
            id: unstagedArea
            width: 320
            height: 240
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
        }

        ListView {
            id: stagedArea
            y: 240
            width: 320
            height: 240
            clip: true
        }
    }

    SplitView {
        id: splitView1
        x: 320
        width: 320
        height: 480
        orientation: Qt.Vertical

        ScrollView {
            width: 320
            height: 240
            x: 0
            TextArea {
                id: diffView
                text: ""
                font.family: "Monospace"
                textFormat: Text.RichText
                font.pixelSize: 12
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
                    id: button
                    text: qsTr("Button")
                }

                Button {
                    id: button1
                    text: qsTr("Button")
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
                    id: textEdit1
                    width: 80
                    height: 20
                    text: qsTr("Text Edit")
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.pixelSize: 12
                }
            }
        }
    }
}
