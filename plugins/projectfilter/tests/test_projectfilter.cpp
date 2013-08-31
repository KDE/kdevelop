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

#include <qtest_kde.h>

#include <QDebug>
#include <KConfigGroup>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>

#include "../projectfilter.h"

QTEST_KDEMAIN(TestProjectFilter, NoGUI);

using namespace KDevelop;

typedef QSharedPointer<ProjectFilter> TestFilter;

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
            << KUrl(project.folder(), test.path)
            << test.isFolder
            << test.shouldMatch;

        if (test.isFolder) {
            // also test folder with trailing slash - should not make a difference
            QTest::newRow(qstrdup(qPrintable(tag + ':' + test.path + '/')))
                << filter
                << KUrl(project.folder(), test.path + '/')
                << test.isFolder
                << test.shouldMatch;
        }
    }
}

///FIXME: remove once we can use c++11
#define ADD_TESTS(tag, project, filter, tests) addTests(tag, project, filter, tests, sizeof(tests) / sizeof(tests[0]))

}

void TestProjectFilter::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    qRegisterMetaType<TestFilter>();
}

void TestProjectFilter::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProjectFilter::match()
{
    QFETCH(TestFilter, filter);
    QFETCH(KUrl, path);
    QFETCH(bool, isFolder);
    QFETCH(bool, expectedIsValid);

    QCOMPARE(filter->isValid(path, isFolder), expectedIsValid);
}

void TestProjectFilter::match_data()
{
    QTest::addColumn<TestFilter>("filter");
    QTest::addColumn<KUrl>("path");
    QTest::addColumn<bool>("isFolder");
    QTest::addColumn<bool>("expectedIsValid");

    {
        // test default filters
        const TestProject project;
        TestFilter filter(new ProjectFilter(&project, deserialize(defaultFilters())));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"folder", Folder, Valid},
            {"folder/folder", Folder, Valid},
            {"file", File, Valid},
            {"folder/file", File, Valid},
            {".file", File, Invalid},
            {".folder", Folder, Invalid},
            {"folder/.folder", Folder, Invalid},
            {"folder/.file", File, Invalid},

            {".git", Folder, Invalid},
            {"_darcs", Folder, Invalid},
            {"_svn", Folder, Invalid},
            {".svn", Folder, Invalid},
            {"CVS", Folder, Invalid},
            {"SCCS", Folder, Invalid},
            {".hg", Folder, Invalid},
            {".bzr", Folder, Invalid},

            {"foo.o", File, Invalid},
            {"foo.so", File, Invalid},
            {"foo.so.1", File, Invalid},
            {"foo.a", File, Invalid},
            {"moc_foo.cpp", File, Invalid},
            {"foo.moc", File, Invalid},
            {"ui_foo.h", File, Invalid},
            {"qrc_foo.cpp", File, Invalid},
            {"foo.cpp~", File, Invalid},
            {".foo.cpp.kate-swp", File, Invalid},
            {".foo.cpp.swp", File, Invalid}
        };
        ADD_TESTS("default", project, filter, tests);
    }
    {
        // test exclude files, basename
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("*.cpp", Filter::Files));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"folder", Folder, Valid},
            {"file", File, Valid},
            {"file.cpp", File, Invalid},
            {"folder.cpp", Folder, Valid},
            {"folder/file.cpp", File, Invalid},
            {"folder/folder.cpp", Folder, Valid}
        };
        ADD_TESTS("exclude:*.cpp", project, filter, tests);
    }
    {
        // test excludes on folders
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("foo", Filter::Folders));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"folder", Folder, Valid},
            {"file", File, Valid},
            {"foo", Folder, Invalid},
            {"folder/file", File, Valid},
            {"folder/foo", Folder, Invalid},
            {"folder/foo", File, Valid}
        };
        ADD_TESTS("exclude:foo", project, filter, tests);
    }
    {
        // test includes
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("*", Filter::Files))
            << Filter(SerializedFilter("*.cpp", Filter::Files, Filter::Inclusive));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"folder", Folder, Valid},
            {"file", File, Invalid},
            {"file.cpp", File, Valid},
            {".file.cpp", File, Valid},
            {"folder/file.cpp", File, Valid},
            {"folder/.file.cpp", File, Valid}
        };
        ADD_TESTS("include:*.cpp", project, filter, tests);
        project.projectConfiguration();
    }
    {
        // test mixed stuff
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("*", Filter::Files, Filter::Exclusive))
            << Filter(SerializedFilter("*.inc", Filter::Files, Filter::Inclusive))
            << Filter(SerializedFilter("*ex.inc", Filter::Files, Filter::Exclusive))
            << Filter(SerializedFilter("bar", Filter::Folders, Filter::Exclusive));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"folder", Folder, Valid},
            {"file", File, Invalid},
            {"file.inc", File, Valid},
            {"file.ex.inc", File, Invalid},
            {"folder/file", File, Invalid},
            {"folder/file.inc", File, Valid},
            {"folder/file.ex.inc", File, Invalid},
            {"bar", Folder, Invalid},
        };
        ADD_TESTS("mixed", project, filter, tests);
    }
    {
        // relative path
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("/foo/*bar", Filter::Targets(Filter::Files | Filter::Folders)));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"foo", Folder, Valid},
            {"bar", File, Valid},
            {"foo/bar", Folder, Invalid},
            {"foo/bar", File, Invalid},
            {"foo/asdf/bar", Folder, Invalid},
            {"foo/asdf/bar", File, Invalid},
            {"foo/asdf_bar", Folder, Invalid},
            {"foo/asdf_bar", File, Invalid},
            {"asdf/bar", File, Valid},
            {"asdf/foo/bar", File, Valid},
        };
        ADD_TESTS("relative", project, filter, tests);
    }
    {
        // trailing slash
        const TestProject project;
        const Filters filters = Filters()
            << Filter(SerializedFilter("bar/", Filter::Targets(Filter::Files | Filter::Folders)));
        TestFilter filter(new ProjectFilter(&project, filters));

        QTest::newRow("projectRoot") << filter << project.folder() << Folder << Valid;
        QTest::newRow("project.kdev4") << filter << project.projectFileUrl() << File << Invalid;

        MatchTest tests[] = {
            //{path, isFolder, isValid}
            {".kdev4", Folder, Invalid},

            {"foo", Folder, Valid},
            {"bar", File, Valid},
            {"bar", Folder, Invalid},
            {"foo/bar", File, Valid},
            {"foo/bar", Folder, Invalid}
        };
        ADD_TESTS("trailingslash", project, filter, tests);
    }
}

#include "test_projectfilter.moc"
