/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_findreplace.h"

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QRegExp>

#include <QTemporaryFile>
#include <QTemporaryDir>

#include <project/projectmodel.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <util/filesystemhelpers.h>

#include "../grepjob.h"
#include "../grepviewplugin.h"
#include "../grepoutputmodel.h"

#include <iterator>
#include <vector>

using namespace KDevelop;

namespace {
GrepJobSettings verbatimGrepJobSettings()
{
    GrepJobSettings settings;
    settings.caseSensitive = true;
    settings.regexp = false;
    const QString verbatimTemplate = "%s";
    settings.searchTemplate = verbatimTemplate;
    settings.replacementTemplate = verbatimTemplate;
    return settings;
}
}

void FindReplaceTest::initTestCase()
{
    KDevelop::AutoTestShell::init({{}}); // do not load plugins at all
    const auto core = TestCore::initialize(Core::NoUi);

    delete core->projectController();
    m_projectController = new TestProjectController(core);
    core->setProjectController(m_projectController);
}

void FindReplaceTest::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}

void FindReplaceTest::init()
{
    // Ensure there are no open projects even if the last test run crashed.
    m_projectController->closeAllProjects();
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
    QFETCH(const QString, subject);
    QFETCH(const QRegExp, search);
    QFETCH(const MatchList, matches);

    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(subject.toUtf8());
    file.close();

    const auto actualMatches = grepFile(file.fileName(), search);

    QCOMPARE(actualMatches->size(), matches.size());

    for(int i=0; i<matches.length(); i++)
    {
        const auto* const item = dynamic_cast<GrepOutputItem*>(actualMatches->at(i));
        QVERIFY(item);
        const auto range = item->change()->m_range;
        QCOMPARE(range.start().line(), matches[i].line);
        QCOMPARE(range.start().column(), matches[i].start);
        QCOMPARE(range.end().line(), matches[i].line);
        QCOMPARE(range.end().column(), matches[i].end);
    }

    // check that file has not been altered by grepFile
    QVERIFY(file.open());
    QCOMPARE(QString(file.readAll()), subject);
}

void FindReplaceTest::testSingleFileAsDirectoryChoice()
{
    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), qPrintable("couldn't create temporary directory: " + tmpDir.errorString()));

    const QByteArray fileContents = "A";
    QString filePath = "testfile.cpp";

    using FilesystemHelpers::makeAbsoluteCreateAndWrite;
    QString errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), filePath, fileContents);
    QVERIFY2(errorPath.isEmpty(), qPrintable("couldn't create or write to temporary file or directory " + errorPath));

    const QString siblingDirPath = tmpDir.filePath("dir");
    QVERIFY(QDir{}.mkpath(siblingDirPath));

    const QString siblingDirSameStartPath = tmpDir.filePath("test");
    QVERIFY(filePath.startsWith(siblingDirSameStartPath));
    QVERIFY(QDir{}.mkpath(siblingDirSameStartPath));

    GrepJobSettings settings = verbatimGrepJobSettings();
    settings.pattern = fileContents;

    const auto test = [filePath, &settings](const QList<QUrl>& directoryChoice, bool expectMatch) {
        GrepJob job;
        GrepOutputModel model;
        job.setOutputModel(&model);
        job.setDirectoryChoice(directoryChoice);
        job.setSettings(settings);

        QVERIFY(job.exec());

        const auto index = model.nextItemIndex(QModelIndex{});

        if (!expectMatch) {
            QVERIFY(!index.isValid());
            return;
        }

        QVERIFY(index.isValid());
        auto* const item = dynamic_cast<const GrepOutputItem*>(model.itemFromIndex(index));
        QVERIFY(item);
        QVERIFY(item->isText());
        QCOMPARE(item->filename(), filePath);

        const auto nextIndex = model.nextItemIndex(index);
        QVERIFY2(!nextIndex.isValid() || nextIndex == index, "unexpected second matched file");
    };

    const QString nonexistentFilePath = "/tmp/nonexistent/file/path.kdevelop";
    QVERIFY2(!QFileInfo::exists(nonexistentFilePath), "what a strange file path to exist...");
    // Test searching nowhere, in a nonexistent file and in empty directories in addition to
    // the file set up above. The search locations, where nothing can be found, are
    // unrelated to the main test, but do not complicate this test function much either.
    const QList<QUrl> directoryChoices[] = {{},
                                            {QUrl::fromLocalFile(nonexistentFilePath)},
                                            {QUrl::fromLocalFile(siblingDirPath)},
                                            {QUrl::fromLocalFile(siblingDirSameStartPath)},
                                            {QUrl::fromLocalFile(filePath)}};

    varyProjectFilesOnly(settings, tmpDir.path(), [this, &settings, &directoryChoices, test] {
        if (settings.projectFilesOnly) {
            for (const int i : {2, 3, 4}) {
                QVERIFY(m_projectController->findProjectForUrl(directoryChoices[i].constFirst()));
            }
        }

        for (const auto& directoryChoice : directoryChoices) {
            qDebug() << "\tsearch locations:" << directoryChoice;
            const bool expectMatch = &directoryChoice == &*std::rbegin(directoryChoices);
            for (int depth = -1; depth <= 2; ++depth) {
                // Depth makes no difference to matching a file path search location.
                qDebug("\t\tdepth=%d", depth);
                settings.depth = depth;
                for (const auto files : {"*", "*.nonmatching"}) {
                    // A file path search location is not matched against the Files filter.
                    qDebug("\t\t\tFiles filter: \"%s\"", files);
                    settings.files = files;
                    for (const auto exclude : {"", "/"}) {
                        // A file path search location is not matched against the Exclude filter.
                        qDebug("\t\t\t\tExclude filter: \"%s\"", exclude);
                        settings.exclude = exclude;

                        test(directoryChoice, expectMatch);
                    }
                }
            }
        }
    });
}

