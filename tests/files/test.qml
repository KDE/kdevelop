import QtQuick 1.1

/**
 * "toString" : "class someId",
 * "type" : { "toString" : "Text" }
 */
Text {
    id: someId
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
     * "toString" : "class ",
     * "type" : { "toString" : "Foo" }
     */
    Foo {
        /**
         * "toString" : "class bar",
         * "type" : { "toString" : "Bar" }
         */
        Bar {
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
