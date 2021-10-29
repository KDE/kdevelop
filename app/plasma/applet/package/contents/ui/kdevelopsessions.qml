/*
    SPDX-FileCopyrightText: 2012 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.kdevelopsessions 1.0 as KDevelopSessions

Item {
   id: kdevelopSessions

    property int minimumWidth: 200
    property int minimumHeight: 150

    function popupEventSlot(shown) {
        if(shown)
            view.forceActiveFocus();
    }

    KDevelopSessions.SessionListModel {
        id: sessionsModel
    }


    Component.onCompleted: {
        plasmoid.popupIcon = "kdevelop";
        plasmoid.popupEvent.connect('popupEvent', popupEventSlot);
    }

   PlasmaCore.Svg {
       id: lineSvg
       imagePath: "widgets/line"
    }

    Row {
        id: headerRow
        anchors { left: parent.left; right: parent.right }

        PlasmaCore.IconItem {
            id: appIcon
            source: "kdevelop"
            width: 32
            height: 32
        }

        PlasmaComponents.Label {
            id: header
            text: i18n("KDevelop Sessions")
            horizontalAlignment: Text.AlignHCenter | Text.AlignVCenter
            width: parent.width - appIcon.width * 2
            height: parent.height
        }
    }

    PlasmaCore.SvgItem {
        id: separator

        anchors { left: headerRow.left; right: headerRow.right; top: headerRow.bottom }
        svg: lineSvg
        elementId: "horizontal-line"
        height: lineSvg.elementSize("horizontal-line").height
    }

    Text {
        id: textMetric
        visible: false
        // translated but not used, we just need length/height
        text: i18n("Arbitrary String Which Says Something")
    }
    Keys.forwardTo: view

    ListView {
        id: view

        anchors { left: parent.left; right: scrollBar.left; bottom: parent.bottom; top: separator.bottom; topMargin: 5 }

        model: PlasmaCore.SortFilterModel {
            id: filterModel
            sourceModel: sessionsModel
        }
        clip: true
        focus: true

        delegate: Item {
            id: listdelegate
            height: textMetric.paintedHeight * 2

            anchors {
                left: parent.left
                right: parent.right
            }

            function openSession() {
                sessionsModel.openSession(sessionId);
                plasmoid.expanded = false
            }

            PlasmaComponents.Label {
                id: sessionText

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    right: parent.right
                    leftMargin: 10
                    rightMargin: 10
                }

                verticalAlignment: Text.AlignVCenter
                text: model.display
                elide: Text.ElideRight
            }

            MouseArea {
                height: parent.height + 15
                anchors { left: parent.left; right: parent.right;}
                hoverEnabled: true

                onClicked: {
                    openSession();
                }

                onEntered: {
                    view.currentIndex = index
                    view.highlightItem.opacity = 1
                }

                onExited: {
                    view.highlightItem.opacity = 0
                }
            }
            Keys.onPressed: {
            if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                openSession();
            }
        }
        Keys.onReturnPressed: { currentItem.Keys.onPressed(event); }
        Keys.onEnterPressed: { currentItem.Keys.onPressed(event); }
        highlight: PlasmaComponents.Highlight {
            hover: true
        }

        highlightMoveDuration: PlasmaCore.Units.shortDuration
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar

        anchors { bottom: parent.bottom; top: separator.top; right: parent.right }

        orientation: Qt.Vertical
        stepSize: view.count / 4
        scrollButtonInterval: view.count / 4

        flickableItem: view
    }

    Plasmoid.onExpandedChanged: {
        if (plasmoid.expanded) {
            view.currentIndex = 0;
            view.highlightItem.opacity = 1;
        }
    }
}
