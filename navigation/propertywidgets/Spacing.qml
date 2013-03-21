import QtQuick 1.1

Rectangle {
    width: 150
    height: 110
    property string value: "3"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider/8.0)
    }
    function valueToSlider(value) {
        return value*8;
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
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Spacing between items"
        color: "white"
        opacity: 0.8
    }
    Grid {
        columns: 3
        spacing: parent.value
        anchors.centerIn: parent
        Repeater {
            model: 9
            delegate: Rectangle {
                color: "#40a838"
                height: 8
                width: 8
            }
        }
    }
}