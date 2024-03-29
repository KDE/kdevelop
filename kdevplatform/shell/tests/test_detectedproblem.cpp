/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <shell/problem.h>
#include <KLocalizedString>

#include <serialization/itemrepositoryregistry.h>
#include <interfaces/icore.h>
#include <language/editor/documentrange.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

using namespace KDevelop;

class TestDetectedProblem : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testSource();
    void testSeverity();
    void testDescription();
    void testExplanation();
    void testFinalLocation();
    void testDiagnostics();
    void testPluginName();

private:
    IProblem::Ptr m_problem;
};

void TestDetectedProblem::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_problem = new DetectedProblem();
}

void TestDetectedProblem::cleanupTestCase()
{
    TestCore::shutdown();
}

struct Source
{
    IProblem::Source source;
    QString sourceString;
};

void TestDetectedProblem::testSource()
{
    static const Source sources[] =
    {
        { IProblem::Unknown, i18n("Unknown") },
        { IProblem::Disk, i18n("Disk") },
        { IProblem::Preprocessor, i18n("Preprocessor") },
        { IProblem::Lexer, i18n("Lexer") },
        { IProblem::Parser, i18n("Parser") },
        { IProblem::DUChainBuilder, i18n("DuchainBuilder") },
        { IProblem::SemanticAnalysis, i18n("Semantic analysis") },
        { IProblem::ToDo, i18n("Todo") },
        { IProblem::Plugin, i18n("Plugin") }
    };

    for (auto& source : sources) {
        m_problem->setSource(source.source);

        QCOMPARE(source.source, m_problem->source());
        QCOMPARE(source.sourceString, m_problem->sourceString());
    }

}

struct Severity
{
    IProblem::Severity severity;
    QString severityString;
};

void TestDetectedProblem::testSeverity()
{
    static const Severity severities[] =
    {
        { IProblem::Error, i18n("Error") },
        { IProblem::Warning, i18n("Warning") },
        { IProblem::Hint, i18n("Hint") },
    };

    for (auto& severity : severities) {
        m_problem->setSeverity(severity.severity);

        QCOMPARE(severity.severity, m_problem->severity());
        QCOMPARE(severity.severityString, m_problem->severityString());
    }
}

void TestDetectedProblem::testDescription()
{
    QString TESTDESCRIPTION = QStringLiteral("Just a test description");

    m_problem->setDescription(TESTDESCRIPTION);
    QCOMPARE(TESTDESCRIPTION, m_problem->description());
}

void TestDetectedProblem::testExplanation()
{
    QString TESTEXPLANATION = QStringLiteral("Just a test explanation");

    m_problem->setExplanation(TESTEXPLANATION);
    QCOMPARE(TESTEXPLANATION, m_problem->explanation());
}

void TestDetectedProblem::testFinalLocation()
{
    QString TESTPATH = QStringLiteral("/just/a/bogus/path/to/a/fake/document");
    int TESTLINE     = 9001;
    int TESTCOLUMN   = 1337;

    DocumentRange range;
    range.document = IndexedString(TESTPATH);
    range.setBothLines(TESTLINE);
    range.setBothColumns(TESTCOLUMN);

    m_problem->setFinalLocation(range);

    QCOMPARE(TESTPATH, m_problem->finalLocation().document.str());
    QCOMPARE(TESTLINE, m_problem->finalLocation().start().line());
    QCOMPARE(TESTLINE, m_problem->finalLocation().end().line());
    QCOMPARE(TESTCOLUMN, m_problem->finalLocation().start().column());
    QCOMPARE(TESTCOLUMN, m_problem->finalLocation().end().column());
}

void TestDetectedProblem::testDiagnostics()
{
    QString one   = QStringLiteral("One");
    QString two   = QStringLiteral("Two");
    QString three = QStringLiteral("Three");

    IProblem::Ptr p1(new DetectedProblem());
    IProblem::Ptr p2(new DetectedProblem());
    IProblem::Ptr p3(new DetectedProblem());

    p1->setDescription(one);
    p2->setDescription(two);
    p3->setDescription(three);

    QCOMPARE(m_problem->diagnostics().size(), 0);

    m_problem->addDiagnostic(p1);
    m_problem->addDiagnostic(p2);
    m_problem->addDiagnostic(p3);

    QCOMPARE(m_problem->diagnostics().size(), 3);
    QCOMPARE(m_problem->diagnostics().at(0)->description(), one);
    QCOMPARE(m_problem->diagnostics().at(1)->description(), two);
    QCOMPARE(m_problem->diagnostics().at(2)->description(), three);

    m_problem->clearDiagnostics();
    QCOMPARE(0, m_problem->diagnostics().size());

    QVector<IProblem::Ptr> diags;
    diags.push_back(p3);
    diags.push_back(p2);
    diags.push_back(p1);
    m_problem->setDiagnostics(diags);

    QCOMPARE(m_problem->diagnostics().size(), 3);
    QCOMPARE(m_problem->diagnostics().at(2)->description(), one);
    QCOMPARE(m_problem->diagnostics().at(1)->description(), two);
    QCOMPARE(m_problem->diagnostics().at(0)->description(), three);

    m_problem->clearDiagnostics();
    QCOMPARE(m_problem->diagnostics().size(), 0);
}

void TestDetectedProblem::testPluginName()
{
    DetectedProblem p1(QStringLiteral("Plugin1"));
    DetectedProblem p2(QStringLiteral("Plugin2"));
    DetectedProblem p3(QStringLiteral(""));
    DetectedProblem p4;

    QCOMPARE(p1.source(), IProblem::Plugin);
    QCOMPARE(p2.source(), IProblem::Plugin);
    QCOMPARE(p3.source(), IProblem::Plugin);
    QCOMPARE(p4.source(), IProblem::Unknown);

    QCOMPARE(p1.sourceString(), QStringLiteral("Plugin1"));
    QCOMPARE(p2.sourceString(), QStringLiteral("Plugin2"));
    QCOMPARE(p3.sourceString(), QStringLiteral(""));
    QCOMPARE(p4.sourceString(), i18n("Unknown"));

    p4.setSource(IProblem::Plugin);
    QCOMPARE(p4.source(), IProblem::Plugin);
    QCOMPARE(p4.sourceString(), i18n("Plugin"));
}

QTEST_MAIN(TestDetectedProblem)

#include "test_detectedproblem.moc"
