import QtQuick 1.0

Rectangle {
    id: rectangle1
    width: 500
    height: 400

    Column {
        id: column1
        height: parent.height
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width/2
        spacing: 50

        Text {
            id: text3
            text: qsTr("New Project")
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 23
            width: parent.width
        }

        Text {
            id: text2
            text: qsTr("Import project")
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 23
            width: parent.width
        }
    }

    ListView {
        id: list_view1
        anchors.left: column1.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        delegate: Item {
            height: 40
            Row {
                spacing: 10
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                }

                Text {
                    text: name
                    anchors.verticalCenter: parent.verticalCenter
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        model: ListModel {
            ListElement {
                name: "Grey"
                colorCode: "grey"
            }

            ListElement {
                name: "Red"
                colorCode: "red"
            }

            ListElement {
                name: "Blue"
                colorCode: "blue"
            }

            ListElement {
                name: "Green"
                colorCode: "green"
            }
        }
    }
}
