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

#include "folderxmltest.h"

QTEST_MAIN(SimpleFolderXmlTest)

void SimpleFolderXmlTest::testEmptyFolder()
{
}

void SimpleFolderXmlTest::testEmptyFolder_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("empty folder 1") << "<folder></folder>";
    QTest::newRow("empty folder 2") << "<folder/>";
}

void SimpleFolderXmlTest::testNonEmptyFolder()
{
    QVERIFY(false);
}

void SimpleFolderXmlTest::testNonEmptyFolder_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("nonempty1") << "<folder name=\"foo\"></folder>"; 
}

void SimpleFolderXmlTest::testFolderWithSubFolders()
{
    QVERIFY(false);
}

void SimpleFolderXmlTest::testFolderWithSubFolders_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("sub1") << "<folder name=\"foo\"><tag1/><tag2/>"
                             "<folder name=\"bar\"></folder></folder>";
}

#include "folderxmltest.moc"
