/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef LEXERTEST_H
#define LEXERTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtCore/QList>


class LexerTest : public QObject
{
        Q_OBJECT
    public:
        LexerTest( QObject* parent = 0 );
        ~LexerTest();
    private slots:
        void init();
        void cleanup();
        void varAssignment();
        void varAssignment_data();
        void operators();
        void operators_data();
        void scope();
        void scope_data();
        void functions();
        void functions_data();

};

#endif

