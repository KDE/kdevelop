/*************************************************************************************
 *  Copyright (C) 2014 by Denis Steckelmacher <steckdenis@yahoo.fr>                  *
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

Module {
    // These declarations are normally provided by the QML runtime
    Component {
        name: "color"
        exports: ["color 1.0"]
    }
    Component {
        name: "date"
        exports: ["date 1.0"]
    }
    Component {
        name: "time"
        exports: ["time 1.0"]
    }
    Component {
        name: "rect"
        exports: ["rect 1.0"]
        prototype: "Rectangle"
    }
    Component {
        name: "font"
        exports: ["font 1.0"]
        prototype: "Font"
    }
    Component {
        name: "point"
        exports: ["point 1.0"]
        Property { name: "x"; type: "double" }
        Property { name: "y"; type: "double" }
    }
    Component {
        name: "size"
        exports: ["size 1.0"]
        Property { name: "width"; type: "double" }
        Property { name: "height"; type: "double" }
    }
    Component {
        name: "url"
        exports: ["url 1.0"]
    }
    Component {
        name: "vector2d"
        exports: ["vector2d 1.0"]
        Property { name: "x"; type: "double" }
        Property { name: "y"; type: "double" }
    }
    Component {
        name: "vector3d"
        exports: ["vector3d 1.0"]
        Property { name: "x"; type: "double" }
        Property { name: "y"; type: "double" }
        Property { name: "z"; type: "double" }
    }
    Component {
        name: "vector4d"
        exports: ["vector4d 1.0"]
        Property { name: "x"; type: "double" }
        Property { name: "y"; type: "double" }
        Property { name: "z"; type: "double" }
        Property { name: "w"; type: "double" }
    }
    Component {
        name: "quaternion"
        exports: ["quaternion 1.0"]
        Property { name: "x"; type: "double" }
        Property { name: "y"; type: "double" }
        Property { name: "z"; type: "double" }
        Property { name: "scalar"; type: "double" }
    }
}