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

#include "cmake_cmakeprojectvisitor_test.h"
#include "cmakeast.h"
#include "cmakeprojectvisitor.h"

QTEST_MAIN( CMakeProjectVisitorTest )

void CMakeProjectVisitorTest::testGoodParse()
{
}

void CMakeProjectVisitorTest::testGoodParse_data()
{
}

void CMakeProjectVisitorTest::testBadParse()
{
}

void CMakeProjectVisitorTest::testBadParse_data()
{
}

void CMakeProjectVisitorTest::testVariables()
{
	QFETCH(QString, input);
	QFETCH(bool, containsVariable);
	QFETCH(QString, result);
	
	QHash<QString, QStringList> vars;
	vars.insert("MY_VAR", QStringList("val"));

	QCOMPARE(CMakeProjectVisitor::hasVariable(input), containsVariable);
	QCOMPARE(CMakeProjectVisitor::resolveVariables(QStringList(input), &vars), result);
}

void CMakeProjectVisitorTest::testVariables_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("containsVariable");
	QTest::addColumn<QString>("result");
	
	QTest::newRow("A variable alone") << "${MY_VAR}" << true << "MY_VAR";
	QTest::newRow("Contains a variable") << "${MY_VAR}/lol" << true << "MY_VAR";
	QTest::newRow("Nothing") << "aaaa${aaaa" << false << "";
	
}

#include "cmake_cmakeprojectvisitor_test.moc"
