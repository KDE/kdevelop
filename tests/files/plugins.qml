/**
 * "toString" : "plugins plugins",
 * "kind" : "Namespace"
 */
Module {
     /**
      * "toString" : "interface QAbstractItemModel",
      * "kind": "Type",
      * "useCount" : 1
      */
     Component {
         name: "QAbstractItemModel"
         prototype: "QObject"
         exports: [
            /**
             * "toString" : "class AbstractItemModel"
             */
            "QtQuick/AbstractItemModel 1.0"
        ]

         /**
          * "type" : { "toString" : "plugins::QAbstractItemModel::LayoutChangeHint" },
          * "kind" : "Type",
          * "internalContext" : { "type" : "Enum" }
          */
         Enum {
             name: "LayoutChangeHint"
             values: {
                 /**
                  * "toString" : "double NoLayoutChangeHint"
                  */
                 "NoLayoutChangeHint": 0,
                 "VerticalSortHint": 1,
                 "HorizontalSortHint": 2
             }
         }
         /**
          * "toString" : "void dataChanged (QModelIndex, QModelIndex, int)"
          */
         Signal {
             name: "dataChanged"

             /**
              * "toString" : "QModelIndex topLeft"
              */
             Parameter /* */ { name: "topLeft"; type: "QModelIndex" }

             /**
              * "toString" : "QModelIndex bottomRight"
              */
             Parameter /* */ { name: "bottomRight"; type: "QModelIndex" }

             /**
              * "toString" : "int roles"
              */
             Parameter /* */ { name: "roles"; type: "int" }
         }

         /**
          * "toString" : "bool isFocusPointModeSupported (QDeclarativeCamera::FocusPointMode)"
          */
         Method {
             name: "isFocusPointModeSupported"
             type: "bool"
             Parameter { name: "mode"; type: "QDeclarativeCamera::FocusPointMode" }
         }
     }

     /**
      * "kind" : "Type"
      */
     QAbstractItemModel
     {
         /**
          * "kind": "Instance"
          */
         id: foo
     }

     /**
      * "kind" : "Type"
      */
     AbstractItemModel
     {
         id: bar
     }
}
