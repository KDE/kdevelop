/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestrunjob.h"
#include "ctestsuite.h"
#include "qttestdelegate.h"
#include "../debug.h"

#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilauncher.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/ilaunchmode.h>
#include <interfaces/iprojectcontroller.h>
#include <util/processlinemaker.h>
#include <util/executecompositejob.h>
#include <outputview/outputmodel.h>

#include <KConfigGroup>
#include <KProcess>
#include <KLocalizedString>
#include <KCompositeJob>
#include <QFile>

using namespace KDevelop;

CTestRunJob::CTestRunJob(CTestSuite* suite, const QStringList& cases, OutputJob::OutputJobVerbosity verbosity, bool expectFail, QObject* parent)
: KJob(parent)
, m_suite(suite)
, m_cases(cases)
, m_job(0)
, m_outputJob(0)
, m_verbosity(verbosity)
, m_expectFail(expectFail)
{
    foreach (const QString& testCase, cases)
    {
        m_caseResults[testCase] = TestResult::NotRun;
    }

    setCapabilities(Killable);
}


KJob* createTestJob(const QString &launchModeId, const QStringList &arguments, IProject* p)
{
    LaunchConfigurationType* type = ICore::self()->runController()->launchConfigurationTypeForId( "Native Application" );
    ILaunchMode* mode = ICore::self()->runController()->launchModeForId( launchModeId );

    qCDebug(CMAKE) << "got mode and type:" << type << type->id() << mode << mode->id() << "for" << arguments;
    Q_ASSERT(type && mode);

    ILauncher* launcher = 0;
    foreach (ILauncher *l, type->launchers())
    {
        //qCDebug(CMAKE) << "avaliable launcher" << l << l->id() << l->supportedModes();
        if (l->supportedModes().contains(mode->id())) {
            launcher = l;
            break;
        }
    }
    Q_ASSERT(launcher);

    ILaunchConfiguration* ilaunch = 0;
    QList<ILaunchConfiguration*> launchConfigurations = ICore::self()->runController()->launchConfigurations();
    foreach (ILaunchConfiguration *l, launchConfigurations) {
        if (l->type() == type && l->config().readEntry("ConfiguredByCTest", false)) {
            ilaunch = l;
            break;
        }
    }
    if (!ilaunch) {
        ilaunch = ICore::self()->runController()->createLaunchConfiguration( type,
                                                qMakePair( mode->id(), launcher->id() ),
                                                p,
                                                i18n("CTest") );
        ilaunch->config().writeEntry("ConfiguredByCTest", true);
        //qCDebug(CMAKE) << "created config, launching";
    } else {
        //qCDebug(CMAKE) << "reusing generated config, launching";
    }
    type->configureLaunchFromCmdLineArguments( ilaunch->config(), arguments );
    return ICore::self()->runController()->execute(launchModeId, ilaunch);
}

void CTestRunJob::start()
{
//     if (!m_suite->cases().isEmpty())
//     {
        // TODO: Find a better way of determining whether QTestLib is used by this test
//         qCDebug(CMAKE) << "Setting a QtTestDelegate";
//         setDelegate(new QtTestDelegate);
//     }
//     setStandardToolView(IOutputView::RunView);

    QStringList arguments = m_cases;
    if (m_cases.isEmpty() && !m_suite->arguments().isEmpty())
    {
        arguments = m_suite->arguments();
    }

    QStringList cases_selected = arguments;
    arguments.prepend(m_suite->executable().toLocalFile());
    Q_ASSERT(QFile::exists(m_suite->executable().toLocalFile()));
    m_job = createTestJob("execute", arguments, m_suite->project());

    m_outputJob = m_job->findChild<OutputJob*>(QString(), Qt::FindDirectChildrenOnly);
    Q_ASSERT(m_outputJob);

    m_outputJob->setVerbosity(m_verbosity);

    QString testName = arguments.value(0).split('/').last();
    QString title;
    if (cases_selected.count() == 1)
        title = i18nc("running test %1, %2 test case", "CTest %1: %2", testName, cases_selected.value(0));
    else
        title = i18ncp("running test %1, %2 number of test cases", "CTest %2 (%1)", "CTest %2 (%1)", cases_selected.count(), testName);

    m_outputJob->setTitle(title);
    connect(m_outputJob->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_job, SIGNAL(finished(KJob*)), SLOT(processFinished(KJob*)));

    ICore::self()->testController()->notifyTestRunStarted(m_suite, cases_selected);
}

bool CTestRunJob::doKill()
{
    if (m_job)
    {
        m_job->kill();
    }
    return true;
}

void CTestRunJob::processFinished(KJob* job)
{
    TestResult result;
    result.testCaseResults = m_caseResults;
    if (job->error() == OutputJob::FailedShownError) {
        result.suiteResult = TestResult::Failed;
    } else if (job->error() == KJob::NoError) {
        result.suiteResult = TestResult::Passed;
    } else {
        result.suiteResult = TestResult::Error;
    }

    // in case the job was killed, mark this job as killed as well
    if (job->error() == KJob::KilledJobError) {
        setError(KJob::KilledJobError);
        setErrorText("Child job was killed.");
    }

    qCDebug(CMAKE) << result.suiteResult << result.testCaseResults;
    ICore::self()->testController()->notifyTestRunFinished(m_suite, result);
    emitResult();
}

void CTestRunJob::rowsInserted(const QModelIndex &parent, int startRow, int endRow)
{
    // This regular expresion matches the name of the testcase (whatever between "::" and "(", indeed )
    // For example, from:
    //      PASS   : ExpTest::testExp(sum)
    // matches "testExp"
    static QRegExp caseRx("::(.*)\\(", Qt::CaseSensitive, QRegExp::RegExp2);
    for (int row = startRow; row <= endRow; ++row)
    {
        QString line = m_outputJob->model()->data(m_outputJob->model()->index(row, 0, parent), Qt::DisplayRole).toString();

        QString testCase;
        if (caseRx.indexIn(line) >= 0) {
            testCase = caseRx.cap(1);
        }

        TestResult::TestCaseResult prevResult = m_caseResults.value(testCase, TestResult::NotRun);
        if (prevResult == TestResult::Passed || prevResult == TestResult::NotRun)
        {
            TestResult::TestCaseResult result = TestResult::NotRun;
            if (line.startsWith("PASS   :"))
            {
                result = m_expectFail ? TestResult::UnexpectedPass : TestResult::Passed;
            }
            else if (line.startsWith("FAIL!  :"))
            {
                result = m_expectFail ? TestResult::ExpectedFail : TestResult::Failed;
            }
            else if (line.startsWith("XFAIL  :"))
            {
                result = TestResult::ExpectedFail;
            }
            else if (line.startsWith("XPASS  :"))
            {
                result = TestResult::UnexpectedPass;
            }
            else if (line.startsWith("SKIP   :"))
            {
                result = TestResult::Skipped;
            }

            if (result != TestResult::NotRun)
            {
                m_caseResults[testCase] = result;
            }
        }
    }
}
