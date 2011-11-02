import QtQuick 1.0

StandardPage
{
    color: "#FFF3EC"
    
    Column {
        id: column1
        height: parent.height
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width/2
        anchors.margins: 30
        spacing: 50

        Link { text: qsTr("New Project") }

        Link { text: qsTr("Import project") }
    }

    ListView {
        anchors.left: column1.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 30

        delegate: Item {
            height: 40
            Row {
                spacing: 10
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                }

                Link {
                    text: name
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        model: ListModel {
            ListElement { name: "Grey"; colorCode: "grey" }
            ListElement { name: "Red"; colorCode: "red" }
            ListElement { name: "Blue"; colorCode: "blue" }
            ListElement { name: "Green"; colorCode: "green" }
        }
    }
}
