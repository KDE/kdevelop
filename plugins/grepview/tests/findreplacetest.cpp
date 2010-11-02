/***************************************************************************
*   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
*   bernd@kdevelop.org                                                    *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QTest>
#include <QRegExp>

#include <ktemporaryfile.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "findreplacetest.h"
#include "../grepjob.h"
//#include "../grepviewplugin.h"

void FindReplaceTest::initTestCase()
{
    KDevelop::AutoTestShell::init();
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::NoUi);
    //m_plugin = new GitPlugin(m_testCore);
}

void FindReplaceTest::cleanupTestCase()
{
    //delete m_plugin;
    m_testCore->cleanup();
    delete m_testCore;
}

void FindReplaceTest::testFind_data()
{
    QTest::addColumn<QString>("subject");
    QTest::addColumn<QRegExp>("search");
    QTest::addColumn<MatchList>("matches");
    
    QTest::newRow("Basic") << "foobar" << QRegExp("foo")
                           << (MatchList() << Match(0, 0, 3));
    QTest::newRow("Multiple matches") << "foobar\nbar\nbarfoo" << QRegExp("foo")
                           << (MatchList() << Match(0, 0, 3) << Match(2, 3, 6));
    QTest::newRow("Multiple on same line") << "foobarbaz" << QRegExp("ba")
                           << (MatchList() << Match(0, 3, 5) << Match(0, 6, 8));
    QTest::newRow("Multiple sticked together") << "foofoobar" << QRegExp("foo")
                           << (MatchList() << Match(0, 0, 3) << Match(0, 3, 6));
    QTest::newRow("RegExp (member call)") << "foo->bar ();\nbar();" << QRegExp("\\->\\s*\\b(bar)\\b\\s*\\(")
                           << (MatchList() << Match(0, 3, 10));
    // the matching must be sertarted after the last previous match
    QTest::newRow("RegExp (greedy match)") << "foofooo" << QRegExp("[o]+")
                           << (MatchList() << Match(0, 1, 3) << Match(0, 4, 7));
}

void FindReplaceTest::testFind()
{
    QFETCH(QString,   subject);
    QFETCH(QRegExp,   search);
    QFETCH(MatchList, matches);
    
    KTemporaryFile file;
    QVERIFY(file.open());
    file.write(subject.toUtf8());
    file.close();
    
    GrepOutputItem::List actualMatches = grepFile(file.fileName(), search, "");
    
    QCOMPARE(actualMatches.length(), matches.length());
    
    for(int i=0; i<matches.length(); i++)
    {
        QCOMPARE(actualMatches[i].change()->m_range.start.line,   matches[i].line);
        QCOMPARE(actualMatches[i].change()->m_range.start.column, matches[i].start);
        QCOMPARE(actualMatches[i].change()->m_range.end.column,   matches[i].end);
    }
    
    // check that file has not been altered by grepFile
    QVERIFY(file.open());
    QCOMPARE(QString(file.readAll()), subject);
}

QTEST_MAIN(FindReplaceTest);