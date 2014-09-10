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
#include <ktempdir.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "test_findreplace.h"
#include "../grepjob.h"
#include "../grepviewplugin.h"
#include "../grepoutputmodel.h"

void FindReplaceTest::initTestCase()
{
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);
}

void FindReplaceTest::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
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
    // the matching must be started after the last previous match
    QTest::newRow("RegExp (greedy match)") << "foofooo" << QRegExp("[o]+")
                           << (MatchList() << Match(0, 1, 3) << Match(0, 4, 7));
    QTest::newRow("Matching EOL") << "foobar\nfoobar" << QRegExp("foo.*")
                           << (MatchList() << Match(0, 0, 6) << Match(1, 0, 6));
    QTest::newRow("Matching EOL (Windows style)") << "foobar\r\nfoobar" << QRegExp("foo.*")
                           << (MatchList() << Match(0, 0, 6) << Match(1, 0, 6));
    QTest::newRow("Empty lines handling") << "foo\n\n\n" << QRegExp("bar")
                           << (MatchList());
    QTest::newRow("Can match empty string (at EOL)") << "foobar\n" << QRegExp(".*")
                           << (MatchList() << Match(0, 0, 6));
    QTest::newRow("Matching empty string anywhere") << "foobar\n" << QRegExp("")
                           << (MatchList());
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

    GrepOutputItem::List actualMatches = grepFile(file.fileName(), search);

    QCOMPARE(actualMatches.length(), matches.length());

    for(int i=0; i<matches.length(); i++)
    {
        QCOMPARE(actualMatches[i].change()->m_range.start().line(),   matches[i].line);
        QCOMPARE(actualMatches[i].change()->m_range.start().column(), matches[i].start);
        QCOMPARE(actualMatches[i].change()->m_range.end().column(),   matches[i].end);
    }

    // check that file has not been altered by grepFile
    QVERIFY(file.open());
    QCOMPARE(QString(file.readAll()), subject);
}


void FindReplaceTest::testReplace_data()
{
    QTest::addColumn<FileList>("subject");
    QTest::addColumn<QString>("searchPattern");
    QTest::addColumn<QString>("searchTemplate");
    QTest::addColumn<QString>("replace");
    QTest::addColumn<QString>("replaceTemplate");
    QTest::addColumn<FileList>("result");

    QTest::newRow("Raw replace")
        << (FileList() << File("myfile.txt", "some text\nreplacement\nsome other test\n")
                       << File("otherfile.txt", "some replacement text\n\n"))
        << "replacement" << "%s"
        << "dummy"       << "%s"
        << (FileList() << File("myfile.txt", "some text\ndummy\nsome other test\n")
                       << File("otherfile.txt", "some dummy text\n\n"));

    // see bug: https://bugs.kde.org/show_bug.cgi?id=301362
    QTest::newRow("LF character replace")
        << (FileList() << File("somefile.txt", "hello world\\n"))
        << "\\\\n" << "%s"
        << "\\n\\n" << "%s"
        << (FileList() << File("somefile.txt", "hello world\\n\\n"));

    QTest::newRow("Template replace")
        << (FileList() << File("somefile.h",   "struct Foo {\n  void setFoo(int foo);\n};")
                       << File("somefile.cpp", "instance->setFoo(0);\n setFoo(0); /*not replaced*/"))
        << "setFoo" << "\\->\\s*\\b%s\\b\\s*\\("
        << "setBar" << "->%s("
        << (FileList() << File("somefile.h",   "struct Foo {\n  void setFoo(int foo);\n};")
                       << File("somefile.cpp", "instance->setBar(0);\n setFoo(0); /*not replaced*/"));

    QTest::newRow("Template with captures")
        << (FileList() << File("somefile.cpp", "inst::func(1, 2)\n otherInst :: func (\"foo\")\n func()"))
        << "func" << "([a-z0-9_$]+)\\s*::\\s*\\b%s\\b\\s*\\("
        << "REPL" << "\\1::%s("
        << (FileList() << File("somefile.cpp", "inst::REPL(1, 2)\n otherInst::REPL(\"foo\")\n func()"));

    QTest::newRow("Regexp pattern")
        << (FileList() << File("somefile.txt", "foobar\n foooobar\n fake"))
        << "f\\w*o" << "%s"
        << "FOO" << "%s"
        << (FileList() << File("somefile.txt", "FOObar\n FOObar\n fake"));
}


void FindReplaceTest::testReplace()
{
    QFETCH(FileList, subject);
    QFETCH(QString,  searchPattern);
    QFETCH(QString,  searchTemplate);
    QFETCH(QString,  replace);
    QFETCH(QString,  replaceTemplate);
    QFETCH(FileList, result);

    KTempDir tempDir;
    QDir     dir(tempDir.name());  // we need some convenience functions that are not in KTempDir

    foreach(File fileData, subject)
    {
        QFile file(dir.filePath(fileData.first));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QVERIFY(file.write(fileData.second.toUtf8()) != -1);
        file.close();
    }

    GrepJob *job = new GrepJob(this);
    GrepOutputModel *model = new GrepOutputModel(job);

    job->setOutputModel(model);
    job->setPatternString(searchPattern);
    job->setTemplateString(searchTemplate);
    job->setReplacementTemplateString(replaceTemplate);
    job->setFilesString("*");
    job->setExcludeString("");
    job->setDirectoryChoice(QList<QUrl>() << QUrl::fromLocalFile(dir.path()));
    job->setDepth(-1); // fully recursive
    job->setRegexpFlag(true);
    job->setCaseSensitive(true);
    job->setProjectFilesFlag(false);
    QVERIFY(job->exec());

    QVERIFY(model->hasResults());
    model->setReplacement(replace);
    model->makeItemsCheckable(true);
    model->doReplacements();

    foreach(File fileData, result)
    {
        QFile file(dir.filePath(fileData.first));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(QString(file.readAll()), fileData.second);
        file.close();
    }
    tempDir.unlink();
}


QTEST_MAIN(FindReplaceTest);
