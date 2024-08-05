/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include <shell/problemmodel.h>
#include <shell/problem.h>
#include <shell/problemconstants.h>
#include <language/editor/documentrange.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <KLocalizedString>

#define MYCOMPARE(actual, expected) \
    if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) \
    return false

#define MYVERIFY(statement) \
    if (!QTest::qVerify((statement), #statement, "", __FILE__, __LINE__))\
        return false

using namespace KDevelop;

class TestProblemModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testNoGrouping();
    void testPathGrouping();
    void testSeverityGrouping();
    void testPlaceholderText();

private:
    QString prependPathRoot(QString const& currentPath) const;
    void generateProblems();
    bool checkIsSame(int row, const QModelIndex &parent, const IProblem::Ptr &problem);
    bool checkDiagnostics(int row, const QModelIndex &parent);
    bool checkDisplay(int row, const QModelIndex &parent, const IProblem::Ptr &problem);
    bool checkLabel(int row, const QModelIndex &parent, const QString &label);
    bool checkPathGroup(int row, const IProblem::Ptr &problem);
    bool checkSeverityGroup(int row, const IProblem::Ptr &problem);

    QScopedPointer<ProblemModel> m_model;
    QVector<IProblem::Ptr> m_problems;
    IProblem::Ptr m_diagnosticTestProblem;
};

void TestProblemModel::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_model.reset(new ProblemModel(nullptr));
    m_model->setScope(BypassScopeFilter);

    generateProblems();
}

void TestProblemModel::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProblemModel::testNoGrouping()
{
    m_model->setGrouping(NoGrouping);
    m_model->setSeverity(IProblem::Hint);

    QCOMPARE(m_model->rowCount(), 0);

    // Check if setting the problems works
    m_model->setProblems(m_problems);
    QCOMPARE(m_model->rowCount(), 3);

    for (int i = 0; i < m_model->rowCount(); i++) {
        QVERIFY(checkIsSame(i, QModelIndex(), m_problems[i]));
    }

    // Check if displaying various data parts works
    QVERIFY(checkDisplay(0, QModelIndex(), m_problems[0]));

    // Check if clearing the problems works
    m_model->clearProblems();
    QCOMPARE(m_model->rowCount(), 0);

    // Check if adding the problems works
    int c = 0;
    for (const IProblem::Ptr& p : std::as_const(m_problems)) {
        m_model->addProblem(p);
        c++;

        QCOMPARE(m_model->rowCount(), c);
    }

    for (int i = 0; i < m_model->rowCount(); i++) {
        QVERIFY(checkIsSame(i, QModelIndex(), m_problems[i]));
    }


    // Check if filtering works
    // old-style setSeverity
    // Error filter
    m_model->setSeverity(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));

    // Warning filter
    m_model->setSeverity(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));
    QVERIFY(checkIsSame(1, QModelIndex(), m_problems[1]));

    // Hint filter
    m_model->setSeverity(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));
    QVERIFY(checkIsSame(1, QModelIndex(), m_problems[1]));
    QVERIFY(checkIsSame(2, QModelIndex(), m_problems[2]));

    // Check if filtering works
    // new style
    // Error filter
    m_model->setSeverities(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));

    // Warning filter
    m_model->setSeverities(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[1]));

    // Hint filter
    m_model->setSeverities(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[2]));

    // Error + Hint filter
    m_model->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));
    QVERIFY(checkIsSame(1, QModelIndex(), m_problems[2]));

    m_model->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    // Check if diagnostics are added properly
    m_model->clearProblems();
    m_model->addProblem(m_diagnosticTestProblem);
    QVERIFY(checkDiagnostics(0, QModelIndex()));

    m_model->clearProblems();
}

