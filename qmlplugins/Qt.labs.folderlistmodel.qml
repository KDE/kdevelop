import QtQuick.tooling 1.1

// This file describes the plugin-supplied types contained in the library.
// It is used for QML tooling purposes only.
//
// This file was auto-generated with the command 'qml1plugindump Qt.labs.folderlistmodel 1.0'.

Module {
    Component {
        name: "QAbstractItemModel"
        prototype: "QObject"
        Enum {
            name: "LayoutChangeHint"
            values: {
                "NoLayoutChangeHint": 0,
                "VerticalSortHint": 1,
                "HorizontalSortHint": 2
            }
        }
        Signal {
            name: "dataChanged"
            Parameter { name: "topLeft"; type: "QModelIndex" }
            Parameter { name: "bottomRight"; type: "QModelIndex" }
            Parameter { name: "roles"; type: "QVector<int>" }
        }
        Signal {
            name: "dataChanged"
            Parameter { name: "topLeft"; type: "QModelIndex" }
            Parameter { name: "bottomRight"; type: "QModelIndex" }
        }
        Signal {
            name: "headerDataChanged"
            Parameter { name: "orientation"; type: "Qt::Orientation" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "layoutChanged"
            Parameter { name: "parents"; type: "QList<QPersistentModelIndex>" }
            Parameter { name: "hint"; type: "QAbstractItemModel::LayoutChangeHint" }
        }
        Signal {
            name: "layoutChanged"
            Parameter { name: "parents"; type: "QList<QPersistentModelIndex>" }
        }
        Signal { name: "layoutChanged" }
        Signal {
            name: "layoutAboutToBeChanged"
            Parameter { name: "parents"; type: "QList<QPersistentModelIndex>" }
            Parameter { name: "hint"; type: "QAbstractItemModel::LayoutChangeHint" }
        }
        Signal {
            name: "layoutAboutToBeChanged"
            Parameter { name: "parents"; type: "QList<QPersistentModelIndex>" }
        }
        Signal { name: "layoutAboutToBeChanged" }
        Signal {
            name: "rowsAboutToBeInserted"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "rowsInserted"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "rowsAboutToBeRemoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "rowsRemoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "columnsAboutToBeInserted"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "columnsInserted"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "columnsAboutToBeRemoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal {
            name: "columnsRemoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "first"; type: "int" }
            Parameter { name: "last"; type: "int" }
        }
        Signal { name: "modelAboutToBeReset" }
        Signal { name: "modelReset" }
        Signal {
            name: "rowsAboutToBeMoved"
            Parameter { name: "sourceParent"; type: "QModelIndex" }
            Parameter { name: "sourceStart"; type: "int" }
            Parameter { name: "sourceEnd"; type: "int" }
            Parameter { name: "destinationParent"; type: "QModelIndex" }
            Parameter { name: "destinationRow"; type: "int" }
        }
        Signal {
            name: "rowsMoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "start"; type: "int" }
            Parameter { name: "end"; type: "int" }
            Parameter { name: "destination"; type: "QModelIndex" }
            Parameter { name: "row"; type: "int" }
        }
        Signal {
            name: "columnsAboutToBeMoved"
            Parameter { name: "sourceParent"; type: "QModelIndex" }
            Parameter { name: "sourceStart"; type: "int" }
            Parameter { name: "sourceEnd"; type: "int" }
            Parameter { name: "destinationParent"; type: "QModelIndex" }
            Parameter { name: "destinationColumn"; type: "int" }
        }
        Signal {
            name: "columnsMoved"
            Parameter { name: "parent"; type: "QModelIndex" }
            Parameter { name: "start"; type: "int" }
            Parameter { name: "end"; type: "int" }
            Parameter { name: "destination"; type: "QModelIndex" }
            Parameter { name: "column"; type: "int" }
        }
        Method { name: "submit"; type: "bool" }
        Method { name: "revert" }
    }
    Component { name: "QAbstractListModel"; prototype: "QAbstractItemModel" }
    Component {
        name: "QDeclarativeFolderListModel"
        prototype: "QAbstractListModel"
        exports: ["FolderListModel 1.0"]
        Enum {
            name: "SortField"
            values: {
                "Unsorted": 0,
                "Name": 1,
                "Time": 2,
                "Size": 3,
                "Type": 4
            }
        }
        Property { name: "folder"; type: "QUrl" }
        Property { name: "parentFolder"; type: "QUrl"; isReadonly: true }
        Property { name: "nameFilters"; type: "QStringList" }
        Property { name: "sortField"; type: "SortField" }
        Property { name: "sortReversed"; type: "bool" }
        Property { name: "showDirs"; type: "bool" }
        Property { name: "showDotAndDotDot"; type: "bool" }
        Property { name: "showOnlyReadable"; type: "bool" }
        Property { name: "count"; type: "int"; isReadonly: true }
        Method {
            name: "isFolder"
            type: "bool"
            Parameter { name: "index"; type: "int" }
        }
    }
}
