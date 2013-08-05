import QtQuick 1.0

/**
 * "toString" : "class someId",
 * "type" : { "toString"  : "Text" }
 */
Text {
    id: someId
    text: "asdf"

    /**
     * "toString" : "int foo"
     */
    property int foo: 1

    /**
     * "toString" : "class ",
     * "type" : { "toString"  : "Foo" }
     */
    Foo {
        /**
         * "toString" : "class bar",
         * "type" : { "toString"  : "Bar" }
         */
        Bar {
            id: bar
        }
    }

    /**
     * "returnType" : { "toString" : "void" }
     */
    function foo(arg)
    {
        someId.text = arg
    }
}
