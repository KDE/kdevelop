/*
    SPDX-FileCopyrightText: 2012 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls as QQC2
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.ksvg as KSvg

import org.kde.plasma.private.kdevelopsessions as KDevelopSessions

PlasmoidItem {
   id: kdevelopSessions

    switchWidth: Kirigami.Units.gridUnit * 11
    switchHeight: Kirigami.Units.gridUnit * 9

    Layout.minimumWidth: Kirigami.Units.gridUnit * 12
    Layout.minimumHeight: Kirigami.Units.gridUnit * 10

    KDevelopSessions.SessionListModel {
        id: sessionsModel
    }

    KSvg.Svg {
       id: lineSvg
       imagePath: "widgets/line"
    }

    fullRepresentation: FocusScope {
        anchors.fill: parent

        Connections {
            target: kdevelopSessions
            function onExpandedChanged() {
                if (kdevelopSessions.expanded) {
                    view.currentIndex = 0;
                    view.highlightItem.opacity = 1;
                    view.forceActiveFocus();
                }
            }
        }

        Row {
            id: headerRow
            anchors { left: parent.left; right: parent.right }

            Kirigami.Icon {
                id: appIcon
                source: "kdevelop"
                width: Kirigami.Units.iconSizes.medium
                height: Kirigami.Units.iconSizes.medium
            }

            PlasmaComponents.Label {
                id: header
                text: i18n("KDevelop Sessions")
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - appIcon.width * 2
                height: parent.height
            }
        }

        KSvg.SvgItem {
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

            model: KItemModels.KSortFilterProxyModel {
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

            QQC2.ScrollBar.vertical: scrollBar

            Keys.onReturnPressed: { currentItem.Keys.onPressed(event); }
            Keys.onEnterPressed: { currentItem.Keys.onPressed(event); }
            highlight: PlasmaExtras.Highlight {
                hovered: true
            }

            highlightMoveDuration: Kirigami.Units.shortDuration
        }

        PlasmaComponents.ScrollBar {
            id: scrollBar

            anchors { bottom: parent.bottom; top: separator.top; right: parent.right }

            orientation: Qt.Vertical
            stepSize: view.count / 4
        }
    }
}
