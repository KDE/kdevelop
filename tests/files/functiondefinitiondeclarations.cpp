/* This file is part of KDevelop
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a);
/// "declaration" : { "null" : true }
void someFunc(int a, int f) { }
void someFunc(int a) { }

class SomeClass
{
    /// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 0 },
    /// "range" : "[(39, 16), (39, 29)]"
    /// }
    void someClassFunc();
    /// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 2 } }
    void someClassFunc(int arg, char arg);
    /// "qualifiedIdentifier" : "SomeClass::anotherClassFunc"
    void anotherClassFunc();
    /// "definition" : { "null" : true }
    void anotherClassFunc(int foo);
};


void SomeClass::someClassFunc() { }
void SomeClass::someClassFunc(int arg, char arg) { }
void SomeClass::anotherClassFunc() { }