void FindReplaceTest::testIncludeExcludeFilters_data()
{
    struct Row{
        const char* dataTag;
        int depth;
        const char* files;
        const char* exclude;
        std::vector<const char*> unmatchedPaths;
        std::vector<const char*> matchedPaths;
    };

    const std::vector<Row> dataRows{
        Row{"depth=0",
            0,
            "*",
            "",
            {"a/b", "x/y.cpp", "my/long/path/n.txt"},
            {"no", "b", "t.cpp", "n.txt", "A very long file name"}},
        Row{"depth=1",
            1,
            "*",
            "",
            {"a/c/d", "p/y/z.cpp", "my/long/path/n.txt"},
            {"y", "t.cpp", "a/b", "a/nt", "x/y.cpp"}},
        Row{"Files filter",
            -1,
            "*.cpp,*.cc,*.h,INSTALL",
            "",
            {"A", "cpp", ".cp", "a.c", "INSTAL", "oINSTALL", "d./h", "d.h/c", "u/INSTALL/v", "a.cpp/b.cp", "INSTALL/h",
             "a.h.c"},
            {"x/INSTALL", "x/.cpp", ".cc", "x.h", "t/s/r/.h/.cc", "INSTALL.cpp", "x/y/z/a/b/c.h", "y/b.cc", "a.hh.cc",
             "t.h.cc"}},
        Row{"Exclude filter",
            4,
            "*",
            "/build/,/.git/,~",
            {"build/C", ".git/config", "~", "a/b/c/build/t/n", "build/me", "a/~/x", "a~b/c", "temp~", "a/p/test~",
             "a/build/b/c", "d/c/.git/t"},
            {"d/build", "a/b/.git", "x", "a.h", "a.git", "a/.gitignore/b", ".gitignore", "buildme/now",
             "to build/.git"}},
        Row{"Files and Exclude filters",
            -1,
            "*.a,*-b,*se",
            "/release/,/.*/,bak",
            {"release/x.a", ".git/q-b", "a-b.c", "bak.a", "abakse", "a/bak-b", "a/x.bak", "u/v/wbakxyz", "a/.g/se",
             "-/b"},
            {"a.a", "b-b", "a/release", ".a", "git/q-b", "se", "a/.se", "a/b.c/d-b", "Bse", "u/v/.a-b", "a/b/.ignorse",
             "ba.k/.a"}},
        Row{"depth=1, Files and Exclude filters",
            1,
            "*.a,*-b,*se",
            "/release/,/.*/,bak",
            {"release/x.a", ".git/q-b", "a-b.c", "bak.a", "abakse", "a/bak-b", "a/x.bak", "u/v/wbakxyz", "a/.g/se",
             "-/b", "a/b.c/d-b", "u/v/.a-b", "a/b/.ignorse"},
            {"a.a", "b-b", "a/release", ".a", "git/q-b", "se", "a/.se", "Bse", "ba.k/.a"}},
        Row{"Matching case-insensitive",
            9,
            "A*b,*.Cd,*.AUX",
            "GiT,garble,B.CD",
            {"acbGArblE.cD", "git/a.b", ".git/x.cd", "b.Cd", "u/v/q", "u/v/bcd", "u/v/b.Cd", "garble.AUX"},
            {"Ab", "a.b", "u/v/ADB", "gi.cd", ".CD", "az.cd", "u/v/w/agb", "p.AuX", "u/q.aux"}}};

    QTest::addColumn<int>("depth");
    QTest::addColumn<QString>("files");
    QTest::addColumn<QString>("exclude");
    QTest::addColumn<QStringList>("unmatchedPaths");
    QTest::addColumn<QStringList>("matchedPaths");

    for (const Row& row : dataRows) {
        const QStringList unmatchedPaths(row.unmatchedPaths.cbegin(), row.unmatchedPaths.cend());
        const QStringList matchedPaths(row.matchedPaths.cbegin(), row.matchedPaths.cend());
        QTest::newRow(row.dataTag) << row.depth << QString{row.files} << QString{row.exclude} << unmatchedPaths
                                   << matchedPaths;
    }
}

