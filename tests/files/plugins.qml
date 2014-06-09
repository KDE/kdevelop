/**
 * "toString" : "plugins plugins",
 * "kind" : "Namespace"
 */
Module {
     Component {
         name: "QModelIndex"

         /**
          * "toString" : "int childCount",
          * "useCount" : 1
          */
         Property { name: "childCount"; type: "int" }
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
          * "toString" : "plugins::QModelIndex rootItem"
          */
         Property { name: "rootItem"; type: "QModelIndex" }

         /**
          * "type" : { "toString" : "plugins::QAbstractItemModel::LayoutChangeHint" },
          * "kind" : "Type",
          * "internalContext" : { "type" : "Enum" }
          */
         Enum {
             name: "LayoutChangeHint"
             values: {
                 /**
                  * "toString" : " NoLayoutChangeHint"
                  */
                 "NoLayoutChangeHint": 0,
                 "VerticalSortHint": 1,
                 "HorizontalSortHint": 2
             }
         }
         /**
          * "toString" : "void dataChanged (plugins::QModelIndex, plugins::QModelIndex, int)",
          * "useCount" : 2
          */
         Signal {
             name: "dataChanged"

             /**
              * "toString" : "plugins::QModelIndex topLeft",
              * "useCount" : 1
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

         onDataChanged: console.log(topLeft);
     }

     /**
      * "kind" : "Type"
      */
     AbstractItemModel
     {
         id: bar

         rootItem.childCount: "foo"

         AbstractItemModel.onDataChanged: {
             return;
         }
     }
}
