/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "projecttestjob.h"
#include <interfaces/icore.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/itestsuite.h>
#include <KLocalizedString>

using namespace KDevelop;

struct ProjectTestJob::Private
{
    Private(ProjectTestJob* q)
        : q(q)
        , m_currentJob(0)
        , m_currentSuite(0)
    {}

    void runNext();
    void gotResult(ITestSuite* suite, const TestResult& result);

    ProjectTestJob* q;

    QList<ITestSuite*> m_suites;
    KJob* m_currentJob;
    ITestSuite* m_currentSuite;
    ProjectTestResult m_result;
};

void ProjectTestJob::Private::runNext()
{
    m_currentSuite = m_suites.takeFirst();
    m_currentJob = m_currentSuite->launchAllCases(ITestSuite::Silent);
    m_currentJob->start();
}

void ProjectTestJob::Private::gotResult(ITestSuite* suite, const TestResult& result)
{
    if (suite == m_currentSuite) {
        m_result.total++;
        q->emitPercent(m_result.total, m_result.total + m_suites.size());

        switch (result.suiteResult)
        {
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
    , d(new Private(this))
{
    setCapabilities(Killable);
    setObjectName(i18n("Run all tests in %1", project->name()));

    d->m_suites = ICore::self()->testController()->testSuitesForProject(project);
    connect(ICore::self()->testController(), SIGNAL(testRunFinished(KDevelop::ITestSuite*,KDevelop::TestResult)),
            SLOT(gotResult(KDevelop::ITestSuite*,KDevelop::TestResult)));
}

ProjectTestJob::~ProjectTestJob()
{

}

void ProjectTestJob::start()
{
    d->runNext();
}

bool ProjectTestJob::doKill()
{
    if (d->m_currentJob) {
        d->m_currentJob->kill();
    } else {
        d->m_suites.clear();
    }
    return true;
}

ProjectTestResult ProjectTestJob::testResult()
{
    return d->m_result;
}

#include "projecttestjob.moc"
