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

#ifndef QMAKETESTMACROS_H
#define QMAKETESTMACROS_H

#define FUNCDEF( funcname )\
        void  funcname();\
        void  funcname##_data();

#define DATAFUNCIMPL( classname, funcname, data ) \
void classname::funcname##_data()\
{\
    QTest::addColumn<QString>( "project" );\
    QTest::addColumn<QString>( "output" );\
    QTest::newRow( "row1" ) << data << data; \
}

#define BEGINTESTFUNCIMPL( classname, funcname, astcount ) \
void classname::funcname()\
{\
    QFETCH( QString, project );\
    QFETCH( QString, output );\
    int ret = QMake::Driver::parseString( project, ast );\
    QVERIFY( ret == 0 );\
    QVERIFY( ast->statements().count() == astcount );\
    QString result;\
    ast->writeToString(result);\
    QVERIFY( result == output );


#define ENDTESTFUNCIMPL }

#endif
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