void TestProblemModel::testPathGrouping()
{
    m_model->setGrouping(PathGrouping);
    m_model->setSeverity(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 0);

    // Check if setting problems works
    m_model->setProblems(m_problems);
    QCOMPARE(m_model->rowCount(), 3);

    for (int i = 0; i < m_model->rowCount(); i++) {
        QVERIFY(checkLabel(i, QModelIndex(), m_problems[i]->finalLocation().document.str()));

        QModelIndex idx = m_model->index(i, 0);
        QVERIFY(idx.isValid());
        QVERIFY(checkIsSame(0, idx, m_problems[i]));
    }

    // Check if displaying various data parts works
    {
        QModelIndex idx = m_model->index(0, 0);
        QVERIFY(idx.isValid());
        QVERIFY(checkDisplay(0, idx, m_problems[0]));
    }

    // Check if clearing works
    m_model->clearProblems();
    QCOMPARE(m_model->rowCount(), 0);

    // Check if add problems works
    int c = 0;
    for (const IProblem::Ptr& p : std::as_const(m_problems)) {
        m_model->addProblem(p);
        c++;

        QCOMPARE(m_model->rowCount(), c);
    }

    for (int i = 0; i < m_model->rowCount(); i++) {
        QVERIFY(checkLabel(i, QModelIndex(), m_problems[i]->finalLocation().document.str()));

        QModelIndex idx = m_model->index(i, 0);
        QVERIFY(idx.isValid());
        QVERIFY(checkIsSame(0, idx, m_problems[i]));
    }

    // Check if filtering works
    // old-style setSeverity
    // Error filtering
    m_model->setSeverity(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkPathGroup(0, m_problems[0]));

    // Warning filtering
    m_model->setSeverity(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkPathGroup(0, m_problems[0]));
    QVERIFY(checkPathGroup(1, m_problems[1]));

    // Hint filtering
    m_model->setSeverity(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);
    QVERIFY(checkPathGroup(0, m_problems[0]));
    QVERIFY(checkPathGroup(1, m_problems[1]));
    QVERIFY(checkPathGroup(2, m_problems[2]));

    // Check if filtering works
    // new style
    // Error filtering
    m_model->setSeverities(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkPathGroup(0, m_problems[0]));

    // Warning filtering
    m_model->setSeverities(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkPathGroup(0, m_problems[1]));

    // Hint filtering
    m_model->setSeverities(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkPathGroup(0, m_problems[2]));

    // Error + Hint filtering
    m_model->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkPathGroup(0, m_problems[0]));
    QVERIFY(checkPathGroup(1, m_problems[2]));

    m_model->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    // Check if diagnostics get to the right place
    m_model->clearProblems();
    m_model->addProblem(m_diagnosticTestProblem);
    {
        QModelIndex parent = m_model->index(0, 0);
        QVERIFY(parent.isValid());
        checkDiagnostics(0, parent);
    }

    m_model->clearProblems();
}

void TestProblemModel::testSeverityGrouping()
{
    m_model->setGrouping(SeverityGrouping);
    m_model->setSeverity(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);

    // Check if setting problems works
    m_model->setProblems(m_problems);
    QCOMPARE(m_model->rowCount(), 3);
    for (int i = 0; i < m_model->rowCount(); i++) {
        QVERIFY(checkSeverityGroup(i, m_problems[i]));
    }

    // Check if displaying works
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex parent = m_model->index(i, 0);
        QVERIFY(parent.isValid());

        QVERIFY(checkDisplay(0, parent, m_problems[i]));
    }

    // Check if clearing works
    m_model->clearProblems();
    QCOMPARE(m_model->rowCount(), 3);

    // Check if adding problems works
    int c = 0;
    for (const IProblem::Ptr& p : std::as_const(m_problems)) {
        m_model->addProblem(p);
        QVERIFY(checkSeverityGroup(c, m_problems[c]));
        c++;
    }

    // Check if filtering works
    // old-style setSeverity
    // Error filtering
    m_model->setSeverity(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(0, m_problems[0]);

    // Warning filtering
    m_model->setSeverity(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(0, m_problems[0]);
    checkSeverityGroup(1, m_problems[1]);

    // Hint filtering
    m_model->setSeverity(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(0, m_problems[0]);
    checkSeverityGroup(1, m_problems[1]);
    checkSeverityGroup(2, m_problems[2]);

    // Check if filtering works
    // Error filtering
    m_model->setSeverities(IProblem::Error);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(0, m_problems[0]);

    // Warning filtering
    m_model->setSeverities(IProblem::Warning);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(1, m_problems[1]);

    // Hint filtering
    m_model->setSeverities(IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(2, m_problems[2]);

    // Error + Hint filtering
    m_model->setSeverities(IProblem::Error | IProblem::Hint);
    QCOMPARE(m_model->rowCount(), 3);
    checkSeverityGroup(0, m_problems[0]);
    checkSeverityGroup(2, m_problems[2]);

    m_model->setSeverities(IProblem::Error | IProblem::Warning | IProblem::Hint);

    // Check if diagnostics get to the right place
    m_model->clearProblems();
    m_model->addProblem(m_diagnosticTestProblem);
    {
        QModelIndex parent = m_model->index(0, 0);
        QVERIFY(parent.isValid());

        checkDiagnostics(0, parent);
    }

    m_model->clearProblems();
}

void TestProblemModel::testPlaceholderText()
{
    const QString text1 = QStringLiteral("testPlaceholderText1");
    const QString text2 = QStringLiteral("testPlaceholderText2");
    const QString empty;

    m_model->setGrouping(NoGrouping);

    // Test model with empty placeholder text

    QCOMPARE(m_model->rowCount(), 0);
    m_model->setPlaceholderText(empty);
    QCOMPARE(m_model->rowCount(), 0);
    m_model->setProblems(m_problems);
    QCOMPARE(m_model->rowCount(), 3);
    m_model->clearProblems();
    QCOMPARE(m_model->rowCount(), 0);

    // Test empty model with non-empty placeholder text

    m_model->setPlaceholderText(text1);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkLabel(0, QModelIndex(), text1));

    m_model->setPlaceholderText(text2);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkLabel(0, QModelIndex(), text2));

    // Test non-empty model with non-empty placeholder text

    m_model->setProblems(m_problems);
    QCOMPARE(m_model->rowCount(), 3);

    m_model->clearProblems();
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkLabel(0, QModelIndex(), text2));

    m_model->addProblem(m_problems[0]);
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));

    m_model->addProblem(m_problems[1]);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkIsSame(1, QModelIndex(), m_problems[1]));

    m_model->setPlaceholderText(text1);
    QCOMPARE(m_model->rowCount(), 2);
    QVERIFY(checkIsSame(0, QModelIndex(), m_problems[0]));
    QVERIFY(checkIsSame(1, QModelIndex(), m_problems[1]));

    m_model->setProblems({});
    QCOMPARE(m_model->rowCount(), 1);
    QVERIFY(checkLabel(0, QModelIndex(), text1));
}

