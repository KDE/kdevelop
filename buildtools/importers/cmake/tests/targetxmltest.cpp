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
#include "targetxmltest.h"
#include <QtXml/QDomDocument>
#include "cmakexmlparser.h"
QTEST_MAIN(TargetXmlTest)

TargetXmlTest::TargetXmlTest(QObject *parent)
  : QObject(parent){
}


TargetXmlTest::~TargetXmlTest()
{
}

void TargetXmlTest::emptyTargetTest()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    TargetInfo ti = CMakeXmlParser::parseTarget( doc.documentElement() );
    QVERIFY( ti.name.isEmpty() );
    QVERIFY( ti.type.isEmpty() );
}

void TargetXmlTest::emptyTargetTest_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("row1") << "<target></target>";
    QTest::newRow("row2") << "<target/>";

}

void TargetXmlTest::noSourcesTargetTest()
{
    QFETCH(QString, xml);
    QFETCH(QString, name);
    QFETCH(QString, type);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    TargetInfo ti = CMakeXmlParser::parseTarget( doc.documentElement() );
    QVERIFY( ti.name == name );
    QVERIFY( ti.type == type );
    QVERIFY( ti.sources.isEmpty() );
}

void TargetXmlTest::noSourcesTargetTest_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("type");
    QTest::newRow("row1") << "<target name=\"foo\" type=\"bar\" />" << "foo" <<
"bar";
    QTest::newRow("row2") << "<target name=\"t1\" type=\"mytype\"></target>" <<
"t1" << "mytype";
}

void TargetXmlTest::fullTargetTest()
{
    QFETCH(QString, xml);
    QFETCH(QString, name);
    QFETCH(QString, type);
    QFETCH(QStringList, sources);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    TargetInfo ti = CMakeXmlParser::parseTarget( doc.documentElement() );
    QVERIFY( ti.name == name );
    QVERIFY( ti.type == type );
    QStringList::iterator checkIt = ti.sources.begin();
    QStringList::iterator resultIt = sources.begin();
    for( ; checkIt != ti.sources.end(), resultIt != sources.end();
         ++checkIt, ++resultIt )
    {
        QVERIFY( (*checkIt) == (*resultIt) );
    }
}

void TargetXmlTest::fullTargetTest_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QStringList>("sources");

    QStringList sources1;
    sources1.append("foo.h");
    QTest::newRow("row1") << "<target name=\"t1\" type=\"mytype\">"
                             "<sources><source>foo.h</source></sources>"
                             "</target>" << "t1" << "mytype" << sources1;

    QStringList sources2;
    sources2.append("foo.cxx");
    sources2.append("bar.c");
    QTest::newRow("row1") << "<target name=\"t1\" type=\"mytype\">"
                             "<sources><source>foo.cxx</source>"
                             "<source>bar.c</source></sources>"
                             "</target>" << "t1" << "mytype" << sources2;
}

#include "targetxmltest.moc"
