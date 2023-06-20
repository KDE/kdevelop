/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_clangtidyjob.h"

// SUT
#include "job.h"
// KDevPlatform
#include <language/editor/documentrange.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
// Qt
#include <QTest>
#include <QSignalSpy>


using namespace KDevelop;
using namespace ClangTidy;

class JobTester : public Job
{
    Q_OBJECT

public:
    JobTester(Job::Parameters params)
        : Job(params)
    {
    }

    using Job::processStdoutLines;
    using Job::processStderrLines;
    using Job::childProcessExited;

    QString standardOutput() const { return m_standardOutput.join('\n'); }
};

void TestClangTidyJob::initTestCase()
{
    AutoTestShell::init({ "kdevclangtidy" });
    TestCore::initialize(Core::NoUi);
}

void TestClangTidyJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangTidyJob::testJob()
{
    QFile output_example_file(QFINDTESTDATA("data/output_example"));
    QVERIFY(output_example_file.open(QIODevice::ReadOnly));
    QTextStream ios(&output_example_file);
    QStringList stdoutOutput;
    QString line;
    while (ios.readLineInto(&line)) {
        stdoutOutput << line;
    }
    QVERIFY(!stdoutOutput.isEmpty());

    Job::Parameters jobParams;
    JobTester jobTester(jobParams);

    qRegisterMetaType<QVector<KDevelop::IProblem::Ptr>>();
    QSignalSpy problemsSpy(&jobTester, &JobTester::problemsDetected);

    jobTester.processStdoutLines(stdoutOutput);
    QCOMPARE(jobTester.standardOutput(), stdoutOutput.join('\n'));

    jobTester.childProcessExited(0, QProcess::NormalExit);

    QCOMPARE(problemsSpy.count(), 1);
    const auto problems = qvariant_cast<QVector<KDevelop::IProblem::Ptr>>(problemsSpy.at(0).at(0));

    QVERIFY(problems[0]->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));
    QVERIFY(
        problems[0]->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
    QVERIFY(problems[1]->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));

    QVERIFY(
        problems[1]->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
    QVERIFY(problems[2]->finalLocation().document.str().contains(QStringLiteral("/kdev-clang-tidy/src/plugin.cpp")));

    QVERIFY(
        problems[2]->explanation().startsWith(QStringLiteral("[cppcoreguidelines-pro-bounds-array-to-pointer-decay]")));
}

QTEST_GUILESS_MAIN(TestClangTidyJob)

#include "test_clangtidyjob.moc"
#include "moc_test_clangtidyjob.cpp"
