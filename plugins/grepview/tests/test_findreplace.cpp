/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_findreplace.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QRegularExpression>

#include <QTemporaryFile>
#include <QTemporaryDir>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <util/filesystemhelpers.h>

#include "../grepjob.h"
#include "../grepviewplugin.h"
#include "../grepoutputmodel.h"

#include <vector>

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
    QTest::addColumn<QRegularExpression>("search");
    QTest::addColumn<MatchList>("matches");

    QTest::newRow("Basic") << "foobar" << QRegularExpression("foo")
                           << (MatchList() << Match(0, 0, 3));
    QTest::newRow("Multiple matches") << "foobar\nbar\nbarfoo" << QRegularExpression("foo")
                           << (MatchList() << Match(0, 0, 3) << Match(2, 3, 6));
    QTest::newRow("Multiple on same line") << "foobarbaz" << QRegularExpression("ba")
                           << (MatchList() << Match(0, 3, 5) << Match(0, 6, 8));
    QTest::newRow("Multiple sticked together") << "foofoobar" << QRegularExpression("foo")
                           << (MatchList() << Match(0, 0, 3) << Match(0, 3, 6));
    QTest::newRow("RegExp (member call)") << "foo->bar ();\nbar();" << QRegularExpression("\\->\\s*\\b(bar)\\b\\s*\\(")
                           << (MatchList() << Match(0, 3, 10));
    // the matching must be started after the last previous match
    QTest::newRow("RegExp (greedy match)") << "foofooo" << QRegularExpression("[o]+")
                           << (MatchList() << Match(0, 1, 3) << Match(0, 4, 7));
    QTest::newRow("Matching EOL") << "foobar\nfoobar" << QRegularExpression("foo.*")
                           << (MatchList() << Match(0, 0, 6) << Match(1, 0, 6));
    QTest::newRow("Matching EOL (Windows style)") << "foobar\r\nfoobar" << QRegularExpression("foo.*")
                           << (MatchList() << Match(0, 0, 6) << Match(1, 0, 6));
    QTest::newRow("Empty lines handling") << "foo\n\n\n" << QRegularExpression("bar")
                           << (MatchList());
    QTest::newRow("Can match empty string (at EOL)") << "foobar\n" << QRegularExpression(".*")
                           << (MatchList() << Match(0, 0, 6));
    QTest::newRow("Matching empty string anywhere") << "foobar\n" << QRegularExpression("")
                           << (MatchList());
}

