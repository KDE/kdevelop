import QtQuick 1.0

Text {
    id: someId
    text: "asdf"
    property int foo: 1

    Foo {
        bar: Text {

        }
    }

    function foo(arg)
    {
        someId.text = arg
    }
}
