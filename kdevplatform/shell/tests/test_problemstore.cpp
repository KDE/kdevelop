/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QSignalSpy>
#include <shell/problemstore.h>
#include <shell/problem.h>
#include <shell/problemstorenode.h>
#include <shell/problemconstants.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

using namespace KDevelop;

class TestProblemStore : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testAddProblems();
    void testClearProblems();
    void testSetProblems();
    void testFindNode();
    void testSeverity();
    void testSeverities();
    void testScope();

private:
    void generateProblems();

    QScopedPointer<ProblemStore> m_store;
    QVector<IProblem::Ptr> m_problems;
};


void TestProblemStore::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_store.reset(new ProblemStore());
    m_store->setScope(CurrentDocument);
    QVERIFY(m_store->scope() == CurrentDocument);

    generateProblems();
}

void TestProblemStore::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProblemStore::testAddProblems()
{
    QCOMPARE(m_store->count(), 0);

    int c = 0;
    for (const IProblem::Ptr& problem : std::as_const(m_problems)) {
        m_store->addProblem(problem);
        c++;

        QCOMPARE(m_store->count(), c);
    }
}

void TestProblemStore::testClearProblems()
{
    m_store->clear();
    QCOMPARE(m_store->count(), 0);
}

void TestProblemStore::testSetProblems()
{
    m_store->setProblems(m_problems);

    QCOMPARE(m_store->count(), m_problems.count());
}

void TestProblemStore::testFindNode()
{
    for (int i = 0; i < m_problems.count(); i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));

        QVERIFY(node);
        QVERIFY(node->problem().data());
        QCOMPARE(node->problem().data()->description(), m_problems[i]->description());
    }
}

void TestProblemStore::testSeverity()
{
    IProblem::Severity severity = IProblem::Error;

    QVERIFY(severity != m_store->severity());

    QSignalSpy spy(m_store.data(), &ProblemStore::changed);
    m_store->setSeverity(severity);

    QVERIFY(m_store->severity() == severity);
    QCOMPARE(spy.count(), 1);
}

void TestProblemStore::testSeverities()
{
    IProblem::Severities severities = IProblem::Error | IProblem::Hint;

    QVERIFY(severities != m_store->severities());

    QSignalSpy spy(m_store.data(), &ProblemStore::changed);
    m_store->setSeverities(severities);

    QVERIFY(m_store->severities() == severities);
    QCOMPARE(spy.count(), 1);
}

void TestProblemStore::testScope()
{
    QSignalSpy spy(m_store.data(), &ProblemStore::changed);
    ProblemScope scope = AllProjects;

    m_store->setScope(scope);
    QVERIFY(m_store->scope() == scope);
    QCOMPARE(spy.count(), 1);
}

void TestProblemStore::generateProblems()
{
    for (int i = 0; i < 5; i++) {
        IProblem::Ptr problem(new DetectedProblem());

        problem->setDescription(QStringLiteral("PROBLEM") + QString::number(i + 1));
        m_problems.push_back(problem);
    }
}

QTEST_MAIN(TestProblemStore)

#include "test_problemstore.moc"
