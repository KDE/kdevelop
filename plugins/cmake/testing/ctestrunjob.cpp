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
#include <debug.h>

#include <algorithm>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilauncher.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/ilaunchmode.h>
#include <util/executecompositejob.h>
#include <outputview/outputmodel.h>

#include <KConfigGroup>
#include <KLocalizedString>

#include <utility>

using namespace KDevelop;

CTestRunJob::CTestRunJob(CTestSuite* suite, const QStringList& cases, OutputJob::OutputJobVerbosity verbosity, QObject* parent)
: KJob(parent)
, m_suite(suite)
, m_cases(cases)
, m_job(nullptr)
, m_outputModel(nullptr)
, m_verbosity(verbosity)
, m_started{false}
{
    Q_ASSERT(m_suite);
    setCapabilities(Killable);

    //TODO!
}


static KJob* createTestJob(const QString& launchModeId, const QStringList& arguments, const QString &workingDirectory, IProject* project)
{
    LaunchConfigurationType* type = ICore::self()->runController()->launchConfigurationTypeForId( QStringLiteral("Native Application") );
    ILaunchMode* mode = ICore::self()->runController()->launchModeForId( launchModeId );

    qCDebug(CMAKE) << "got mode and type:" << type << type->id() << mode << mode->id();
    Q_ASSERT(type && mode);

    ILauncher* launcher = [type, mode]() {
        const auto launchers = type->launchers();
        auto it = std::find_if(launchers.begin(), launchers.end(), [mode](ILauncher *l) {
            return l->supportedModes().contains(mode->id());
        });
        Q_ASSERT(it != launchers.end());
        return *it;
    }();
    Q_ASSERT(launcher);

    auto ilaunch = [type, project]() {
        const auto launchConfigurations = ICore::self()->runController()->launchConfigurations();
        auto it = std::find_if(launchConfigurations.begin(), launchConfigurations.end(),
                            [type, project](ILaunchConfiguration* l) {
                                return (l->type() == type && l->project() == project && l->config().readEntry(QStringLiteral("ConfiguredByCTest"), false));
                            });
        return it == launchConfigurations.end() ? nullptr : *it;
    }();

    if (!ilaunch) {
        ilaunch = ICore::self()->runController()->createLaunchConfiguration( type,
                                                qMakePair( mode->id(), launcher->id() ),
                                                project,
                                                i18n("CTest") );
        ilaunch->config().writeEntry(QStringLiteral("ConfiguredByCTest"), true);
        //qCDebug(CMAKE) << "created config, launching";
    } else {
        //qCDebug(CMAKE) << "reusing generated config, launching";
    }
    if (!workingDirectory.isEmpty())
        ilaunch->config().writeEntry( "Working Directory", QUrl::fromLocalFile( workingDirectory ) );
    type->configureLaunchFromCmdLineArguments( ilaunch->config(), arguments );
    return ICore::self()->runController()->execute(launchModeId, ilaunch);
}

void CTestRunJob::start()
{
    Q_ASSERT(!m_started);
    if (!m_suite) {
        qCDebug(CMAKE) << "Cannot start running tests after being killed.";
        return;
    }

    m_started = true;
    for (const QString& testCase : m_cases) {
        m_caseResults[testCase] = TestResult::NotRun;
    }

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
    const QString workingDirectory = m_suite->properties().value(QStringLiteral("WORKING_DIRECTORY"), QString());

    m_job = createTestJob(QStringLiteral("execute"), arguments, workingDirectory, m_suite->project());

    if (auto* cjob = qobject_cast<ExecuteCompositeJob*>(m_job)) {
        auto* outputJob = cjob->findChild<OutputJob*>();
        Q_ASSERT(outputJob);
        outputJob->setVerbosity(m_verbosity);

        QString testName = m_suite->name();
        QString title;
        if (cases_selected.count() == 1)
            title = i18nc("running test %1, %2 test case", "CTest %1: %2", testName, cases_selected.value(0));
        else
            title = i18ncp("running test %1, %2 number of test cases", "CTest %2 (%1)", "CTest %2 (%1)", cases_selected.count(), testName);

        outputJob->setTitle(title);

        m_outputModel = qobject_cast<OutputModel*>(outputJob->model());
        connect(m_outputModel, &QAbstractItemModel::rowsInserted, this, &CTestRunJob::rowsInserted);
    }
    connect(m_job, &KJob::finished, this, &CTestRunJob::processFinished);

    ICore::self()->testController()->notifyTestRunStarted(m_suite, cases_selected);
}

