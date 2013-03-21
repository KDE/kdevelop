import QtQuick 1.1

Rectangle {
    id: root
    property int initialValue: 0
    property int maxValue: 100
    property int initialized: 0
    onInitialValueChanged: {
        if ( initialized == 2 ) return;
        initialized += 1
        blob.x = initialValue
    }
    border.width: 1
    function validate(x) {
        if ( x > root.width - 6 ) {
            x = root.width - 6
        }
        if ( x < 0 ) {
            x = 0;
        }
        return x;
    }
    radius: 2
    anchors.margins: 6
    border.color: "#999999"
    color: "#00FFFFFF"
    Rectangle {
        id: "blob"
        color: "#52b7ff"
        width: 5
        y: 1
        height: parent.height - 2
        z: 10
        radius: 2
        onXChanged: {
            if ( validate(x) != x ) x = validate(x)
        }
    }
    Rectangle {
        radius: 2
        width: blob.x
        height: parent.height - 2
        y: 1
        x: 2
        color: "#32364e"
        opacity: 0.6
    }
    gradient: Gradient {
        GradientStop {
            position: 0.0;
            color: "#999999";
        }
        GradientStop {
            position: 0.5;
            color: "#5a5a5a";
        }
    }
    MouseArea {
        anchors.fill: parent
        onPositionChanged: {
            var res = mouse.x >= 0 ? mouse.x : 0
            blob.x = res
            valueChanged(sliderToValue(res))
        }
    }
}