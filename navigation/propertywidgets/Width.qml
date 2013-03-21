import QtQuick 1.1

Rectangle {
    width: 220
    height: 110
    property string value: "10"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider*slider/30);
    }
    function valueToSlider(value) {
        return Math.sqrt(value*30);
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Width of an item in pixels"
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
    Rectangle {
        anchors.centerIn: parent
        color: "#65a800"
        height: 30
        width: parent.value < parent.width - 20 ? parent.value : parent.width - 20
        opacity: parent.value < parent.width - 20 ? 1 : 0.5
    }
}