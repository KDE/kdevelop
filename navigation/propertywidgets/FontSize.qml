import QtQuick 1.1

// TODO support other size types than PointSize

Rectangle {
    width: 220
    height: 120
    property string value: "10"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider/7*20)/20.0;
    }
    function valueToSlider(value) {
        return value*7;
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Font size"
        color: "white"
        opacity: 0.8
    }
    Slider {
        id: "slider"
        initialValue: valueToSlider(parent.value)
        anchors.top: parent.top
        anchors.left: parent.left
        width: 100
        height: 12
        color: "#929292"
    }
    Text {
        color: "white"
        anchors.centerIn: parent
        font.pointSize: parent.value
        font.family: "sans" // TODO
        text: "The lazy dog jumps over<br>the quick brown fox."
        horizontalAlignment: TextInput.AlignHCenter
    }
}