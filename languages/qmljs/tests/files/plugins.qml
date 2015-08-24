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
          * "toString" : "QModelIndex rootItem"
          */
         Property { name: "rootItem"; type: "QModelIndex" }

         /**
          * "type" : { "toString" : "function void ()" }
          */
         Property { name: "script"; type: "QQuickScriptString" }

         /**
          * "type" : { "toString" : "QAbstractItemModel::LayoutChangeHint" },
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
          * "toString" : "void dataChanged (QModelIndex, QModelIndex, int)",
          * "useCount" : 2
          */
         Signal {
             name: "dataChanged"

             /**
              * "toString" : "QModelIndex topLeft",
              * "useCount" : 1
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
          * "toString" : "bool isItemValid (QModelIndex)"
          */
         Method {
             name: "isItemValid"
             type: "bool"
             Parameter { name: "item"; type: "QModelIndex" }
         }
     }

     Component {
         name: "QModelIndex"

         /**
          * "toString" : "int childCount",
          * "useCount" : 2
          */
         Property { name: "childCount"; type: "int" }
     }

     /**
      * "useCount" : 1
      */
     Component {
         name: "ComponentOfAProperty"
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

         property ComponentOfAProperty my_prop;
         onDataChanged: console.log(topLeft);

         rootItem {
             childCount: 8
         }
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
