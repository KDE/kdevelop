import QtQuick 1.1

Rectangle {
    width: 120
    height: 120
    property string value: "1"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider/96*20)/20.0;
    }
    function valueToSlider(value) {
        return value*96;
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Opacity of an item"
        color: "white"
        opacity: 0.8
    }
    Slider {
        z: 20
        id: "slider"
        initialValue: valueToSlider(parent.value)
        anchors.verticalCenter: parent.horizontalCenter
        width: 100
        height: 12
        color: "#929292"
    }
    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -8
        anchors.horizontalCenterOffset: -8
        color: "#1f61a8"
        width: 45
        height: 45
        opacity: parent.value
        z: 1
    }
    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 8
        anchors.horizontalCenterOffset: 8
        color: "#a8002f"
        width: 45
        height: 45
        opacity: 1
        z: 0
    }
}