import QtQuick 1.1

Rectangle {
    width: 220
    height: 70
    property string value: "monospace"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    Text {
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 9
        font.family: parent.value.replace('"', '').replace('"', '')
        text: "The lazy dog jumps over<br>the quick brown fox."
        horizontalAlignment: TextInput.AlignHCenter
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Font family"
        color: "white"
        opacity: 0.8
    }
}