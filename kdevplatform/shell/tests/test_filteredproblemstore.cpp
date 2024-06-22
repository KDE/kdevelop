/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QSignalSpy>
#include <shell/filteredproblemstore.h>
#include <shell/problem.h>
#include <shell/problemstorenode.h>
#include <shell/problemconstants.h>
#include <language/editor/documentrange.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <KLocalizedString>

namespace
{
const int ErrorCount = 1;
const int WarningCount = 2;
const int HintCount = 3;
const int ProblemsCount = ErrorCount + WarningCount + HintCount;

const int ErrorFilterProblemCount = ErrorCount;
const int WarningFilterProblemCount = ErrorCount + WarningCount;
const int HintFilterProblemCount = ErrorCount + WarningCount + HintCount;
}

using namespace KDevelop;

class TestFilteredProblemStore : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testSeverity();
    void testSeverities();
    void testGrouping();

    void testNoGrouping();
    void testPathGrouping();
    void testSeverityGrouping();

private:
    // Severity grouping testing
    bool checkCounts(int error, int warning, int hint);
    bool checkNodeLabels();
    // ---------------------------

    void generateProblems();

    QScopedPointer<FilteredProblemStore> m_store;
    QVector<IProblem::Ptr> m_problems;
    IProblem::Ptr m_diagnosticTestProblem;
};



#define MYCOMPARE(actual, expected) \
    if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) \
    return false

#define MYVERIFY(statement) \
    if (!QTest::qVerify((statement), #statement, "", __FILE__, __LINE__))\
        return false

void TestFilteredProblemStore::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_store.reset(new FilteredProblemStore());

    generateProblems();
}

void TestFilteredProblemStore::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestFilteredProblemStore::testSeverity()
{
    QVERIFY(m_store->severity() == IProblem::Hint);

    QSignalSpy changedSpy(m_store.data(), &FilteredProblemStore::changed);
    QSignalSpy beginRebuildSpy(m_store.data(), &FilteredProblemStore::beginRebuild);
    QSignalSpy endRebuildSpy(m_store.data(), &FilteredProblemStore::endRebuild);

    m_store->setSeverity(IProblem::Error);

    QVERIFY(m_store->severity() == IProblem::Error);

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(beginRebuildSpy.count(), 1);
    QCOMPARE(endRebuildSpy.count(), 1);

    m_store->setSeverity(IProblem::Hint);
}

void TestFilteredProblemStore::testSeverities()
{
    QVERIFY(m_store->severities() == (IProblem::Error | IProblem::Warning | IProblem::Hint));

    QSignalSpy changedSpy(m_store.data(), &FilteredProblemStore::changed);
    QSignalSpy beginRebuildSpy(m_store.data(), &FilteredProblemStore::beginRebuild);
    QSignalSpy endRebuildSpy(m_store.data(), &FilteredProblemStore::endRebuild);

    m_store->setSeverities(IProblem::Error | IProblem::Hint);

    QVERIFY(m_store->severities() == (IProblem::Error | IProblem::Hint));

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(beginRebuildSpy.count(), 1);
    QCOMPARE(endRebuildSpy.count(), 1);

    m_store->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);
}

void TestFilteredProblemStore::testGrouping()
{
    QVERIFY(m_store->grouping() == NoGrouping);

    QSignalSpy changedSpy(m_store.data(), &FilteredProblemStore::changed);
    QSignalSpy beginRebuildSpy(m_store.data(), &FilteredProblemStore::beginRebuild);
    QSignalSpy endRebuildSpy(m_store.data(), &FilteredProblemStore::endRebuild);

    m_store->setGrouping(PathGrouping);
    QVERIFY(m_store->grouping() == PathGrouping);

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(beginRebuildSpy.count(), 1);
    QCOMPARE(endRebuildSpy.count(), 1);

    m_store->setGrouping(NoGrouping);
}

