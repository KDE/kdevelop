/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef PARSETEST_H
#define PARSETEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

class ParseTest : public QObject
{
        Q_OBJECT
    public:
        ParseTest( QObject* parent = 0 );
        ~ParseTest();
    private slots:
        void successSimpleProject();
        void successSimpleProject_data();
        void failSimpleProject();
        void failSimpleProject_data();
        void successFullProject();
        void successFullProject_data();
        void failFullProject();
        void failFullProject_data();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

