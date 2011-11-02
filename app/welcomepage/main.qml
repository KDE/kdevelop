import QtQuick 1.0

Rectangle {
    id: rectangle1
    width: 360
    height: 360
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#61B056" }
         GradientStop { position: 1.0; color: "#A3D69B" }
    }

    Column {
        id: tabs
        width: parent.width/4
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 30
        spacing: 30

        Link {
            width: parent.width

            text: "Starting"
            onClicked: { usePage.visible=false; startingPage.visible=true; }
        }

        Link {
            width: parent.width

            text: "Learn KDevelop!"
            onClicked: { usePage.visible=true; startingPage.visible=false; }
        }
    }

    Item {
        id: item1
        x: tabs.width+tabs.x
        height: parent.height
        width: parent.width-x

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