// Needed because util/Path.cpp asserts e.g. C: on Windows
QString TestProblemModel::prependPathRoot(QString const& currentPath) const
{
#ifdef Q_OS_WIN
    QString pathRoot = QStringLiteral("x:/");
#else
    QString pathRoot = QStringLiteral("/");
#endif
    return pathRoot + currentPath;
}

// Generate 3 problems, all with different paths, different severity
// Also generates a problem with diagnostics
void TestProblemModel::generateProblems()
{
    IProblem::Ptr p1(new DetectedProblem());
    IProblem::Ptr p2(new DetectedProblem());
    IProblem::Ptr p3(new DetectedProblem());

    DocumentRange r1;
    r1.document = IndexedString( prependPathRoot("just/a/random/path") );

    p1->setDescription(QStringLiteral("PROBLEM1"));
    p1->setSeverity(IProblem::Error);
    p1->setFinalLocation(r1);

    DocumentRange r2;
    r2.document = IndexedString( prependPathRoot("just/another/path") );

    p2->setDescription(QStringLiteral("PROBLEM2"));
    p2->setSeverity(IProblem::Warning);
    p2->setFinalLocation(r2);

    DocumentRange r3;
    r3.document = IndexedString( prependPathRoot("yet/another/test/path") );

    p2->setDescription(QStringLiteral("PROBLEM3"));
    p3->setSeverity(IProblem::Hint);
    p3->setFinalLocation(r3);

    m_problems.push_back(p1);
    m_problems.push_back(p2);
    m_problems.push_back(p3);

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

bool TestProblemModel::checkIsSame(int row, const QModelIndex &parent, const IProblem::Ptr &problem)
{
    QModelIndex idx;

    idx = m_model->index(row, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->description());

    return true;
}

bool TestProblemModel::checkDiagnostics(int row, const QModelIndex &parent)
{
    MYCOMPARE(m_model->rowCount(parent), 1);

    QModelIndex idx;
    idx = m_model->index(row, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), m_diagnosticTestProblem->description());

    QModelIndex diagidx;
    IProblem::Ptr diag = m_diagnosticTestProblem->diagnostics().at(0);
    diagidx = m_model->index(0, 0, idx);
    MYVERIFY(diagidx.isValid());
    MYCOMPARE(m_model->data(diagidx).toString(), diag->description());

    QModelIndex diagdiagidx;
    IProblem::Ptr diagdiag = diag->diagnostics().at(0);
    diagdiagidx = m_model->index(0, 0, diagidx);
    MYVERIFY(diagdiagidx.isValid());
    MYCOMPARE(m_model->data(diagdiagidx).toString(), diagdiag->description());

    return true;
}

bool TestProblemModel::checkDisplay(int row, const QModelIndex &parent, const IProblem::Ptr &problem)
{
    QModelIndex idx;

    idx = m_model->index(row, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->description());

    idx = m_model->index(row, 1, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->sourceString());

    idx = m_model->index(row, 2, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->finalLocation().document.str());

    idx = m_model->index(row, 3, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), QString::number(problem->finalLocation().start().line() + 1));

    idx = m_model->index(row, 4, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), QString::number(problem->finalLocation().start().column() + 1));

    return true;
}

bool TestProblemModel::checkLabel(int row, const QModelIndex &parent, const QString &label)
{
    QModelIndex idx = m_model->index(row, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), label);

    return true;
}

bool TestProblemModel::checkPathGroup(int row, const IProblem::Ptr &problem)
{
    QModelIndex parent = m_model->index(row, 0);
    MYVERIFY(parent.isValid());
    MYCOMPARE(m_model->data(parent).toString(), problem->finalLocation().document.str());

    QModelIndex idx = m_model->index(0, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->description());

    return true;
}

bool TestProblemModel::checkSeverityGroup(int row, const IProblem::Ptr &problem)
{
    QModelIndex parent = m_model->index(row, 0);
    MYVERIFY(parent.isValid());
    MYCOMPARE(m_model->data(parent).toString(), problem->severityString());

    QModelIndex idx = m_model->index(0, 0, parent);
    MYVERIFY(idx.isValid());
    MYCOMPARE(m_model->data(idx).toString(), problem->description());

    return true;
}

QTEST_MAIN(TestProblemModel)

#include "test_problemmodel.moc"
