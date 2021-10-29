/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.2
import QtWebKit 3.0

StandardPage {
    ColumnLayout {
        id: options
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 30
        
        spacing: 10
        
        width: 200
    }
    
    Column {
        id: info
        anchors {
            top: parent.top
            left: options.right
            right: parent.right
            margins: 30
        }
        
        spacing: 10
        
        WebView {
            id: webview
            width: parent.width
            height: 200
            
            MouseArea { anchors.fill: parent; hoverEnabled: true }
        }
        
        Label {
            id: description
            width: parent.width
        }
        
        Button {
            text: i18n("Go!")
            onClicked: Qt.openUrlExternally(webview.url)
        }
        state: "kdevelop.org"
        states: [
            State {
                name: "userbase"
                PropertyChanges { target: webview; url: "https://userbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: i18n("Documentation for KDevelop users") }
            },
            State {
                name: "kdevelop.org"
                PropertyChanges { target: webview; url: "https://kdevelop.org" }
                PropertyChanges { target: description; text: i18n("Keep up with KDevelop's development") }
            },
            State {
                name: "techbase"
                PropertyChanges { target: webview; url: "https://techbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: i18n("Help us improve KDevelop") }
            }
        ]
    }
}
