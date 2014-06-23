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

/// "internalContext" : { "localDeclarationCount" : 1, "findDeclarations" : { "a" : { "defaultParameter" : "1" } } }
int foo(int a = 1);

/// "internalContext" : { "localDeclarationCount" : 1, "findDeclarations" : { "a" : { "defaultParameter" : "1" } } }
int foodef(int a = 1) {}

/// "internalContext" : { "localDeclarationCount" : 3, "findDeclarations" : {
///    "a" : { "defaultParameter" : "4" },
///    "b" : { "defaultParameter" : "3" }
/// } }
int foo(int sure, int a = 4, int b = 3);