// Compares the node and it's children to a reference problem and it's diagnostics
bool checkDiagnodes(const ProblemStoreNode *node, const IProblem::Ptr &reference)
{
    const auto *problemNode = dynamic_cast<const ProblemNode*>(node);
    MYVERIFY(problemNode);
    MYCOMPARE(problemNode->problem()->description(), reference->description());
    MYCOMPARE(problemNode->problem()->finalLocation().document.str(), reference->finalLocation().document.str());
    MYCOMPARE(problemNode->count(), 1);

    const IProblem::Ptr diag = reference->diagnostics().at(0);
    const auto *diagNode = dynamic_cast<const ProblemNode*>(problemNode->child(0));
    MYVERIFY(diagNode);
    MYCOMPARE(diagNode->problem()->description(), diag->description());
    MYCOMPARE(diagNode->count(), 1);

    const IProblem::Ptr diagdiag = diag->diagnostics().at(0);
    const auto *diagdiagNode = dynamic_cast<const ProblemNode*>(diagNode->child(0));
    MYVERIFY(diagdiagNode);
    MYCOMPARE(diagdiagNode->problem()->description(), diagdiag->description());
    MYCOMPARE(diagdiagNode->count(), 0);

    return true;
}

void TestFilteredProblemStore::testNoGrouping()
{
    // Add problems
    int c = 0;
    for (const IProblem::Ptr& p : std::as_const(m_problems)) {
        m_store->addProblem(p);

        c++;
        QCOMPARE(m_store->count(), c);
    }
    for (int i = 0; i < c; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Check if clear works
    m_store->clear();
    QCOMPARE(m_store->count(), 0);

    // Set problems
    m_store->setProblems(m_problems);
    QCOMPARE(m_problems.count(), m_store->count());
    for (int i = 0; i < c; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);
        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Check old style severity filtering
    // old-style setSeverity
    // Error filter
    m_store->setSeverity(IProblem::Error);
    QCOMPARE(m_store->count(), ErrorFilterProblemCount);
    for (int i = 0; i < ErrorFilterProblemCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(0));
        QVERIFY(node);
        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Warning filter
    m_store->setSeverity(IProblem::Warning);
    QCOMPARE(m_store->count(), WarningFilterProblemCount);
    for (int i = 0; i < WarningFilterProblemCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Hint filter
    m_store->setSeverity(IProblem::Hint);
    QCOMPARE(m_store->count(), HintFilterProblemCount);
    for (int i = 0; i < HintFilterProblemCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Check new severity filtering
    // Error filter
    m_store->setSeverities(IProblem::Error);
    QCOMPARE(m_store->count(), ErrorCount);
    for (int i = 0; i < ErrorCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);
        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }

    // Warning filter
    m_store->setSeverities(IProblem::Warning);
    QCOMPARE(m_store->count(), WarningCount);
    for (int i = 0; i < WarningCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i+ErrorCount]->description());
    }

    // Hint filter
    m_store->setSeverities(IProblem::Hint);
    QCOMPARE(m_store->count(), HintCount);
    for (int i = 0; i < HintCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i+ErrorCount+WarningCount]->description());
    }

    //Error + Hint filter
    m_store->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_store->count(), HintCount + ErrorCount);
    for (int i = 0; i < ErrorCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i]->description());
    }
    for (int i = ErrorCount; i < ErrorCount+HintCount; i++) {
        const auto *node = dynamic_cast<const ProblemNode*>(m_store->findNode(i));
        QVERIFY(node);

        QCOMPARE(node->problem()->description(), m_problems[i+WarningCount]->description());
    }

    m_store->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);
    m_store->clear();

    // Check if diagnostics are added properly
    m_store->addProblem(m_diagnosticTestProblem);
    QCOMPARE(m_store->count(), 1);
    QVERIFY(checkDiagnodes(m_store->findNode(0), m_diagnosticTestProblem));
}