void FindReplaceTest::testFind()
{
    QFETCH(QString,   subject);
    QFETCH(QRegularExpression,   search);
    QFETCH(MatchList, matches);

    QTemporaryFile file;
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

void FindReplaceTest::testIncludeExcludeFilters_data()
{
    struct Row{
        const char* dataTag;
        const char* files;
        const char* exclude;
        std::vector<const char*> unmatchedPaths;
        std::vector<const char*> matchedPaths;
    };

    const std::vector<Row> dataRows{
        Row{"Files filter",
        "*.cpp,*.cc,*.h,INSTALL",
        "",
        {"A", "cpp", ".cp", "a.c", "INSTAL", "oINSTALL", "d./h", "d.h/c", "u/INSTALL/v", "a.cpp/b.cp", "INSTALL/h", "a.h.c"},
        {"x/INSTALL", "x/.cpp", ".cc", "x.h", "t/s/r/.h/.cc", "INSTALL.cpp", "x/y/z/a/b/c.h", "y/b.cc", "a.hh.cc", "t.h.cc"}
    }, Row{"Exclude filter",
        "*",
        "/build/,/.git/,~",
        {"build/C", ".git/config", "~", "a/b/c/build/t/n", "build/me", "a/~/x", "a~b/c", "temp~", "a/p/test~", "a/build/b/c", "d/c/.git/t"},
        {"d/build", "a/b/.git", "x", "a.h", "a.git", "a/.gitignore/b", ".gitignore", "buildme/now", "to build/.git"}
    }, Row{"Files and Exclude filters",
        "*.a,*-b,*se",
        "/release/,/.*/,bak",
        {"release/x.a", ".git/q-b", "a-b.c", "bak.a", "abakse", "a/bak-b", "a/x.bak", "u/v/wbakxyz", "a/.g/se", "-/b"},
        {"a.a", "b-b", "a/release", ".a", "git/q-b", "se", "a/.se", "a/b.c/d-b", "Bse", "u/v/.a-b", "a/b/.ignorse", "ba.k/.a"}
    }, Row{"Matching case-insensitive",
        "A*b,*.Cd,*.AUX",
        "GiT,garble,B.CD",
        {"acbGArblE.cD", "git/a.b", ".git/x.cd", "b.Cd", "u/v/q", "u/v/bcd", "u/v/b.Cd", "garble.AUX"},
        {"Ab", "a.b", "u/v/ADB", "gi.cd", ".CD", "az.cd", "u/v/w/agb", "p.AuX", "u/q.aux"}
    }};

    QTest::addColumn<QString>("files");
    QTest::addColumn<QString>("exclude");
    QTest::addColumn<QStringList>("unmatchedPaths");
    QTest::addColumn<QStringList>("matchedPaths");

    for (const Row& row : dataRows) {
        const QStringList unmatchedPaths(row.unmatchedPaths.cbegin(), row.unmatchedPaths.cend());
        const QStringList matchedPaths(row.matchedPaths.cbegin(), row.matchedPaths.cend());
        QTest::newRow(row.dataTag) << QString{row.files} << QString{row.exclude} << unmatchedPaths << matchedPaths;
    }
}

void FindReplaceTest::testIncludeExcludeFilters()
{
    QFETCH(QString, files);
    QFETCH(QString, exclude);
    QFETCH(QStringList, unmatchedPaths);
    QFETCH(QStringList, matchedPaths);

    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), qPrintable("couldn't create temporary directory: " + tmpDir.errorString()));

    const QByteArray commonFileContents = "x";

    using FilesystemHelpers::makeAbsoluteCreateAndWrite;
    QString errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), unmatchedPaths, commonFileContents);
    if (errorPath.isEmpty()) {
        errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), matchedPaths, commonFileContents);
    }
    QVERIFY2(errorPath.isEmpty(), qPrintable("couldn't create or write to temporary file or directory " + errorPath));

    GrepJob job;
    GrepOutputModel model;
    job.setOutputModel(&model);
    job.setDirectoryChoice({QUrl::fromLocalFile(tmpDir.path())});

    GrepJobSettings settings;
    settings.projectFilesOnly = false;
    settings.caseSensitive = true;
    settings.regexp = false;
    settings.depth = -1; // fully recursive
    settings.pattern = commonFileContents;
    const QString verbatimTemplate = "%s";
    settings.searchTemplate = verbatimTemplate;
    settings.replacementTemplate = verbatimTemplate;
    settings.files = files;
    settings.exclude = exclude;
    job.setSettings(settings);

    QVERIFY(job.exec());

    QModelIndex index;
    const GrepOutputItem* previousItem = nullptr;
    while (true) {
        index = model.nextItemIndex(index);
        if (!index.isValid()) {
            break;
        }
        auto* const item = dynamic_cast<const GrepOutputItem*>(model.itemFromIndex(index));
        QVERIFY(item);
        QVERIFY(item->isText());
        if (item == previousItem) {
            break; // This must be the last match.
        }
        previousItem = item;

        const QString filename = item->filename();
        QVERIFY2(matchedPaths.contains(filename), qPrintable("unexpected matched file " + filename));
        QVERIFY2(matchedPaths.removeOne(filename), qPrintable("there must be exactly one text match for " + filename));
    }
    QVERIFY2(matchedPaths.empty(), qPrintable("these files should have been matched, but weren't: " + matchedPaths.join("; ")));
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
        << (FileList() << File(QStringLiteral("myfile.txt"), QStringLiteral("some text\nreplacement\nsome other test\n"))
                       << File(QStringLiteral("otherfile.txt"), QStringLiteral("some replacement text\n\n")))
        << "replacement" << "%s"
        << "dummy"       << "%s"
        << (FileList() << File(QStringLiteral("myfile.txt"), QStringLiteral("some text\ndummy\nsome other test\n"))
                       << File(QStringLiteral("otherfile.txt"), QStringLiteral("some dummy text\n\n")));

    // see bug: https://bugs.kde.org/show_bug.cgi?id=301362
    QTest::newRow("LF character replace")
        << (FileList() << File(QStringLiteral("somefile.txt"), QStringLiteral("hello world\\n")))
        << "\\\\n" << "%s"
        << "\\n\\n" << "%s"
        << (FileList() << File(QStringLiteral("somefile.txt"), QStringLiteral("hello world\\n\\n")));

    QTest::newRow("Template replace")
        << (FileList() << File(QStringLiteral("somefile.h"),   QStringLiteral("struct Foo {\n  void setFoo(int foo);\n};"))
                       << File(QStringLiteral("somefile.cpp"), QStringLiteral("instance->setFoo(0);\n setFoo(0); /*not replaced*/")))
        << "setFoo" << "\\->\\s*\\b%s\\b\\s*\\("
        << "setBar" << "->%s("
        << (FileList() << File(QStringLiteral("somefile.h"),   QStringLiteral("struct Foo {\n  void setFoo(int foo);\n};"))
                       << File(QStringLiteral("somefile.cpp"), QStringLiteral("instance->setBar(0);\n setFoo(0); /*not replaced*/")));

    QTest::newRow("Template with captures")
        << (FileList() << File(QStringLiteral("somefile.cpp"), QStringLiteral("inst::func(1, 2)\n otherInst :: func (\"foo\")\n func()")))
        << "func" << "([a-z0-9_$]+)\\s*::\\s*\\b%s\\b\\s*\\("
        << "REPL" << "\\1::%s("
        << (FileList() << File(QStringLiteral("somefile.cpp"), QStringLiteral("inst::REPL(1, 2)\n otherInst::REPL(\"foo\")\n func()")));

    QTest::newRow("Regexp pattern")
        << (FileList() << File(QStringLiteral("somefile.txt"), QStringLiteral("foobar\n foooobar\n fake")))
        << "f\\w*o" << "%s"
        << "FOO" << "%s"
        << (FileList() << File(QStringLiteral("somefile.txt"), QStringLiteral("FOObar\n FOObar\n fake")));
}


