/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_projectfilter.h"

#include <QTest>
#include <KConfigGroup>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>

#include "../projectfilter.h"

QTEST_GUILESS_MAIN(TestProjectFilter)

using namespace KDevelop;

using TestFilter = QSharedPointer<ProjectFilter>;

Q_DECLARE_METATYPE(TestFilter)

namespace {

const bool Invalid = false;
const bool Valid = true;
const bool Folder = true;
const bool File = false;

struct MatchTest
{
    QString path;
    bool isFolder;
    bool shouldMatch;
};

void addTests(const QString& tag, const TestProject& project, const TestFilter& filter, MatchTest* tests, uint numTests)
{
    for (uint i = 0; i < numTests; ++i) {
        const MatchTest& test = tests[i];
        QTest::newRow(qstrdup(qPrintable(tag + ':' + test.path)))
            << filter
            << Path(project.path(), test.path)
            << test.isFolder
            << test.shouldMatch;

        if (test.isFolder) {
            // also test folder with trailing slash - should not make a difference
            QTest::newRow(qstrdup(qPrintable(tag + ':' + test.path + '/')))
                << filter
                << Path(project.path(), test.path)
                << test.isFolder
                << test.shouldMatch;
        }
    }
}

///FIXME: remove once we can use c++11
#define ADD_TESTS(tag, project, filter, tests) addTests(QStringLiteral(tag), project, filter, tests, sizeof(tests) / sizeof(tests[0]))

struct BenchData
{
    BenchData(const Path &path = Path(), bool isFolder = false)
    : path(path)
    , isFolder(isFolder)
    {}

    Path path;
    bool isFolder;
};

}

Q_DECLARE_METATYPE(QVector<BenchData>)

void TestProjectFilter::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    qRegisterMetaType<TestFilter>();
    qRegisterMetaType<Path>();
    qRegisterMetaType<QVector<BenchData> >();
}

void TestProjectFilter::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProjectFilter::match()
{
    QFETCH(TestFilter, filter);
    QFETCH(KDevelop::Path, path);
    QFETCH(bool, isFolder);
    QFETCH(bool, expectedIsValid);

    QCOMPARE(filter->isValid(path, isFolder), expectedIsValid);
}