bool checkNodeLabel(const ProblemStoreNode *node, const QString &label)
{
    const auto *parent = dynamic_cast<const LabelNode*>(node);

    MYVERIFY(parent);
    MYCOMPARE(parent->label(), label);

    return true;
}

bool checkNodeDescription(const ProblemStoreNode *node, const QString &descr)
{
    const auto *n = dynamic_cast<const ProblemNode*>(node);

    MYVERIFY(n);
    MYCOMPARE(n->problem()->description(), descr);

    return true;
}

void TestFilteredProblemStore::testPathGrouping()
{
    m_store->clear();

    // Rebuild the problem list with grouping
    m_store->setGrouping(PathGrouping);

    // Add problems
    for (const IProblem::Ptr& p : std::as_const(m_problems)) {
        m_store->addProblem(p);
    }

    QCOMPARE(m_store->count(), ProblemsCount);

    for (int i = 0; i < 3; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);

        checkNodeDescription(node->child(0), m_problems[i]->description());
    }


    // Now add a new problem
    IProblem::Ptr p(new DetectedProblem());
    p->setDescription(QStringLiteral("PROBLEM4"));
    p->setFinalLocation(m_problems[2]->finalLocation());
    m_store->addProblem(p);

    QCOMPARE(m_store->count(), ProblemsCount);

    // Check the first 2 top-nodes
    for (int i = 0; i < 2; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);

        checkNodeDescription(node->child(0), m_problems[i]->description());
    }

    // check the last one, and check the added problem is at the right place
    {
        const ProblemStoreNode *node = m_store->findNode(2);
        checkNodeLabel(node, m_problems[2]->finalLocation().document.str());
        QCOMPARE(node->count(), 2);

        checkNodeDescription(node->child(1), p->description());
    }

    m_store->clear();
    m_store->setProblems(m_problems);

    // Check filters
    // old-style setSeverity
    // Error filter
    m_store->setSeverity(IProblem::Error);
    QCOMPARE(m_store->count(), ErrorFilterProblemCount);
    {
        const ProblemStoreNode *node = m_store->findNode(0);
        checkNodeLabel(node, m_problems[0]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[0]->description());
    }

    // Warning filter
    m_store->setSeverity(IProblem::Warning);
    QCOMPARE(m_store->count(), WarningFilterProblemCount);
    for (int i = 0; i < WarningFilterProblemCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i]->description());
    }

    // Hint filter
    m_store->setSeverity(IProblem::Hint);
    QCOMPARE(m_store->count(), HintFilterProblemCount);
    for (int i = 0; i < HintFilterProblemCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i]->description());
    }

    // Check new severity filtering
    // Error filter
    m_store->setSeverities(IProblem::Error);
    for (int i = 0; i < ErrorCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i]->description());
    }

    // Warning filter
    m_store->setSeverities(IProblem::Warning);
    for (int i = 0; i < WarningCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i+ErrorCount]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i+ErrorCount]->description());
    }

    // Hint filter
    m_store->setSeverities(IProblem::Hint);
    for (int i = 0; i < HintCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i+ErrorCount+WarningCount]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i+ErrorCount+WarningCount]->description());
    }

    //Error + Hint filter
    m_store->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_store->count(), HintCount + ErrorCount);
    for (int i = 0; i < ErrorCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i]->description());
    }
    for (int i = ErrorCount; i < ErrorCount+HintCount; i++) {
        const ProblemStoreNode *node = m_store->findNode(i);
        checkNodeLabel(node, m_problems[i+WarningCount]->finalLocation().document.str());
        QCOMPARE(node->count(), 1);
        checkNodeDescription(node->child(0), m_problems[i+WarningCount]->description());
    }

    m_store->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    m_store->clear();
    // Check if the diagnostics are added properly
    m_store->addProblem(m_diagnosticTestProblem);
    QCOMPARE(m_store->count(), 1);
    const auto *node = dynamic_cast<const LabelNode*>(m_store->findNode(0));
    QVERIFY(node);
    QCOMPARE(node->label(), m_diagnosticTestProblem->finalLocation().document.str());
    QVERIFY(checkDiagnodes(node->child(0), m_diagnosticTestProblem));
}

