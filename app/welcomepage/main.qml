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

import QtQuick 1.0

Rectangle
{
    id: root
    width: 360
    height: 360
    property Item current: null
    property string areaName: area
    
    onAreaNameChanged: {
        if(current!=null) {
            current.destroy(1000)
        }
        var component = Qt.createComponent(":/"+area+".qml")
        console.log(component)
        if (component.status == Component.Ready) {
            console.log(component)
            current = component.createObject(root);
            current.anchors.fill=root
        } else
            console.log("Error loading component ", component.errorString());
    }
}
