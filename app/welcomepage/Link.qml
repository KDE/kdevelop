import QtQuick 1.1

Text {
    id: text
    signal clicked;
    
    horizontalAlignment: Text.AlignHCenter
    font.pointSize: 20

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onClicked: text.clicked();
        onEntered: text.state="hovered"
        onExited: text.state="default"
    }
    
    states: [
         State {
             name: "default"
             PropertyChanges { target: text; color: "black" }
         },
         State {
             name: "hovered"
             PropertyChanges { target: text; color: "red" }
         }
     ]
}