void TestFilteredProblemStore::testSeverityGrouping()
{
    m_store->clear();
    m_store->setGrouping(SeverityGrouping);
    QCOMPARE(m_store->count(), 3);
    const ProblemStoreNode *errorNode = m_store->findNode(0);
    const ProblemStoreNode *warningNode = m_store->findNode(1);
    const ProblemStoreNode *hintNode = m_store->findNode(2);

    // Add problems
    for (int i=0;i<ProblemsCount;i++)
    {
        m_store->addProblem(m_problems[i]);
        int severityType = 0; //error
        int addedCountOfCurrentSeverityType = i + 1;
        if (i>=ErrorCount)
        {
            severityType = 1; //warning
            addedCountOfCurrentSeverityType = i - ErrorCount + 1;
        }
        if (i>=ErrorCount+WarningCount)
        {
            severityType = 2; //hint
            addedCountOfCurrentSeverityType = i - (ErrorCount + WarningCount) + 1;
        }
        QCOMPARE(m_store->findNode(severityType)->count(), addedCountOfCurrentSeverityType);
    }

    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, WarningCount, HintCount));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());
    checkNodeDescription(warningNode->child(0), m_problems[1]->description());
    checkNodeDescription(hintNode->child(0), m_problems[3]->description());

    // Clear
    m_store->clear();
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkCounts(0,0,0));

    // Set problems
    m_store->setProblems(m_problems);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, WarningCount, HintCount));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());
    checkNodeDescription(warningNode->child(0), m_problems[1]->description());
    checkNodeDescription(hintNode->child(0), m_problems[3]->description());

    // Check severity filter
    // old-style setSeverity
    // Error filter
    m_store->setSeverity(IProblem::Error);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, 0, 0));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());

    // Warning filter
    m_store->setSeverity(IProblem::Warning);
    QCOMPARE(m_store->count(), 3);
    checkNodeLabels();
    QVERIFY(checkCounts(ErrorCount, WarningCount, 0));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());
    checkNodeDescription(warningNode->child(0), m_problems[1]->description());

    // Hint filter
    m_store->setSeverity(IProblem::Hint);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, WarningCount, HintCount));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());
    checkNodeDescription(warningNode->child(0), m_problems[1]->description());
    checkNodeDescription(hintNode->child(0), m_problems[3]->description());

    // Check severity filter
    // Error filter
    m_store->setSeverities(IProblem::Error);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, 0, 0));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());

    // Warning filter
    m_store->setSeverities(IProblem::Warning);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(0, WarningCount, 0));
    checkNodeDescription(warningNode->child(0), m_problems[1]->description());

    // Hint filter
    m_store->setSeverities(IProblem::Hint);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(0, 0, HintCount));
    checkNodeDescription(hintNode->child(0), m_problems[3]->description());

    // Error + Hint filter
    m_store->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_store->count(), 3);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(ErrorCount, 0, HintCount));
    checkNodeDescription(errorNode->child(0), m_problems[0]->description());
    checkNodeDescription(hintNode->child(0), m_problems[3]->description());

    m_store->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    m_store->clear();
    // Check if diagnostics are added properly
    m_store->addProblem(m_diagnosticTestProblem);
    QVERIFY(checkNodeLabels());
    QVERIFY(checkCounts(1, 0, 0));
    QVERIFY(checkDiagnodes(m_store->findNode(0)->child(0), m_diagnosticTestProblem));
}

