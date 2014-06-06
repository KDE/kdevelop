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
     * "useCount" : 2
     */
    text: "asdf"

    /**
     * "type" : { "toString" : "string" },
     * "useCount" : 1
     */
    dont_use_me: "I'm used only here, and Behavior on foo.onLoad cannot see me"

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
        onLoad: {
            dont_use_me = 2;
        }
    }

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
}