void TestProjectFilter::match_data()
{
    QTest::addColumn<TestFilter>("filter");
    QTest::addColumn<Path>("path");
    QTest::addColumn<bool>("isFolder");
    QTest::addColumn<bool>("expectedIsValid");

    {
        // test default filters
        const TestProject project;
        TestFilter filter(new ProjectFilter(&project, deserialize(defaultFilters())));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("folder"), Folder, Valid},
            {QStringLiteral("folder/folder"), Folder, Valid},
            {QStringLiteral("file"), File, Valid},
            {QStringLiteral("folder/file"), File, Valid},
            {QStringLiteral(".file"), File, Invalid},
            {QStringLiteral(".folder"), Folder, Invalid},
            {QStringLiteral("folder/.folder"), Folder, Invalid},
            {QStringLiteral("folder/.file"), File, Invalid},

            {QStringLiteral(".git"), Folder, Invalid},
            {QStringLiteral(".gitignore"), File, Valid},
            {QStringLiteral(".gitmodules"), File, Valid},
            {QStringLiteral("_darcs"), Folder, Invalid},
            {QStringLiteral("_svn"), Folder, Invalid},
            {QStringLiteral(".svn"), Folder, Invalid},
            {QStringLiteral("CVS"), Folder, Invalid},
            {QStringLiteral("SCCS"), Folder, Invalid},
            {QStringLiteral(".hg"), Folder, Invalid},
            {QStringLiteral(".bzr"), Folder, Invalid},

            {QStringLiteral("foo.o"), File, Invalid},
            {QStringLiteral("foo.so"), File, Invalid},
            {QStringLiteral("foo.so.1"), File, Invalid},
            {QStringLiteral("foo.a"), File, Invalid},
            {QStringLiteral("moc_foo.cpp"), File, Invalid},
            {QStringLiteral("ui_foo.h"), File, Invalid},
            {QStringLiteral("qrc_foo.cpp"), File, Invalid},
            {QStringLiteral("foo.cpp~"), File, Invalid},
            {QStringLiteral(".foo.cpp.kate-swp"), File, Invalid},
            {QStringLiteral(".foo.cpp.swp"), File, Invalid}
        };
        ADD_TESTS("default", project, filter, tests);
    }
    {
        // test exclude files, basename
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("*.cpp"), Filter::Files));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("folder"), Folder, Valid},
            {QStringLiteral("file"), File, Valid},
            {QStringLiteral("file.cpp"), File, Invalid},
            {QStringLiteral("folder.cpp"), Folder, Valid},
            {QStringLiteral("folder/file.cpp"), File, Invalid},
            {QStringLiteral("folder/folder.cpp"), Folder, Valid}
        };
        ADD_TESTS("exclude:*.cpp", project, filter, tests);
    }
    {
        // test excludes on folders
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("foo"), Filter::Folders));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("folder"), Folder, Valid},
            {QStringLiteral("file"), File, Valid},
            {QStringLiteral("foo"), Folder, Invalid},
            {QStringLiteral("folder/file"), File, Valid},
            {QStringLiteral("folder/foo"), Folder, Invalid},
            {QStringLiteral("folder/foo"), File, Valid}
        };
        ADD_TESTS("exclude:foo", project, filter, tests);
    }
    {
        // test includes
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("*"), Filter::Files))
            << Filter(SerializedFilter(QStringLiteral("*.cpp"), Filter::Files, Filter::Inclusive));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("folder"), Folder, Valid},
            {QStringLiteral("file"), File, Invalid},
            {QStringLiteral("file.cpp"), File, Valid},
            {QStringLiteral(".file.cpp"), File, Valid},
            {QStringLiteral("folder/file.cpp"), File, Valid},
            {QStringLiteral("folder/.file.cpp"), File, Valid}
        };
        ADD_TESTS("include:*.cpp", project, filter, tests);
        project.projectConfiguration();
    }
    {
        // test mixed stuff
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("*"), Filter::Files, Filter::Exclusive))
            << Filter(SerializedFilter(QStringLiteral("*.inc"), Filter::Files, Filter::Inclusive))
            << Filter(SerializedFilter(QStringLiteral("*ex.inc"), Filter::Files, Filter::Exclusive))
            << Filter(SerializedFilter(QStringLiteral("bar"), Filter::Folders, Filter::Exclusive));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("folder"), Folder, Valid},
            {QStringLiteral("file"), File, Invalid},
            {QStringLiteral("file.inc"), File, Valid},
            {QStringLiteral("file.ex.inc"), File, Invalid},
            {QStringLiteral("folder/file"), File, Invalid},
            {QStringLiteral("folder/file.inc"), File, Valid},
            {QStringLiteral("folder/file.ex.inc"), File, Invalid},
            {QStringLiteral("bar"), Folder, Invalid},
        };
        ADD_TESTS("mixed", project, filter, tests);
    }
    {
        // relative path
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("/foo/*bar"), Filter::Targets(Filter::Files | Filter::Folders)));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("foo"), Folder, Valid},
            {QStringLiteral("bar"), File, Valid},
            {QStringLiteral("foo/bar"), Folder, Invalid},
            {QStringLiteral("foo/bar"), File, Invalid},
            {QStringLiteral("foo/asdf/bar"), Folder, Invalid},
            {QStringLiteral("foo/asdf/bar"), File, Invalid},
            {QStringLiteral("foo/asdf_bar"), Folder, Invalid},
            {QStringLiteral("foo/asdf_bar"), File, Invalid},
            {QStringLiteral("asdf/bar"), File, Valid},
            {QStringLiteral("asdf/foo/bar"), File, Valid},
        };
        ADD_TESTS("relative", project, filter, tests);
    }
    {
        // trailing slash
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("bar/"), Filter::Targets(Filter::Files | Filter::Folders)));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("foo"), Folder, Valid},
            {QStringLiteral("bar"), File, Valid},
            {QStringLiteral("bar"), Folder, Invalid},
            {QStringLiteral("foo/bar"), File, Valid},
            {QStringLiteral("foo/bar"), Folder, Invalid}
        };
        ADD_TESTS("trailingslash", project, filter, tests);
    }
    {
        // escaping
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter(QStringLiteral("foo\\*bar"), Filter::Files));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.path() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFile() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {QStringLiteral(".kdev4"), Folder, Invalid},

            {QStringLiteral("foobar"), Folder, Valid},
            {QStringLiteral("fooasdfbar"), File, Valid},
            {QStringLiteral("foo*bar"), File, Invalid},
            {QStringLiteral("foo/bar"), Folder, Valid}
        };
        ADD_TESTS("escaping", project, filter, tests);
    }
}

static QVector<BenchData> createBenchData(const Path& base, int folderDepth, int foldersPerFolder, int filesPerFolder)
{
    QVector<BenchData> data;
    data << BenchData(base, true);

    for(int i = 0; i < filesPerFolder; ++i) {
        if (i % 2) {
            data << BenchData(Path(base, QStringLiteral("file%1.cpp").arg(i)), false);
        } else {
            data << BenchData(Path(base, QStringLiteral("file%1.h").arg(i)), true);
        }
    }
    for(int i = 0; i < foldersPerFolder && folderDepth > 0; ++i) {
        data += createBenchData(Path(base, QStringLiteral("folder%1").arg(i)), folderDepth - 1, foldersPerFolder, filesPerFolder);
    }
    return data;
}

void TestProjectFilter::bench()
{
    QFETCH(TestFilter, filter);
    QFETCH(const QVector<BenchData>, data);

    QBENCHMARK {
        for (const BenchData& bench : data) {
            filter->isValid(bench.path, bench.isFolder);
        }
    }
}

void TestProjectFilter::bench_data()
{
    QTest::addColumn<TestFilter>("filter");
    QTest::addColumn<QVector<BenchData> >("data");

    const TestProject project;

    const QVector<QVector<BenchData> > dataSets = QVector<QVector<BenchData> >()
        << createBenchData(project.path(), 3, 5, 10)
        << createBenchData(project.path(), 3, 5, 20)
        << createBenchData(project.path(), 4, 5, 10)
        << createBenchData(project.path(), 3, 10, 10);

    {
        TestFilter filter(new ProjectFilter(&project, Filters()));
        for (const QVector<BenchData>& data : dataSets) {
            QTest::newRow(QByteArray("baseline-" + QByteArray::number(data.size()))) << filter << data;
        }
    }

    {
        TestFilter filter(new ProjectFilter(&project, deserialize(defaultFilters())));
        for (const QVector<BenchData>& data : dataSets) {
            QTest::newRow(QByteArray("defaults-" + QByteArray::number(data.size()))) << filter << data;
        }
    }
}

