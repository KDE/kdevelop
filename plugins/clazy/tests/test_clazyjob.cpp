/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_clazyjob.h"

#include "job.h"

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <language/editor/documentrange.h>

#include <QTest>

using namespace KDevelop;
using namespace Clazy;

class JobTester : public Job
{
    Q_OBJECT

public:
    JobTester()
    {
        connect(this, &JobTester::problemsDetected,
                this, [this](const QVector<KDevelop::IProblem::Ptr>& problems) {
                    m_problems += problems;
                });
    }

    ~JobTester() override = default;

    using Job::processStdoutLines;
    using Job::processStderrLines;

    const QVector<KDevelop::IProblem::Ptr>& problems() const
    {
        return m_problems;
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

private:
    QVector<KDevelop::IProblem::Ptr> m_problems;
};

void TestClazyJob::initTestCase()
{
    AutoTestShell::init({"kdevclazy"});
    TestCore::initialize(Core::NoUi);
}

void TestClazyJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClazyJob::testJob()
{
    JobTester jobTester;

    // test errors parsing =========================================================================

    static const QStringList stderrOutput1 = {
        QStringLiteral("source2.cpp:13:10: warning: unused variable 'check' [-Wunused-variable]"),
        QStringLiteral("    auto check = db.checks()[\"returning-void-expression\"];")
    };

    static const QStringList stderrOutput2 = {
        QStringLiteral("source3.cpp:248:21: warning: Don't call QList::first() on temporary [-Wclazy-detaching-temporary]"),
        QStringLiteral("        auto item = pContext->items().first();"),
        QStringLiteral("                    ^"),
        QStringLiteral("1 warning generated.")
    };

    static const QStringList stderrOutput3 = {
        QStringLiteral("source4.cpp:47:9: warning: unused QString [-Wclazy-unused-non-trivial-variable]"),
        QStringLiteral("        auto test = QString(\"%1 : %2\").arg(\"a\").arg(\"b\");"),
        QStringLiteral("        ^"),
        QStringLiteral("source4.cpp:47:47: warning: Use multi-arg instead [-Wclazy-qstring-arg]"),
        QStringLiteral("        auto test = QString(\"%1 : %2\").arg(\"a\").arg(\"b\");"),
        QStringLiteral("                                              ^"),
        QStringLiteral("2 warnings generated.")
    };

    jobTester.processStderrLines(stderrOutput1);
    QCOMPARE(jobTester.problems().size(), 0);

    jobTester.processStderrLines(stderrOutput2);
    QCOMPARE(jobTester.problems().size(), 1);

    jobTester.processStderrLines(stderrOutput3);
    QCOMPARE(jobTester.problems().size(), 3);

    // test common values
    const auto problems = jobTester.problems();
    for (auto problem : problems) {
        QCOMPARE(problem->severity(), KDevelop::IProblem::Warning);
        QCOMPARE(problem->source(), KDevelop::IProblem::Plugin);
    }

    // test problem description
    QCOMPARE(problems[0]->description(), QStringLiteral("Don't call QList::first() on temporary"));
    QCOMPARE(problems[1]->description(), QStringLiteral("unused QString"));
    QCOMPARE(problems[2]->description(), QStringLiteral("Use multi-arg instead"));

    // test problem location (file)
    QCOMPARE(problems[0]->finalLocation().document.str(), QStringLiteral("source3.cpp"));
    QCOMPARE(problems[1]->finalLocation().document.str(), QStringLiteral("source4.cpp"));
    QCOMPARE(problems[2]->finalLocation().document.str(), QStringLiteral("source4.cpp"));

    // test problem location (line)
    QCOMPARE(problems[0]->finalLocation().start().line(), 247);
    QCOMPARE(problems[1]->finalLocation().start().line(), 46);
    QCOMPARE(problems[2]->finalLocation().start().line(), 46);

    // test problem location (column)
    QCOMPARE(problems[0]->finalLocation().start().column(), 20);
    QCOMPARE(problems[1]->finalLocation().start().column(), 8);
    QCOMPARE(problems[2]->finalLocation().start().column(), 46);
}

QTEST_GUILESS_MAIN(TestClazyJob)

#include "test_clazyjob.moc"
