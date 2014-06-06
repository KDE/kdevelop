/**
 * "toString" : "plugins plugins",
 * "kind" : "Namespace"
 */
Module {
     Component {
         name: "QModelIndex"
     }

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
          * "toString" : "void dataChanged (plugins::QModelIndex, plugins::QModelIndex, int)",
          * "useCount" : 1
          */
         Signal {
             name: "dataChanged"

             /**
              * "toString" : "plugins::QModelIndex topLeft"
              */
             Parameter /* */ { name: "topLeft"; type: "QModelIndex" }

             /**
              * "toString" : "plugins::QModelIndex bottomRight"
              */
             Parameter /* */ { name: "bottomRight"; type: "QModelIndex" }

             /**
              * "toString" : "int roles"
              */
             Parameter /* */ { name: "roles"; type: "int" }
         }

         /**
          * "toString" : "bool isItemValid (plugins::QModelIndex)"
          */
         Method {
             name: "isItemValid"
             type: "bool"
             Parameter { name: "item"; type: "QModelIndex" }
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

         onDataChanged: {
             return;
         }
     }

     /**
      * "kind" : "Type"
      */
     AbstractItemModel
     {
         id: bar
     }
}
