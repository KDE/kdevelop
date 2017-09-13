import QtQuick 2.0

Image {
    fillMode: Image.PreserveAspectFit

    RegExpValidator {
        regExp: /asdf/
    }

    Rectangle {
        anchors.fill: parent
    }
}
