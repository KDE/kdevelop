import QtQuick 2.0

Rectangle {
    width: 300
    height: 240
    color: "steelblue"
    Text {
        text: "Right click to change background color \nand left click to quit."
        anchors.centerIn: parent
        color: "white"
    }
    MouseArea {
         anchors.fill: parent
         acceptedButtons: Qt.LeftButton | Qt.RightButton
         onClicked: {
             if (mouse.button == Qt.RightButton)
                 parent.color = 'green';
             else
                 Qt.quit();
         }
     }
}
