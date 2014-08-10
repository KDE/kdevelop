/*************************************************************************************
 *  Copyright (C) 2013 by Sven Brauch <svenbrauch@gmail.com>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/
import QtQuick 2.2

PropertyWidget {
    id: root
    width: 150
    height: 120
    value: "1"
    function sliderToValue(slider) {
        return slider*31;
    }
    function valueToSlider(value) {
        return value/31;
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Duration in milliseconds")
        color: "white"
        opacity: 0.8
    }
    Slider {
        z: 20
        initialValue: valueToSlider(parent.value)
        width: 100
        height: 12
    }
    Rectangle {
        id: sample
        anchors.centerIn: parent
        color: "#1f61a8"
        width: 50
        height: 50
        SequentialAnimation {
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: root.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: 20; duration: root.value }
            }
            PauseAnimation { duration: 500 }
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: root.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: -20; duration: root.value }
            }
            PauseAnimation { duration: 500 }
            running: true
            loops: Animation.Infinite
        }
    }
}
