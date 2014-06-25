import QtQuick 2.0

Text {
    /*
     * "toString" : "test someId",
     * "useCount" : 3
     */
    id: someId

    /**
     * "type" : { "toString" : "string" },
     * "useCount" : 2
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
     * "toString" : "mixed bar"
     */
    property var bar: "I can contain anything"

    /**
     * "toString" : "mixed baz"
     */
    property variant baz

    /**
     * "toString" : "int[] ints"
     */
    property list<int> ints

    /**
     * "toString" : "void test (int, int)",
     * "useCount" : 1
     */
    signal test(
        /**
         * "toString" : "int a",
         * "useCount" : 1
         */
        int a,

        /**
         * "toString" : "int b",
         * "useCount" : 0
         */
        int b
    )
    signal test2

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

        onLoad: {
            console.log(parent.text);
        }
    }

    /**
     * "type" : { "toString" : "function void (mixed)" },
     * "returnType" : { "toString" : "void" }
     */
    function foo(arg)
    {
        someId.text = arg;
        someId.inexistant = false;  // Don't use "someId" at "inexistant". someId must be used only 3 times (declaration, this line and the above one)
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

    onTest: {
        a = 3;
    }
}
