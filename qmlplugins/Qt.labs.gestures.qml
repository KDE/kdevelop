import QtQuick.tooling 1.1

// This file describes the plugin-supplied types contained in the library.
// It is used for QML tooling purposes only.
//
// This file was auto-generated with the command 'qml1plugindump Qt.labs.gestures 1.0'.

Module {
    Component {
        name: "QDeclarativeGestureArea"
        defaultProperty: "data"
        prototype: "QDeclarativeItem"
        exports: ["GestureArea 1.0"]
        Property { name: "gesture"; type: "QGesture"; isReadonly: true; isPointer: true }
    }
    Component {
        name: "QGesture"
        prototype: "QObject"
        exports: ["Gesture 1.0"]
        Property { name: "state"; type: "Qt::GestureState"; isReadonly: true }
        Property { name: "gestureType"; type: "Qt::GestureType"; isReadonly: true }
        Property { name: "gestureCancelPolicy"; type: "QGesture::GestureCancelPolicy" }
        Property { name: "hotSpot"; type: "QPointF" }
        Property { name: "hasHotSpot"; type: "bool"; isReadonly: true }
    }
    Component {
        name: "QPanGesture"
        prototype: "QGesture"
        exports: ["PanGesture 1.0"]
        Property { name: "lastOffset"; type: "QPointF" }
        Property { name: "offset"; type: "QPointF" }
        Property { name: "delta"; type: "QPointF"; isReadonly: true }
        Property { name: "acceleration"; type: "double" }
        Property { name: "horizontalVelocity"; type: "double" }
        Property { name: "verticalVelocity"; type: "double" }
    }
    Component {
        name: "QPinchGesture"
        prototype: "QGesture"
        exports: ["PinchGesture 1.0"]
        Enum {
            name: "ChangeFlag"
            values: {
                "ScaleFactorChanged": 1,
                "RotationAngleChanged": 2,
                "CenterPointChanged": 4
            }
        }
        Enum {
            name: "ChangeFlags"
            values: {
                "ScaleFactorChanged": 1,
                "RotationAngleChanged": 2,
                "CenterPointChanged": 4
            }
        }
        Property { name: "totalChangeFlags"; type: "ChangeFlags" }
        Property { name: "changeFlags"; type: "ChangeFlags" }
        Property { name: "totalScaleFactor"; type: "double" }
        Property { name: "lastScaleFactor"; type: "double" }
        Property { name: "scaleFactor"; type: "double" }
        Property { name: "totalRotationAngle"; type: "double" }
        Property { name: "lastRotationAngle"; type: "double" }
        Property { name: "rotationAngle"; type: "double" }
        Property { name: "startCenterPoint"; type: "QPointF" }
        Property { name: "lastCenterPoint"; type: "QPointF" }
        Property { name: "centerPoint"; type: "QPointF" }
    }
    Component {
        name: "QSwipeGesture"
        prototype: "QGesture"
        exports: ["SwipeGesture 1.0"]
        Enum {
            name: "SwipeDirection"
            values: {
                "NoDirection": 0,
                "Left": 1,
                "Right": 2,
                "Up": 3,
                "Down": 4
            }
        }
        Property { name: "horizontalDirection"; type: "SwipeDirection"; isReadonly: true }
        Property { name: "verticalDirection"; type: "SwipeDirection"; isReadonly: true }
        Property { name: "swipeAngle"; type: "double" }
        Property { name: "velocity"; type: "double" }
    }
    Component {
        name: "QTapAndHoldGesture"
        prototype: "QGesture"
        exports: ["TapAndHoldGesture 1.0"]
        Property { name: "position"; type: "QPointF" }
    }
    Component {
        name: "QTapGesture"
        prototype: "QGesture"
        exports: ["TapGesture 1.0"]
        Property { name: "position"; type: "QPointF" }
    }
}
