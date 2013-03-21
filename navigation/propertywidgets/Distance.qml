import QtQuick 1.1

Rectangle {
    width: 150
    height: 110
    property string value: "10"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider*slider/40);
    }
    function valueToSlider(value) {
        return Math.sqrt(value*40);
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Location in pixels"
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
        anchors.horizontalCenterOffset: -value/2.0 - 2
        color: "#0070a8"
        width: 4
        height: 60
    }

    Rectangle {
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: +value/2.0 + 2
        color: "#0070a8"
        width: 4
        height: 60
    }
}