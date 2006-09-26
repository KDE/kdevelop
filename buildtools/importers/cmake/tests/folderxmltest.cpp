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
#include <QtXml/QDomDocument>

QTEST_MAIN(SimpleFolderXmlTest)

struct FolderInfo
{
    QString name;
    QList<FolderInfo> subFolders;
};

void SimpleFolderXmlTest::testNonValidFolder()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    QDomElement docElem = doc.documentElement();
    QVERIFY( docElem.tagName() == "folder" );
    FolderInfo fi;
    fi.name = docElem.attribute( "name" );
    QVERIFY( fi.name.isEmpty() );
}

void SimpleFolderXmlTest::testNonValidFolder_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("empty folder 1") << "<folder></folder>";
    QTest::newRow("empty folder 2") << "<folder/>";
}

void SimpleFolderXmlTest::testEmptyFolder()
{
    QFETCH(QString, xml);
    QFETCH(QString, foldername);
    QDomDocument doc;
    if ( ! doc.setContent( xml ) )
        QFAIL("Unable to set XML contents");
    QDomElement docElem = doc.documentElement();
    QVERIFY( docElem.tagName() == "folder" );
    FolderInfo fi;
    fi.name = docElem.attribute( "name" );
    QVERIFY( !fi.name.isEmpty() );
    QVERIFY( fi.name == foldername );
}

void SimpleFolderXmlTest::testEmptyFolder_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("foldername");
    QTest::newRow("nonempty1") << "<folder name=\"foo\"></folder>" << "foo";
}

void SimpleFolderXmlTest::testFolderWithSubFolders()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    QVERIFY( doc.setContent( xml ) );
    QDomElement docElem = doc.documentElement();
    FolderInfo mainInfo;
    mainInfo.name = docElem.attribute( "name" );
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if ( e.tagName() == "folder" )
            {
                FolderInfo fi;
                fi.name = e.attribute("name");
                mainInfo.subFolders.append(fi);
            }
        }
        n = n.nextSibling();
    }
    QVERIFY( mainInfo.subFolders.isEmpty() == false );
    QVERIFY( mainInfo.subFolders.count() == 1 );
}

void SimpleFolderXmlTest::testFolderWithSubFolders_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("sub1") << "<folder name=\"foo\"><tag1/><tag2/>"
                             "<folder name=\"bar\"></folder></folder>";
}

#include "folderxmltest.moc"
