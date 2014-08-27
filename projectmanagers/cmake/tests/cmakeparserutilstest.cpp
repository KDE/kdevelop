/* KDevelop CMake Support
 *
 * Copyright 2008 Matt Rogers <mattr@kde.org>
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

#include "cmakeparserutilstest.h"
#include "cmakeparserutils.h"

QTEST_MAIN( CMakeParserUtilsTest )

void CMakeParserUtilsTest::validVersionsTest()
{
    bool ok = true;
    QList<int> versions;
    QFETCH(QString, version);
    versions = CMakeParserUtils::parseVersion(version, &ok);
    QVERIFY(ok == true);
    QVERIFY(versions.count() != 0);
}

void CMakeParserUtilsTest::validVersionsTest_data()
{
    QTest::addColumn<QString>("version");
    QTest::newRow("zero major version") << QString("0");
    QTest::newRow("small major version") << QString("2");
    QTest::newRow("larger major version") << QString("24");
    QTest::newRow("major.minor") << QString("2.9");
    QTest::newRow("major.minor.patch") << QString("12.3.2");
}

void CMakeParserUtilsTest::invalidVersionsTest()
{
    bool ok = true;
    QList<int> versions;
    QFETCH(QString, version);
    versions = CMakeParserUtils::parseVersion(version, &ok);
    QVERIFY(ok == false);
    QVERIFY(versions.count() == 0);
}

void CMakeParserUtilsTest::invalidVersionsTest_data()
{
    QTest::addColumn<QString>("version");
    QTest::newRow("letter version") << QString("A");
    QTest::newRow("major.minor") << QString("A.B");
    QTest::newRow("major.minor.patch") << QString("AA.B.C");
}


