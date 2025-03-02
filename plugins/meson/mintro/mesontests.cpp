/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesontests.h"
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>
#include <debug.h>
#include <interfaces/iproject.h>
#include <kjob.h>
#include <outputview/outputexecutejob.h>
#include <util/executecompositejob.h>

#include <KLocalizedString>

using namespace std;
using namespace KDevelop;

namespace {
class MesonTestJob : public OutputExecuteJob
{
public:
    explicit MesonTestJob(QObject* parent = nullptr, OutputJobVerbosity verbosity = OutputJob::Verbose)
        : OutputExecuteJob(parent, verbosity)
    {
        setCapabilities(Killable);
        setStandardToolView(IOutputView::RunView);
        setBehaviours(IOutputView::AllowUserClose | IOutputView::AutoScroll);
        setFilteringStrategy(OutputModel::NativeAppErrorFilter);
        setProperties(DisplayStdout | DisplayStderr);
    }
};
} // unnamed namespace

// Class MesonTest

MesonTest::MesonTest(const QJsonObject& json, IProject* project)
    : m_project(project)
{
    fromJson(json);
}

MesonTest::~MesonTest() {}

QString MesonTest::name() const
{
    return m_name;
}

QStringList MesonTest::suites() const
{
    return m_suites;
}

IProject* MesonTest::project() const
{
    return m_project;
}

KJob* MesonTest::job(ITestSuite::TestJobVerbosity verbosity)
{
    auto convVerbosity = [verbosity]() {
        switch (verbosity) {
        case KDevelop::ITestSuite::Verbose:
            return OutputJob::Verbose;
        case KDevelop::ITestSuite::Silent:
            return OutputJob::Silent;
        }
        Q_UNREACHABLE();
    }();

    auto* const job = new MesonTestJob(m_project, convVerbosity);
    *job << m_command;
    if (!m_workDir.isEmpty()) {
        job->setWorkingDirectory(m_workDir.toUrl());
    }
    job->setJobName(i18nc("%1 - test case name", "Meson Test \"%1\"", m_name));
    for (auto i = begin(m_env); i != end(m_env); ++i) {
        job->addEnvironmentOverride(i.key(), i.value());
    }
    return job;
}

void MesonTest::fromJson(const QJsonObject& json)
{
    m_name = json[QStringLiteral("name")].toString();
    m_workDir = Path(json[QStringLiteral("workdir")].toString());

    QJsonArray cmd = json[QStringLiteral("cmd")].toArray();
    QJsonArray suites = json[QStringLiteral("suite")].toArray();
    QJsonObject env = json[QStringLiteral("env")].toObject();

    transform(begin(cmd), end(cmd), back_inserter(m_command), [](const auto& x) { return x.toString(); });
    transform(begin(suites), end(suites), back_inserter(m_suites), [](const auto& x) { return x.toString(); });

    for (auto i = begin(env); i != end(env); ++i) {
        m_env[i.key()] = i.value().toString();
    }

    qCDebug(KDEV_Meson) << "MINTRO:   - Loaded test" << m_name << "suites:" << m_suites;
}

// Class MesonTestSuite

MesonTestSuite::MesonTestSuite(QString name, IProject* project)
    : m_name(name)
    , m_project(project)
{
    qCDebug(KDEV_Meson) << "MINTRO:   - New test suite" << m_name;
}

MesonTestSuite::~MesonTestSuite() {}

QString MesonTestSuite::name() const
{
    return m_name;
}

QStringList MesonTestSuite::cases() const
{
    QStringList result;
    for (auto i : m_tests) {
        result << i->name();
    }
    return result;
}

IProject* MesonTestSuite::project() const
{
    return m_project;
}

KJob* MesonTestSuite::launchCase(const QString& testCase, TestJobVerbosity verbosity)
{
    auto iter = m_tests.find(testCase);
    if (iter == end(m_tests)) {
        return nullptr;
    }

    return (*iter)->job(verbosity);
}

KJob* MesonTestSuite::launchCases(const QStringList& testCases, TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    for (const auto& i : testCases) {
        auto iter = m_tests.find(i);
        if (iter == end(m_tests)) {
            continue;
        }

        jobs << (*iter)->job(verbosity);
    }

    return new ExecuteCompositeJob(m_project, jobs);
}

KJob* MesonTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    for (auto& i : m_tests) {
        jobs << i->job(verbosity);
    }

    return new ExecuteCompositeJob(m_project, jobs);
}

IndexedDeclaration MesonTestSuite::declaration() const
{
    return IndexedDeclaration();
}

IndexedDeclaration MesonTestSuite::caseDeclaration(const QString&) const
{
    return IndexedDeclaration();
}

void MesonTestSuite::addTestCase(MesonTestPtr test)
{
    if (!test) {
        qCDebug(KDEV_Meson) << "TESTS: Tried to add a nullptr test";
        return;
    }

    m_tests[test->name()] = test;
}

QHash<QString, MesonTestPtr> MesonTestSuite::tests()
{
    return m_tests;
}

// Class MesonTestSuites

MesonTestSuites::MesonTestSuites(const QJsonArray& json, IProject* project)
    : m_project(project)
{
    fromJSON(json);
}

MesonTestSuites::~MesonTestSuites() {}

QHash<QString, MesonTestSuitePtr> MesonTestSuites::testSuites()
{
    return m_suites;
}

MesonTestSuitePtr MesonTestSuites::testSuite(QString name)
{
    auto iter = m_suites.find(name);
    if (iter == end(m_suites)) {
        return nullptr;
    }
    return *iter;
}

MesonTestSuitePtr MesonTestSuites::operator[](QString name)
{
    return testSuite(name);
}

void MesonTestSuites::fromJSON(const QJsonArray& json)
{
    QVector<MesonTestPtr> tests;
    qCDebug(KDEV_Meson) << "MINTRO: -- Loading tests from JSON...";
    for (const auto& i : json) {
        tests << make_shared<MesonTest>(i.toObject(), m_project);
    }

    qCDebug(KDEV_Meson) << "MINTRO: -- Adding tests to suites";
    for (auto& i : tests) {
        for (QString j : i->suites()) {
            auto suite = testSuite(j);
            if (!suite) {
                suite = make_shared<MesonTestSuite>(j, m_project);
                m_suites[j] = suite;
            }
            suite->addTestCase(i);
            qCDebug(KDEV_Meson) << "MINTRO:   - Added test" << i->name() << "to suite" << suite->name();
        }
    }
}