void FindReplaceTest::testIncludeExcludeFilters()
{
    QFETCH(const int, depth);
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

    GrepJobSettings settings = verbatimGrepJobSettings();
    settings.depth = depth;
    settings.pattern = commonFileContents;
    settings.files = files;
    settings.exclude = exclude;

    // Modify a copy of the matchedPaths list - pathsToMatch - in order
    // to keep matchedPaths intact for each invocation of this lambda.
    const auto test = [&tmpDir, &settings](QStringList pathsToMatch) {
        GrepJob job;
        GrepOutputModel model;
        job.setOutputModel(&model);
        job.setDirectoryChoice({QUrl::fromLocalFile(tmpDir.path())});
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
            QVERIFY2(pathsToMatch.contains(filename), qPrintable("unexpected matched file " + filename));
            QVERIFY2(pathsToMatch.removeOne(filename),
                     qPrintable("there must be exactly one text match for " + filename));
        }
        QVERIFY2(pathsToMatch.empty(),
                 qPrintable("these files should have been matched, but weren't: " + pathsToMatch.join("; ")));
    };

    varyProjectFilesOnly(settings, tmpDir.path(), [&matchedPaths, test] {
        test(matchedPaths);
    });
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

    for (const File& fileData : std::as_const(subject)) {
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

    for (const File& fileData : std::as_const(result)) {
        QFile file(dir.filePath(fileData.first));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(QString(file.readAll()), fileData.second);
        file.close();
    }
    tempDir.remove();
}

void FindReplaceTest::addTestProjectFromFileSystem(const QString& path)
{
    auto* const project = new TestProject(Path{path});
    TestProjectUtils::addChildrenFromFileSystem(project->projectItem());
    m_projectController->addProject(project);
}

template<typename Test>
void FindReplaceTest::varyProjectFilesOnly(GrepJobSettings& settings, const QString& projectPath, Test testToRun)
{
    for (const bool projectFilesOnly : {false, true}) {
        qDebug() << "limit to project files:" << projectFilesOnly;
        settings.projectFilesOnly = projectFilesOnly;

        if (projectFilesOnly) {
            addTestProjectFromFileSystem(projectPath);
        }

        testToRun();

        m_projectController->closeAllProjects();
    }
}

QTEST_MAIN(FindReplaceTest)

#include "moc_test_findreplace.cpp"
