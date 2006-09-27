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
    QStringList includes;
    QStringList defines;
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
    QTest::newRow("sub2") << "<folder name=\"foo\"><folder name=\"bar\">"
                             "</folder></folder>";
}

void SimpleFolderXmlTest::testFolderWithIncludes()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    QVERIFY( doc.setContent( xml ) );
    QDomElement docElem = doc.documentElement();
    FolderInfo mainInfo;
    mainInfo.name = docElem.attribute( "name" );
    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull())
        {
            if ( e.tagName() == "includes" )
            {
                QDomNode in = e.firstChild();
                while ( !in.isNull() )
                {
                    QDomElement ie = in.toElement(); // try to convert the node to an element.
                    if ( !ie.isNull() )
                    {
                        if ( ie.tagName() == "include" )
                            mainInfo.includes.append( ie.text() );
                    }
                    in = in.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }
    QVERIFY( mainInfo.name.isEmpty() == false );
    QVERIFY( mainInfo.includes.isEmpty() == false );
    QVERIFY( mainInfo.includes.count() == 1 );
}

void SimpleFolderXmlTest::testFolderWithIncludes_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("includes1") << "<folder name=\"foo\"><includes>"
                                  "<include>/path/to/neato/place</include>"
                                  "</includes></folder>";
}

void SimpleFolderXmlTest::testFolderWithDefines()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    QVERIFY( doc.setContent( xml ) );
    QDomElement docElem = doc.documentElement();
    FolderInfo mainInfo;
    mainInfo.name = docElem.attribute( "name" );
    QDomNode n = docElem.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
        {
            if ( e.tagName() == "definitions" )
            {
                QDomNode dn = e.firstChild();
                while ( !dn.isNull() )
                {
                    QDomElement de = dn.toElement(); // try to convert the node to an element.
                    if ( !de.isNull() )
                    {
                        if ( de.tagName() == "define" )
                            mainInfo.defines.append( de.text() );
                    }
                    dn = dn.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }
    QVERIFY( mainInfo.name.isEmpty() == false );
    QVERIFY( mainInfo.defines.isEmpty() == false );
    QVERIFY( mainInfo.defines.count() == 1 );
}

void SimpleFolderXmlTest::testFolderWithDefines_data()
{
    QTest::addColumn<QString>("xml");
    QTest::newRow("defines1") << "<folder name=\"foo\"><definitions>"
                                  "<define>-DQT_NO_STL</define>"
                                  "</definitions></folder>";
}

void SimpleFolderXmlTest::fullFolderTest()
{
    QFETCH(QString, xml);
    QDomDocument doc;
    QVERIFY( doc.setContent( xml ) );
    QDomElement docElem = doc.documentElement();
    FolderInfo mainInfo;
    mainInfo.name = docElem.attribute( "name" );
    QDomNode n = docElem.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
        {
            if ( e.tagName() == "definitions" )
            {
                QDomNode dn = e.firstChild();
                while ( !dn.isNull() )
                {
                    QDomElement de = dn.toElement(); // try to convert the node to an element.
                    if ( !de.isNull() )
                    {
                        if ( de.tagName() == "define" )
                            mainInfo.defines.append( de.text() );
                    }
                    dn = dn.nextSibling();
                }
            }

            if ( e.tagName() == "includes" )
            {
                QDomNode in = e.firstChild();
                while ( !in.isNull() )
                {
                    QDomElement ie = in.toElement(); // try to convert the node to an element.
                    if ( !ie.isNull() )
                    {
                        if ( ie.tagName() == "include" )
                            mainInfo.includes.append( ie.text() );
                    }
                    in = in.nextSibling();
                }
            }

            if ( e.tagName() == "folder" )
            {
                FolderInfo fi;
                fi.name = e.attribute("name");
                mainInfo.subFolders.append(fi);
            }

        }
        n = n.nextSibling();
    }
    QFETCH( QString, folderName );
    QFETCH( int, numIncludes );
    QFETCH( int, numDefines );
    QFETCH( int, numFolders );
    QFETCH( QStringList, includeValues );
    QFETCH( QStringList, defineValues );

    QVERIFY( mainInfo.name == folderName );
    QVERIFY( mainInfo.subFolders.count() == numFolders );
    QVERIFY( mainInfo.includes.count() == numIncludes );
    QVERIFY( mainInfo.defines.count() == numDefines );
    QStringList::iterator checkIt = mainInfo.includes.begin();
    QStringList::iterator resultIt = includeValues.begin();
    for( ; checkIt != mainInfo.includes.end(), resultIt != includeValues.end();
         ++checkIt, ++resultIt )
    {
        QVERIFY( (*checkIt) == (*resultIt) );
    }

    checkIt = mainInfo.defines.begin();
    resultIt = defineValues.begin();
    for( ; checkIt != mainInfo.defines.end(), resultIt != defineValues.end();
         ++checkIt, ++resultIt )
    {
        QVERIFY( (*checkIt) == (*resultIt) );
    }
}

void SimpleFolderXmlTest::fullFolderTest_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("folderName");
    QTest::addColumn<int>("numIncludes");
    QTest::addColumn<int>("numDefines");
    QTest::addColumn<int>("numFolders");
    QTest::addColumn<QStringList>("includeValues");
    QTest::addColumn<QStringList>("defineValues");

    QStringList includeList;
    includeList.append( "/path/to/neato/place" );
    QStringList defineList;
    defineList.append( "-DQT_NO_SQL" );
    QTest::newRow("folder1") << "<folder name=\"foo\"><folder name=\"bar\"></folder>"
                                "<includes><include>/path/to/neato/place</include></includes>"
                                "<definitions><define>-DQT_NO_SQL</define></definitions>"
                                "</folder>"
                             << "foo" << 1 << 1 << 1 << includeList << defineList ;

}

#include "folderxmltest.moc"
