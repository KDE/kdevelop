/* KDevelop CMake Support
 *
 * Copyright 2011 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

import QtQuick 1.0
import QtWebKit 1.0

StandardPage {
    color: "#CDDFE3"
    
    Column {
        id: options
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 30
        
        spacing: 10
        
        width: 200
        Link { text: qsTr("Official Website"); onClicked: info.state="kdevelop.org" }
        Link { text: qsTr("Userbase"); onClicked: info.state="userbase" }
        Link { text: qsTr("Techbase"); onClicked: info.state="techbase" }
        Link { text: qsTr("Handbook"); onClicked: kdev.retrieveMenuAction("help/help_contents").trigger() }
    }
    
    Column {
        id: info
        anchors.top: parent.top
        anchors.left: options.right
        anchors.margins: 30
        width: parent.width-200
        
        spacing: 10
        
        WebView {
            id: webview
            width: parent.width-3*parent.anchors.margins
            height: 200
        }
        
        Text {
            id: description
            text: ""
        }
        
        Link {
            text: "Go!"
            onClicked: kdev.openUrl(webview.url)
        }
        states: [
            State {
                name: "userbase"
                PropertyChanges { target: webview; url: "http://userbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: qsTr("Documentation for KDevelop users") }
            },
            State {
                name: "kdevelop.org"
                PropertyChanges { target: webview; url: "http://kdevelop.org" }
                PropertyChanges { target: description; text: qsTr("Keep up with KDevelop's development!") }
            },
            State {
                name: "techbase"
                PropertyChanges { target: webview; url: "http://techbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: qsTr("Help us improve KDevelop!") }
            }
        ]
    }
    
    Component.onCompleted: info.state="kdevelop.org"
}
