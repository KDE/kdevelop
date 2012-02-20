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
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itestcontroller.h>
#include <util/processlinemaker.h>
#include <outputview/outputmodel.h>
#include <KConfigGroup>
#include <KProcess>
#include <KDebug>
#include "ctestsuite.h"

using namespace KDevelop;

CTestRunJob::CTestRunJob(CTestSuite* suite, const QStringList& cases, QObject* parent): OutputJob(parent), 
m_suite(suite),
m_cases(cases),
m_process(0),
m_lineMaker(0)
{
    foreach (const QString& testCase, cases)
    {
        m_caseResults[testCase] = TestResult::NotRun;
    }
}



void CTestRunJob::start()
{
    KDevelop::OutputModel* outputModel = new KDevelop::OutputModel;
    setModel( outputModel, KDevelop::IOutputView::TakeOwnership );
    
    QStringList arguments = m_cases;
    if (m_cases.isEmpty() && !m_suite->arguments().isEmpty())
    {
        arguments = m_suite->arguments();
    }
    
    m_process = new KProcess(this);
    m_process->setProgram(m_suite->url().toLocalFile(), arguments);
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    
    kDebug() << m_process->program();
    
    m_lineMaker = new ProcessLineMaker(m_process, this);
    startOutput();
    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)), SLOT(receivedLines(QStringList)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
    m_process->start();
}

bool CTestRunJob::doKill()
{
    if (m_process)
    {
        m_process->kill();
    }
    return true;
}

void CTestRunJob::processFinished(int exitCode)
{
    QString line = QString("*** Test process exited with exit code %1 ***").arg(exitCode);
    
    TestResult result;
    result.testCaseResults = m_caseResults;
    m_suite->setResult(result);

    ITestController* tc = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    tc->notifyTestRunFinished(m_suite);
    
    kDebug() << line;
    emitResult();
}

void CTestRunJob::processError()
{
    setErrorText(m_process->errorString());
    emitResult();
}

void CTestRunJob::receivedLines(const QStringList& lines)
{
    foreach (const QString& line, lines )
    {
        // TODO: Highlight parts of the line
        qobject_cast<OutputModel*>(model())->appendLine(line);
    
        if (!line.contains("()"))
        {
            continue;
        }
        
        QString testCase = line.split("()").first();
        if (line.contains("::"))
        {
            testCase = testCase.split("::").last();
        }
        else
        {
            testCase = testCase.split(' ').last();
        }
        
        if (m_suite->cases().contains(testCase))
        {
            TestResult::TestCaseResult result = TestResult::NotRun;
            if (line.startsWith("PASS"))
            {
                result = TestResult::Passed;
            }
            else if (line.startsWith("FAIL") || line.startsWith("XFAIL"))
            {
                result = TestResult::Failed;
            }
            else if (line.startsWith("SKIP"))
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
