import QtQuick 1.1

Rectangle {
    id: root
    width: 150
    height: 120
    property string value: "1"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return slider*8;
    }
    function valueToSlider(value) {
        return value/8;
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Duration in milliseconds"
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
        id: "sample"
        anchors.centerIn: parent
        color: "#1f61a8"
        width: 50
        height: 50
        SequentialAnimation {
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: root.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: 20; duration: root.value }
            }
            PauseAnimation { duration: 500 }
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: root.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: -20; duration: root.value }
            }
            PauseAnimation { duration: 500 }
            running: true
            loops: Animation.Infinite
        }
    }
}