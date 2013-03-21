import QtQuick 1.1

Rectangle {
    width: 150
    height: 220
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
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Height of an item in pixels"
        color: "white"
        opacity: 0.8
    }
    Slider {
        z: 20
        id: "slider"
        initialValue: valueToSlider(parent.value)
        width: 100
        height: 12
        color: "#929292"
    }
    Rectangle {
        anchors.centerIn: parent
        color: "#65a800"
        width: 30
        height: parent.value < parent.height - 40 ? parent.value : parent.height - 40
        opacity: parent.value < parent.height - 40 ? 1 : 0.5
    }
}