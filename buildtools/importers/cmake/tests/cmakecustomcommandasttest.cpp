/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "cmakecustomcommandasttest.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( CustomCommandAstTests )

void CustomCommandAstTests::testParsing()
{
//    QFETCH( CMakeFunctionDesc, func );
    QFAIL( "no magic" );
}

void CustomCommandAstTests::testParsing_data()
{
    QTest::addColumn<CMakeFunctionDesc>( "function" );
    CMakeFunctionDesc func;
    func.name = "add_custom_command";
    QStringList argList;
    argList << "output" << "foo" << "command" << "bar";
    func.addArguments( argList );
    func.filePath = QString();
    func.line = 0;

    CMakeFunctionDesc func1;
    func1.name = "add_custom_command";
    QStringList argList1;
    argList << "output" << "foo" << "command" << "bar";
    argList << "main_dependency" << "dep1" << "depends" << "dep1" << "dep2";
    argList << "working_directory" << "dir1" << "comment" << "some comment";
    argList << "verbatim" << "append";
    func1.addArguments( argList1 );

    CMakeFunctionDesc func2;
    func2.name = "ADD_CUSTOM_COMMAND";
    QStringList argList2;
    argList << "OUTPUT" << "foo" << "COMMAND" << "bar";
    argList << "MAIN_DEPENDENCY" << "dep1" << "DEPENDS" << "dep1" << "dep2";
    argList << "WORKING_DIRECTORY" << "dir1" << "COMMENT" << "some comment";
    argList << "VERBATIM" << "APPEND";
    func2.addArguments( argList2 );

    QTest::newRow( "no optional" ) << func;
    QTest::newRow( "all optional" ) << func1;
    QTest::newRow( "all optional uppercase" ) << func2;

}

#include "cmakecustomcommandasttest.moc"
