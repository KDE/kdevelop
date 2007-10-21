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

#ifndef SCOPETEST_H
#define SCOPETEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtCore/QList>


namespace QMake
{
    class StatementAST;
    class ProjectAST;
    class FunctionArgAST;
}

class ScopeTest : public QObject
{
        Q_OBJECT
    public:
        ScopeTest( QObject* parent = 0 );
        ~ScopeTest();
    private slots:
        void init();
        void cleanup();
        void basicScope();
        void basicScope_data();
        void basicScopeBrace();
        void basicScopeBrace_data();
        void nestedScope();
        void nestedScope_data();
        void missingStatement();
        void missingStatement_data();
        void missingColon();
        void missingColon_data();
    private:
        QMake::ProjectAST* ast;

};

#endif

