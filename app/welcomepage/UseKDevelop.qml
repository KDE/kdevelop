import QtQuick 1.0
import QtWebKit 1.0

Rectangle {
    width: 100
    height: 62

    WebView {
        id: web_view1
        x: 23
        y: 55
        width: 300
        height: 300
        scale: 1
        rotation: 0
        contentsScale: 1
        url: "http://kdevelop.org"
    }
}
