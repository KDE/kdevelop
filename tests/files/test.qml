import QtQuick 1.1

/**
 * "type" : { "toString" : "<class>" }
 */
Text {
    /*
     * "toString" : "<class> someId",
     * "useCount" : 3
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
     * "toString" : "mixed bar"
     */
    property var bar: "I can contain anything"

    /**
     * "toString" : "int[] ints"
     */
    property list<int> ints

    /**
     * "toString" : "void test (int, int)",
     * "useCount" : 1
     */
    signal test(int a, int b)
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
    }
}
