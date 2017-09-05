/* KDevelop
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
                PropertyChanges { target: webview; url: "http://userbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: i18n("Documentation for KDevelop users") }
            },
            State {
                name: "kdevelop.org"
                PropertyChanges { target: webview; url: "http://kdevelop.org" }
                PropertyChanges { target: description; text: i18n("Keep up with KDevelop's development") }
            },
            State {
                name: "techbase"
                PropertyChanges { target: webview; url: "http://techbase.kde.org/KDevelop" }
                PropertyChanges { target: description; text: i18n("Help us improve KDevelop") }
            }
        ]
    }
}
