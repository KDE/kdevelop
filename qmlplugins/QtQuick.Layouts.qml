import QtQuick.tooling 1.1

// This file describes the plugin-supplied types contained in the library.
// It is used for QML tooling purposes only.
//
// This file was auto-generated with the command 'qmlplugindump -nonrelocatable QtQuick.Layouts 1.1'.

Module {
    Component {
        name: "QQuickColumnLayout"
        defaultProperty: "data"
        prototype: "QQuickLinearLayout"
        exports: ["QtQuick.Layouts/ColumnLayout 1.0"]
        exportMetaObjectRevisions: [0]
    }
    Component {
        name: "QQuickGridLayout"
        defaultProperty: "data"
        prototype: "QQuickGridLayoutBase"
        exports: ["QtQuick.Layouts/GridLayout 1.0"]
        exportMetaObjectRevisions: [0]
        Enum {
            name: "Flow"
            values: {
                "LeftToRight": 0,
                "TopToBottom": 1
            }
        }
        Property { name: "columnSpacing"; type: "double" }
        Property { name: "rowSpacing"; type: "double" }
        Property { name: "columns"; type: "int" }
        Property { name: "rows"; type: "int" }
        Property { name: "flow"; type: "Flow" }
    }
    Component {
        name: "QQuickGridLayoutBase"
        defaultProperty: "data"
        prototype: "QQuickLayout"
        exports: ["QtQuick.Layouts/LayoutBase 1.1"]
        exportMetaObjectRevisions: [1]
        Property { name: "layoutDirection"; revision: 1; type: "Qt::LayoutDirection" }
    }
    Component {
        name: "QQuickLayout"
        defaultProperty: "data"
        prototype: "QQuickItem"
        exports: ["QtQuick.Layouts/Layout 1.0"]
        exportMetaObjectRevisions: [0]
        attachedType: "QQuickLayoutAttached"
    }
    Component {
        name: "QQuickLayoutAttached"
        prototype: "QObject"
        Property { name: "minimumWidth"; type: "double" }
        Property { name: "minimumHeight"; type: "double" }
        Property { name: "preferredWidth"; type: "double" }
        Property { name: "preferredHeight"; type: "double" }
        Property { name: "maximumWidth"; type: "double" }
        Property { name: "maximumHeight"; type: "double" }
        Property { name: "fillHeight"; type: "bool" }
        Property { name: "fillWidth"; type: "bool" }
        Property { name: "row"; type: "int" }
        Property { name: "column"; type: "int" }
        Property { name: "rowSpan"; type: "int" }
        Property { name: "columnSpan"; type: "int" }
        Property { name: "alignment"; type: "Qt::Alignment" }
    }
    Component {
        name: "QQuickLinearLayout"
        defaultProperty: "data"
        prototype: "QQuickGridLayoutBase"
        Property { name: "spacing"; type: "double" }
    }
    Component {
        name: "QQuickRowLayout"
        defaultProperty: "data"
        prototype: "QQuickLinearLayout"
        exports: ["QtQuick.Layouts/RowLayout 1.0"]
        exportMetaObjectRevisions: [0]
    }
}
