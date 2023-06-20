/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_compileanalyzejob.h"

#include "compileanalyzejob.h"

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QTest>

using namespace KDevelop;

class JobTester : public CompileAnalyzeJob
{
    Q_OBJECT

public:
    JobTester()
    {
        setToolDisplayName("TestAnalyzer");
        connect(this, &JobTester::infoMessage, this, &JobTester::collectStarted);
    }

public:
    using CompileAnalyzeJob::parseProgress;

    const QVector<QString>& started() const
    {
        return m_started;
    }

// implementation detail not accessible
#if 0
    void setTotalCount(int totalCount)
    {
        m_totalCount = totalCount;
    }

    int finishedCount() const
    {
        return m_finishedCount;
    }
#endif

private Q_SLOTS:
    void collectStarted(KJob*, const QString& name)
    {
        m_started += name;
    }

private:
    QVector<QString> m_started;
};

void TestCompileAnalyzeJob::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCompileAnalyzeJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCompileAnalyzeJob::testJob()
{
    JobTester jobTester;

    // test progress parsing =======================================================================

    static const QStringList stdoutOutput1 = {
        QStringLiteral("TestAnalyzer check started  for source2.cpp"),
        QStringLiteral("TestAnalyzer check started  for source1.cpp")
    };

    static const QStringList stdoutOutput2 = {
        QStringLiteral("TestAnalyzer check finished for source2.cpp"),
        QStringLiteral("TestAnalyzer check started  for source3.cpp"),
        QStringLiteral("TestAnalyzer check started  for source4.cpp")
    };

    static const QStringList stdoutOutput3 = {
        QStringLiteral("TestAnalyzer check finished for source1.cpp"),
        QStringLiteral("TestAnalyzer check finished for source4.cpp")
    };

    static const QStringList stdoutOutput4 = {
        QStringLiteral("TestAnalyzer check finished for source3.cpp"),
    };

//     jobTester.setTotalCount(4);

    jobTester.parseProgress(stdoutOutput1);
    QCOMPARE(jobTester.started().size(), 2);
    QCOMPARE(jobTester.started().at(0), QStringLiteral("source2.cpp"));
    QCOMPARE(jobTester.started().at(1), QStringLiteral("source1.cpp"));
//     QCOMPARE(jobTester.finishedCount(), 0);
    QCOMPARE(jobTester.percent(), (unsigned long)0);

    jobTester.parseProgress(stdoutOutput2);
    QCOMPARE(jobTester.started().size(), 4);
    QCOMPARE(jobTester.started().at(2), QStringLiteral("source3.cpp"));
    QCOMPARE(jobTester.started().at(3), QStringLiteral("source4.cpp"));
//     QCOMPARE(jobTester.finishedCount(), 1);
//     QCOMPARE(jobTester.percent(), (unsigned long)25);

    jobTester.parseProgress(stdoutOutput3);
    QCOMPARE(jobTester.started().size(), 4);
//     QCOMPARE(jobTester.finishedCount(), 3);
//     QCOMPARE(jobTester.percent(), (unsigned long)75);

    jobTester.parseProgress(stdoutOutput4);
    QCOMPARE(jobTester.started().size(), 4);
//     QCOMPARE(jobTester.finishedCount(), 4);
//     QCOMPARE(jobTester.percent(), (unsigned long)100);

    QCOMPARE(jobTester.started().at(0), QStringLiteral("source2.cpp"));
    QCOMPARE(jobTester.started().at(1), QStringLiteral("source1.cpp"));
    QCOMPARE(jobTester.started().at(2), QStringLiteral("source3.cpp"));
    QCOMPARE(jobTester.started().at(3), QStringLiteral("source4.cpp"));
}

QTEST_GUILESS_MAIN(TestCompileAnalyzeJob)

#include "test_compileanalyzejob.moc"
#include "moc_test_compileanalyzejob.cpp"
