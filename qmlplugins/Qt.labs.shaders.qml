import QtQuick.tooling 1.1

// This file describes the plugin-supplied types contained in the library.
// It is used for QML tooling purposes only.
//
// This file was auto-generated with the command 'qml1plugindump Qt.labs.shaders 1.0'.

Module {
    Component {
        name: "ShaderEffectItem"
        defaultProperty: "data"
        prototype: "QDeclarativeItem"
        exports: ["ShaderEffectItem 1.0"]
        Property { name: "fragmentShader"; type: "string" }
        Property { name: "vertexShader"; type: "string" }
        Property { name: "blending"; type: "bool" }
        Property { name: "meshResolution"; type: "QSize" }
        Signal { name: "activeChanged" }
    }
    Component {
        name: "ShaderEffectSource"
        defaultProperty: "data"
        prototype: "QDeclarativeItem"
        exports: ["ShaderEffectSource 1.0"]
        Enum {
            name: "WrapMode"
            values: {
                "ClampToEdge": 0,
                "RepeatHorizontally": 1,
                "RepeatVertically": 2,
                "Repeat": 3
            }
        }
        Enum {
            name: "Format"
            values: {
                "Alpha": 6406,
                "RGB": 6407,
                "RGBA": 6408
            }
        }
        Property { name: "sourceItem"; type: "QDeclarativeItem"; isPointer: true }
        Property { name: "sourceRect"; type: "QRectF" }
        Property { name: "textureSize"; type: "QSize" }
        Property { name: "live"; type: "bool" }
        Property { name: "hideSource"; type: "bool" }
        Property { name: "wrapMode"; type: "WrapMode" }
        Signal { name: "formatChanged" }
        Signal { name: "activeChanged" }
        Signal { name: "repaintRequired" }
        Method { name: "markSceneGraphDirty" }
        Method { name: "markSourceSizeDirty" }
        Method { name: "markSourceItemDirty" }
        Method { name: "grab" }
    }
}
