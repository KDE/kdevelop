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
     * "toString" : "int foo"
     */
    property int foo: 1

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
