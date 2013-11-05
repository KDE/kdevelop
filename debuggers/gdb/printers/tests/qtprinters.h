/*
   Copyright 2010 Niko Sams <niko.sams@gmail.com>

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

#ifndef QTPRINTERSTEST_H
#define QTPRINTERSTEST_H

#include <QObject>

namespace GDBDebugger
{

class QtPrintersTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQString();
    void testQByteArray();
    void testQListContainer_data();
    void testQListContainer();
    void testQMapInt();
    void testQMapString();
    void testQMapStringBool();
    void testQDate();
    void testQTime();
    void testQDateTime();
    void testQUrl();
    void testQHashInt();
    void testQHashString();
    void testQSetInt();
    void testQSetString();
    void testQChar();
    void testQUuid();
    void testKTextEditorTypes();
};

}

#endif
