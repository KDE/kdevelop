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

#include "projectxmltest.h"

#include <QtXml/QDomDocument>
#include "cmakexmlparser.h"
QTEST_MAIN(ProjectXmlTest)

ProjectXmlTest::ProjectXmlTest( QObject* parent )
 : QObject( parent )
{
}

ProjectXmlTest::~ProjectXmlTest()
{
}

void ProjectXmlTest::testEmptyProject()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    ProjectInfo pi = m_parser.parseProject( doc );
    QVERIFY( pi.name.isEmpty() );
}

void ProjectXmlTest::testEmptyProject_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("empty project 1") << "<project></project>";
    QTest::newRow("empty project 2") << "<project/>";
}

void ProjectXmlTest::testFullProject()
{
    QFETCH(QString, xml);
    QFETCH(QString, projectname);
    QFETCH(int, foldercount);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    ProjectInfo pi = m_parser.parseProject( doc );
    QVERIFY( pi.name == projectname );
    QVERIFY( pi.folders.count() == foldercount );
}

void ProjectXmlTest::testFullProject_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("projectname");
    QTest::addColumn<int>("foldercount");
    QTest::newRow("full project") << "<project name=\"foo\"><folder name=\"foobar\">"
            "<includes><include>/path/to/neato/include/</include></includes>"
            "<definitions><define>-DQT_NO_STL</define></definitions>"
            "</folder></project>" << "foo" << 1;
}


#include "projectxmltest.moc"
