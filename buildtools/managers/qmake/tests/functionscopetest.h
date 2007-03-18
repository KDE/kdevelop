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

#ifndef FUNCTIONSCOPETEST_H
#define FUNCTIONSCOPETEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtCore/QList>


namespace QMake
{
    class StatementAST;
    class ProjectAST;
    class FunctionArgAST;
}

class FunctionScopeTest : public QObject
{
        Q_OBJECT
    public:
        FunctionScopeTest( QObject* parent = 0 );
        ~FunctionScopeTest();
    private slots:
        void init();
        void cleanup();
        void execSimpleFunc();
        void execSimpleFunc_data();
        void execBasicFunc();
        void execBasicFunc_data();
    private:
        bool matchArguments( QList<QMake::FunctionArgAST*>,
                             QList<QMake::FunctionArgAST*> );
        QMake::ProjectAST* ast;

};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
