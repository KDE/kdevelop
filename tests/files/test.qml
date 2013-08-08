import QtQuick 1.0

/**
 * "toString" : "class someId",
 * "type" : { "toString"  : "Text" }
 */
Text {
    id: someId
    text: "asdf"

    /**
     * "EXPECT_FAIL" : { "toString" : "type deduction not implemented" },
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
