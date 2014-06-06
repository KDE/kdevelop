import QtQuick 1.1

/**
 * "type" : { "toString" : "<class>" }
 */
Text {
    /*
     * "toString" : "<class> someId"
     */
    id: someId

    /**
     * "type" : { "toString" : "string" },
     * "useCount" : 1
     */
    property string text: "Hello"

    /**
     * "toString" : "int foo"
     */
    property int foo: 1

    /**
     * "type" : { "toString" : "<class>" }
     */
    Behavior on foo {
        /**
        * "toString" : "<class> behavior"
        */
        id: behavior
    }

    /**
     * "type" : { "toString" : "bool" }
     */
    property alias behaviorEnabled: behavior.enabled

    /**
     * "toString" : "bool bar"
     */
    property bool bar: false

    /**
     * "type" : { "toString" : "<class>" }
     */
    Foo {
        /**
         * "type" : { "toString" : "<class>" }
         */
        Bar {
            /**
             * "toString" : "<class> bar"
             */
            id: bar
        }
    }

    /**
     * "type" : { "toString" : "function void (mixed)" },
     * "returnType" : { "toString" : "void" }
     */
    function foo(arg)
    {
        someId.text = arg
    }

    onFontChanged: {
        /**
         * "type" : { "toString" : "int" },
         * "useCount": 0
         */
        var im_not_visible_outside_this_slot = 2;
    }

    onElideModeChanged: {
        im_not_visible_outside_this_slot = "fail";
    }
}