bool CTestRunJob::doKill()
{
    // Check m_started here so as to call notifyTestRunFinished() only if we have called notifyTestRunStarted().
    if (m_started) {
        if (m_job) {
            // Killing m_job invokes processFinished() => set m_job to nullptr before killing it.
            auto* const job = m_job;
            m_job = nullptr;
            job->kill();
        }

        if (m_suite) {
            qCDebug(CMAKE) << "Reporting incomplete test results because this job is being killed.";
            TestResult result{std::move(m_caseResults), TestResult::Error};
            ICore::self()->testController()->notifyTestRunFinished(m_suite, std::move(result));
        }
    }

    Q_ASSERT(!m_job);
    m_suite = nullptr;

    return true;
}

void CTestRunJob::processFinished(KJob* job)
{
    if (!m_job) {
        return; // this CTestRunJob has been killed.
    }
    Q_ASSERT_X(m_suite, Q_FUNC_INFO, "Whenever m_suite is nullptr, m_job must be nullptr too.");
    Q_ASSERT(m_job == job);
    m_job = nullptr;

    int error = job->error();
    auto finished = [this,error]() {
        if (!m_suite) {
            return; // this CTestRunJob has been killed.
        }

        TestResult result;
        result.testCaseResults = m_caseResults;
        if (error == OutputJob::FailedShownError) {
            result.suiteResult = TestResult::Failed;
        } else if (error == KJob::NoError) {
            result.suiteResult = TestResult::Passed;
        } else {
            result.suiteResult = TestResult::Error;
        }

        // in case the job was killed, mark this job as killed as well
        if (error == KJob::KilledJobError) {
            setError(KJob::KilledJobError);
            setErrorText(QStringLiteral("Child job was killed."));
        }

        qCDebug(CMAKE) << result.suiteResult << result.testCaseResults;
        ICore::self()->testController()->notifyTestRunFinished(m_suite, result);
        m_suite = nullptr;
        emitResult();
    };

    if (m_outputModel)
    {
        connect(m_outputModel, &OutputModel::allDone, this, finished, Qt::QueuedConnection);
        m_outputModel->ensureAllDone();
    }
    else
    {
        finished();
    }
}

void CTestRunJob::rowsInserted(const QModelIndex &parent, int startRow, int endRow)
{
    if (!m_suite) {
        return; // this CTestRunJob has been killed.
    }

    // This regular expression matches the name of the testcase (whatever between "::" and "(", indeed )
    // For example, from:
    //      PASS   : ExpTest::testExp(sum)
    // matches "testExp"
    static QRegExp caseRx(QStringLiteral("::(.*)\\("), Qt::CaseSensitive, QRegExp::RegExp2);
    for (int row = startRow; row <= endRow; ++row)
    {
        QString line = m_outputModel->data(m_outputModel->index(row, 0, parent), Qt::DisplayRole).toString();

        QString testCase;
        if (caseRx.indexIn(line) >= 0) {
            testCase = caseRx.cap(1);
        }

        TestResult::TestCaseResult prevResult = m_caseResults.value(testCase, TestResult::NotRun);
        if (prevResult == TestResult::Passed || prevResult == TestResult::NotRun)
        {
            TestResult::TestCaseResult result = TestResult::NotRun;
            const bool expectFail = m_suite->properties().value(QStringLiteral("WILL_FAIL"), QStringLiteral("FALSE")) == QLatin1String("TRUE");
            if (line.startsWith(QLatin1String("PASS   :")))
            {
                result = expectFail ? TestResult::UnexpectedPass : TestResult::Passed;
            }
            else if (line.startsWith(QLatin1String("FAIL!  :")))
            {
                result = expectFail ? TestResult::ExpectedFail : TestResult::Failed;
            }
            else if (line.startsWith(QLatin1String("XFAIL  :")))
            {
                result = TestResult::ExpectedFail;
            }
            else if (line.startsWith(QLatin1String("XPASS  :")))
            {
                result = TestResult::UnexpectedPass;
            }
            else if (line.startsWith(QLatin1String("SKIP   :")))
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
