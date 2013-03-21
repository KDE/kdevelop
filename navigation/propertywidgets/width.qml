import QtQuick 1.1

Rectangle {
    property string value: "10"
    property string initialValue
    onInitialValueChanged: value = initialValue
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return 3*slider;
    }
    function valueToSlider(value) {
        return value/3;
    }
    Slider {
        id: "slider"
        initialValue: valueToSlider(parent.initialValue)
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
        width: parent.value
    }
}