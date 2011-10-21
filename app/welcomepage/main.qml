import QtQuick 1.0

Rectangle {
    id: rectangle1
    width: 360
    height: 360
    color: 'green'

    Column {
        id: tabs
        width: parent.width/4
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 30
        spacing: 30

        Text {
            width: parent.width

            text: "Starting"
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 20

            MouseArea {
                anchors.fill: parent

                onClicked: { usePage.visible=false; startingPage.visible=true; }
            }
        }

        Text {
            width: parent.width

            text: "Learn KDevelop!"
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 20

            MouseArea {
                anchors.fill: parent

                onClicked: { usePage.visible=true; startingPage.visible=false; }
            }
        }
    }

    Item {
        id: item1
        x: tabs.width+tabs.x
        height: parent.height
        width: parent.width-x
        anchors.margins: 50

        Starting {
            id: startingPage
            anchors.fill: parent
        }

        UseKDevelop {
            id: usePage

            anchors.fill: parent
            visible: false
        }
    }
}