bool TestFilteredProblemStore::checkCounts(int error, int warning, int hint)
{
    const ProblemStoreNode *errorNode = m_store->findNode(0);
    const ProblemStoreNode *warningNode = m_store->findNode(1);
    const ProblemStoreNode *hintNode = m_store->findNode(2);

    MYVERIFY(errorNode);
    MYVERIFY(warningNode);
    MYVERIFY(hintNode);

    MYCOMPARE(errorNode->count(), error);
    MYCOMPARE(warningNode->count(), warning);
    MYCOMPARE(hintNode->count(), hint);

    return true;
}

bool TestFilteredProblemStore::checkNodeLabels()
{
    const ProblemStoreNode *errorNode = m_store->findNode(0);
    const ProblemStoreNode *warningNode = m_store->findNode(1);
    const ProblemStoreNode *hintNode = m_store->findNode(2);

    MYCOMPARE(checkNodeLabel(errorNode, i18n("Error")), true);
    MYCOMPARE(checkNodeLabel(warningNode, i18n("Warning")), true);
    MYCOMPARE(checkNodeLabel(hintNode, i18n("Hint")), true);

    return true;
}

// Generate 3 problems, all with different paths, different severity
// Also generates a problem with diagnostics
void TestFilteredProblemStore::generateProblems()
{
    IProblem::Ptr p1(new DetectedProblem());
    IProblem::Ptr p2(new DetectedProblem());
    IProblem::Ptr p3(new DetectedProblem());
    IProblem::Ptr p4(new DetectedProblem());
    IProblem::Ptr p5(new DetectedProblem());
    IProblem::Ptr p6(new DetectedProblem());

    DocumentRange r1;
    r1.document = IndexedString("/just/a/random/path");

    p1->setDescription(QStringLiteral("PROBLEM1"));
    p1->setSeverity(IProblem::Error);
    p1->setFinalLocation(r1);

    DocumentRange r2;
    r2.document = IndexedString("/just/another/path");

    p2->setDescription(QStringLiteral("PROBLEM2"));
    p2->setSeverity(IProblem::Warning);
    p2->setFinalLocation(r2);

    DocumentRange r3;
    r3.document = IndexedString("/just/another/pathy/patha");

    p3->setDescription(QStringLiteral("PROBLEM3"));
    p3->setSeverity(IProblem::Warning);
    p3->setFinalLocation(r3);

    DocumentRange r4;
    r4.document = IndexedString("/yet/another/test/path");

    p4->setDescription(QStringLiteral("PROBLEM4"));
    p4->setSeverity(IProblem::Hint);
    p4->setFinalLocation(r4);

    DocumentRange r5;
    r5.document = IndexedString("/yet/another/pathy/test/path");

    p5->setDescription(QStringLiteral("PROBLEM5"));
    p5->setSeverity(IProblem::Hint);
    p5->setFinalLocation(r5);

    DocumentRange r6;
    r6.document = IndexedString("/yet/another/test/pathy/path");

    p6->setDescription(QStringLiteral("PROBLEM6"));
    p6->setSeverity(IProblem::Hint);
    p6->setFinalLocation(r6);


    m_problems.push_back(p1);
    m_problems.push_back(p2);
    m_problems.push_back(p3);
    m_problems.push_back(p4);
    m_problems.push_back(p5);
    m_problems.push_back(p6);

    // Problem for diagnostic testing
    IProblem::Ptr p(new DetectedProblem());
    DocumentRange r;
    r.document = IndexedString("DIAGTEST");
    p->setFinalLocation(r);
    p->setDescription(QStringLiteral("PROBLEM"));
    p->setSeverity(IProblem::Error);

    IProblem::Ptr d(new DetectedProblem());
    d->setDescription(QStringLiteral("DIAG"));

    IProblem::Ptr dd(new DetectedProblem());
    dd->setDescription(QStringLiteral("DIAGDIAG"));
    d->addDiagnostic(dd);
    p->addDiagnostic(d);
    m_diagnosticTestProblem = p;
}

QTEST_MAIN(TestFilteredProblemStore)

#include "test_filteredproblemstore.moc"
