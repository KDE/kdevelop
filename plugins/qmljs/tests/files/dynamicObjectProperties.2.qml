import QtQuick 2.0
import QtQuick 2.0 as Qt47

Qt.QtObject {
    // qmljs isn't capable of parsing this type, see bug 364145
    property Qt47.QtObject objectProperty
}
