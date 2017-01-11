/* KDevelop
 *
 * Copyright 2017 Kevin Funk <kfunk@kde.org>
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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2
import QtQuick.XmlListModel 2.0

import "storage.js" as Storage

ListView {
    id: root

    /// Update interval (in minutes) in which the news feed is polled
    property int updateInterval: 24 * 60 * 1000 // 24 hours
    /// Max age (in minutes) of a news entry so it is shown in the list view
    /// TODO: Implement me
    property int maxNewsAge: 3 * 30 * 24 * 60 // 3 months
    /// Max age (in minutes) of a news entry so it is considered 'new' (thus highlighted with a bold font)
    property int maxHighlightedNewsAge: 30 * 24 * 60 // a month

    readonly property string feedUrl: "https://www.kdevelop.org/news/feed"
    readonly property bool loading: newsFeedSyncModel.status === XmlListModel.Loading

    /// Returns a date parsed from the pubDate
    function parsePubDate(pubDate) {
        // We need to modify the pubDate read from the RSS feed
        // so the JavaScript Date object can interpret it
        var d = pubDate.replace(',','').split(' ');
        if (d.length != 6)
            return new Date(NaN);

        return new Date([d[0], d[2], d[1], d[3], d[4], 'GMT' + d[5]].join(' '));
    }

    // there's no builtin function for this(?)
    function toMap(obj) {
        var map = {};
        for (var k in obj) {
            map[k] = obj[k];
        }
        return map;
    }

    function minutesSince(date) {
        return !isNaN(date) ? Math.floor(Number((new Date() - date)) / 60000) : -1;
    }

    function loadFromCache() {
        newsFeedOfflineModel.clear()

        var data = Storage.get("newsFeedOfflineModelData", null);
        if (data) {
            var newsEntries = JSON.parse(data);
            for (var i = 0; i < newsEntries.length; ++i) {
                newsFeedOfflineModel.append(newsEntries[i]);
            }
        }
        root.positionViewAtBeginning()
    }
    function saveToCache() {
        var newsEntries = [];
        for (var i = 0; i < newsFeedSyncModel.count; ++i) {
            var entry = newsFeedSyncModel.get(i);
            newsEntries.push(toMap(entry));
        }
        Storage.set("newsFeedOfflineModelData", JSON.stringify(newsEntries));
        Storage.set("newsFeedLastFetchDate", JSON.stringify(new Date()));
    }

    spacing: 10

    // Note: this model is *not* attached to the the view -- it's merely used for fetching the RSS feed
    XmlListModel {
        id: newsFeedSyncModel

        property bool active: false

        source: active ? feedUrl : ""
        query: "/rss/channel/item"

        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "link"; query: "link/string()" }
        XmlRole { name: "pubDate"; query: "pubDate/string()" }

        onStatusChanged: {
            if (status == XmlListModel.Ready) {
                saveToCache();
                loadFromCache();
            } else if (status == XmlListModel.Error) {
                console.log("Failed to fetch news feed: " + errorString());
            }
        }
    }

    model: ListModel {
        id: newsFeedOfflineModel
    }

    delegate: Column {
        id: feedDelegate

        readonly property date publicationDate: parsePubDate(model.pubDate)
        readonly property int ageInMinutes: minutesSince(publicationDate)
        readonly property bool isNew: ageInMinutes != -1 && ageInMinutes < maxHighlightedNewsAge
        readonly property string dateString: isNaN(publicationDate.getDate()) ? model.pubDate : publicationDate.toLocaleDateString()

        x: 10
        width: parent.width - 2*x

        Link {
            width: parent.width

            text: model.title

            onClicked: Qt.openUrlExternally(model.link)
        }

        Label {
            width: parent.width

            font.bold: isNew
            font.pointSize: 8
            color: disabledPalette.windowText

            text: isNew ? i18nc("Example: Tue, 03 Jan 2017 10:00:00 (new)", "%1 (new)", dateString) : dateString
        }
    }

    BusyIndicator {
        id: busyIndicator

        height: newsHeading.height

        running: newsFeed.loading
    }

    Label {
        id: placeHolderLabel

        x: 10
        width: parent.width - 2*x

        text: i18n("No recent news")
        color: disabledPalette.windowText
        visible: root.count === 0 && !root.loading

        Behavior on opacity { NumberAnimation {} }
    }

    SystemPalette {
        id: disabledPalette
        colorGroup: SystemPalette.Disabled
    }

    function fetchFeed() {
        console.log("Fetching news feed")

        newsFeedSyncModel.active = true
        newsFeedSyncModel.reload()
    }

    Timer {
        id: delayedStartupTimer

        // delay loading a bit so it has no effect on the KDevelop startup
        interval: 3000
        running: true

        onTriggered: {
            // only fetch feed if items are out of date
            var lastFetchDate = new Date(JSON.parse(Storage.get("newsFeedLastFetchDate", null)));
            if (minutesSince(lastFetchDate) > root.updateInterval) {
                console.log("Last fetch of news feed was on " + lastFetchDate + ", updating now");
                root.fetchFeed();
            }
        }
    }

    Timer {
        id: reloadFeedTimer

        interval: root.updateInterval
        running: true
        repeat: true

        onTriggered: root.fetchFeed()
    }

    Component.onCompleted: loadFromCache()
}
