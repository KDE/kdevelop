/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
