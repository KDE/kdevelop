Module {
     /**
      * "toString" : "class QAbstractItemModel",
      * "useCount" : 2
      */
     Component {
         name: "QAbstractItemModel"
         prototype: "QObject"
         exports: ["QtQuick/AbstractItemModel 1.0"]

         /**
          * "type" : { "toString" : "LayoutChangeHint" },
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
      * "toString" : "class foo"
      */
     QAbstractItemModel
     {
         /**
         * "toString" : "class foo"
         */
         id: foo
     }

     /**
      * "type" : { "toString" : "AbstractItemModel" }
      */
     AbstractItemModel
     {
         id: bar
     }
}
