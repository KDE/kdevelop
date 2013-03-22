import QtQuick 1.1

// TODO support other size types than PointSize

Rectangle {
    id: root
    width: 140
    height: 140
    property string value: "10"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider)+2;
    }
    function valueToSlider(value) {
        return value;
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Item count"
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
        width: (4+2)*10
        height: (4+2)*10
        anchors.centerIn: parent
        color: "#00000000"
        Grid {
            spacing: 2
            columns: 10
            Repeater {
                model: root.value < 100 ? root.value : 100
                delegate: Rectangle {
                    opacity: root.value <= 100 ? 1 : 0.2
                    color: "#0095ff"
                    width: 4
                    height: 4
                }
            }
        }
    }
}