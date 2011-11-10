import QtQuick 1.0

Rectangle {
    id: root
    width: 360
    height: 360
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#61B056" }
         GradientStop { position: 1.0; color: "#A3D69B" }
    }
    
    Component.onCompleted: root.state="starting"

    Column {
        id: tabs
        width: 250
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 30
        spacing: 30

        Link {
            width: parent.width

            text: "Starting"
            onClicked: root.state = "starting"
        }

        Link {
            width: parent.width

            text: "Learn KDevelop!"
            onClicked: root.state = "learning"
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
            visible: root.state=="starting"
        }

        UseKDevelop {
            id: usePage

            anchors.fill: parent
            visible: root.state=="learning"
        }
    }
    states: [
        State { name: "learning" },
        State { name: "starting" }
    ]
}
