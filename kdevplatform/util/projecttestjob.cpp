/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projecttestjob.h"
#include <interfaces/icore.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/itestsuite.h>
#include <KLocalizedString>

using namespace KDevelop;

class KDevelop::ProjectTestJobPrivate
{
public:
    explicit ProjectTestJobPrivate(ProjectTestJob* q)
        : q(q)
        , m_currentJob(nullptr)
        , m_currentSuite(nullptr)
    {}

    void runNext();
    void gotResult(ITestSuite* suite, const TestResult& result);

    ProjectTestJob* q;

    QList<ITestSuite*> m_suites;
    KJob* m_currentJob;
    ITestSuite* m_currentSuite;
    ProjectTestResult m_result;
};

void ProjectTestJobPrivate::runNext()
{
    m_currentSuite = m_suites.takeFirst();
    m_currentJob = m_currentSuite->launchAllCases(ITestSuite::Silent);
    m_currentJob->start();
}

void ProjectTestJobPrivate::gotResult(ITestSuite* suite, const TestResult& result)
{
    if (suite == m_currentSuite) {
        m_result.total++;
        q->emitPercent(m_result.total, m_result.total + m_suites.size());

        switch (result.suiteResult) {
        case TestResult::Passed:
            m_result.passed++;
            break;

        case TestResult::Failed:
            m_result.failed++;
            break;

        case TestResult::Error:
            m_result.error++;
            break;

        default:
            break;
        }

        if (m_suites.isEmpty()) {
            q->emitResult();
        } else {
            runNext();
        }
    }
}

ProjectTestJob::ProjectTestJob(IProject* project, QObject* parent)
    : KJob(parent)
    , d_ptr(new ProjectTestJobPrivate(this))
{
    Q_D(ProjectTestJob);

    setCapabilities(Killable);
    setObjectName(i18n("Run all tests in %1", project->name()));

    d->m_suites = ICore::self()->testController()->testSuitesForProject(project);
    connect(ICore::self()->testController(), &ITestController::testRunFinished,
            this, [this](ITestSuite* suite, const TestResult& result) {
        Q_D(ProjectTestJob);
        d->gotResult(suite, result);
    });
}

ProjectTestJob::~ProjectTestJob()
{

}

void ProjectTestJob::start()
{
    Q_D(ProjectTestJob);
    d->runNext();
}

bool ProjectTestJob::doKill()
{
    Q_D(ProjectTestJob);
    if (d->m_currentJob) {
        d->m_currentJob->kill();
    } else {
        d->m_suites.clear();
    }
    return true;
}

ProjectTestResult ProjectTestJob::testResult()
{
    Q_D(ProjectTestJob);
    return d->m_result;
}

#include "moc_projecttestjob.cpp"