void FindReplaceTest::testReplace()
{
    QFETCH(FileList, subject);
    QFETCH(QString,  searchPattern);
    QFETCH(QString,  searchTemplate);
    QFETCH(QString,  replace);
    QFETCH(QString,  replaceTemplate);
    QFETCH(FileList, result);

    QTemporaryDir tempDir;
    QDir     dir(tempDir.path());  // we need some convenience functions that are not in QTemporaryDir

    for (const File& fileData : qAsConst(subject)) {
        QFile file(dir.filePath(fileData.first));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QVERIFY(file.write(fileData.second.toUtf8()) != -1);
        file.close();
    }

    auto *job = new GrepJob(this);
    auto *model = new GrepOutputModel(job);
    GrepJobSettings settings;

    job->setOutputModel(model);
    job->setDirectoryChoice(QList<QUrl>() << QUrl::fromLocalFile(dir.path()));

    settings.projectFilesOnly = false;
    settings.caseSensitive = true;
    settings.regexp = true;
    settings.depth = -1; // fully recursive
    settings.pattern = searchPattern;
    settings.searchTemplate = searchTemplate;
    settings.replacementTemplate = replaceTemplate;
    settings.files = QStringLiteral("*");
    settings.exclude = QString();

    job->setSettings(settings);

    QVERIFY(job->exec());

    QVERIFY(model->hasResults());
    model->setReplacement(replace);
    model->makeItemsCheckable(true);
    model->doReplacements();

    for (const File& fileData : qAsConst(result)) {
        QFile file(dir.filePath(fileData.first));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(QString(file.readAll()), fileData.second);
        file.close();
    }
    tempDir.remove();
}


QTEST_MAIN(FindReplaceTest)

#include "moc_test_findreplace.cpp"
