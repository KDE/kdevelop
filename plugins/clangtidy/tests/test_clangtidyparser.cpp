/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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

#include "test_clangtidyparser.h"

// SUT
#include "parsers/clangtidyparser.h"
// KDevPlatform
#include <language/editor/documentrange.h>
#include <shell/problem.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
// Qt
#include <QFile>
#include <QTextStream>
#include <QTest>
#include <QSignalSpy>

using namespace KDevelop;
using namespace ClangTidy;

void TestClangTidyParser::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestClangTidyParser::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangTidyParser::testParser()
{
    // prepare QStringList from file to be parsed.
    QFile output_example_file(QFINDTESTDATA("data/output_example"));
    QVERIFY(output_example_file.open(QIODevice::ReadOnly));
    QTextStream ios(&output_example_file);
    QStringList clangtidy_example_output;
    QString line;
    while (ios.readLineInto(&line)) {
        clangtidy_example_output << line;
    }

    QVERIFY(!clangtidy_example_output.isEmpty());
    ClangTidy::ClangTidyParser parser;

    qRegisterMetaType<QVector<KDevelop::IProblem::Ptr>>();
    QSignalSpy problemsSpy(&parser, &ClangTidyParser::problemsDetected);

    parser.addData(clangtidy_example_output);

    QCOMPARE(problemsSpy.count(), 1);
    const auto problems = qvariant_cast<QVector<KDevelop::IProblem::Ptr>>(problemsSpy.at(0).at(0));
    QVERIFY(!problems.empty());

    IProblem::Ptr p = problems[0];
    QCOMPARE(p->description(), QStringLiteral("do not implicitly decay an array into a pointer; consider using "
                                              "gsl::array_view or an explicit cast instead"));
    QVERIFY(p->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
    QVERIFY(p->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));
    QCOMPARE(p->finalLocation().start().line() + 1, 80);
    QCOMPARE(p->finalLocation().start().column() + 1, 5);
    QCOMPARE(p->severity(), IProblem::Warning);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with 2 <location> elements
    p = problems[1];
    QCOMPARE(p->description(), QStringLiteral("do not implicitly decay an array into a pointer; consider using "
                                              "gsl::array_view or an explicit cast instead"));
    QVERIFY(p->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
    QVERIFY(p->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));
    QCOMPARE(p->finalLocation().start().line() + 1, 145);
    QCOMPARE(p->finalLocation().start().column() + 1, 9);
    QCOMPARE(p->severity(), IProblem::Warning);
    QCOMPARE(p->source(), IProblem::Plugin);

    // test problem with '\\012' tokens in verbose message
    p = problems[2];
    QCOMPARE(p->description(), QStringLiteral("do not implicitly decay an array into a pointer; consider using "
                                              "gsl::array_view or an explicit cast instead"));
    QVERIFY(p->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
    QVERIFY(p->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));
    QCOMPARE(p->finalLocation().start().line() + 1, 151);
    QCOMPARE(p->finalLocation().start().column() + 1, 9);
    QCOMPARE(p->severity(), IProblem::Warning);
    QCOMPARE(p->source(), IProblem::Plugin);
}

QTEST_GUILESS_MAIN(TestClangTidyParser)
