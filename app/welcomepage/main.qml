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

Rectangle
{
    id: root
    width: 360
    height: 360
    property Item current
    
    function areasComponent(area)
    {
        var component = Qt.createComponent(":/"+area+".qml")
        console.log(component)
        if (component.status == Component.Ready) {
            console.log(component)
            return component.createObject(root);
        } else
            throw("Error loading component "+component.errorString());
    }
    
    state: area
    onStateChanged: {
        if(current)
            current.visible=false
        
        try { current = areasComponent(area) }
        catch(e) { console.log(e); }
        current.anchors.fill=root
    }
}
