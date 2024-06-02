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

    fullRepresentation: FocusScope {
        anchors.fill: parent

        Connections {
            target: kdevelopSessions
            function onExpandedChanged() {
                if (kdevelopSessions.expanded) {
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
            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }

        Text {
            id: textMetric
            visible: false
            // translated but not used, we just need length/height
            text: i18n("Arbitrary String Which Says Something")
        }

        QQC2.ScrollView {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; top: separator.bottom; topMargin: Kirigami.Units.smallSpacing}

            ListView {
                id: view

                model: KItemModels.KSortFilterProxyModel {
                    id: filterModel
                    sourceModel: sessionsModel
                }
                clip: true
                focus: true
                keyNavigationWraps: true

                delegate: PlasmaComponents.ItemDelegate {
                    id: listdelegate

                    width: ListView.view.width
                    height: textMetric.paintedHeight * 2

                    hoverEnabled: true
                    text: model.display

                    Accessible.role: Accessible.Button

                    function openSession() {
                        sessionsModel.openSession(sessionId);
                        kdevelopSessions.expanded = false
                    }

                    onClicked: {
                        openSession();
                    }

                    onHoveredChanged: {
                        if (hovered) {
                            view.currentIndex = index;
                        }
                    }
                }

                highlight: PlasmaExtras.Highlight {}

                highlightMoveDuration: Kirigami.Units.longDuration
                highlightMoveVelocity: 1
            }
        }
    }
}
