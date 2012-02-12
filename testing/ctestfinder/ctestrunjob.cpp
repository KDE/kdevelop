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
#include <util/processlinemaker.h>
#include <outputview/outputmodel.h>
#include <KConfigGroup>
#include <KProcess>
#include <KDebug>

using namespace KDevelop;

CTestRunJob::CTestRunJob(KDevelop::ILaunchConfiguration* cfg, QObject* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity): OutputJob(parent, verbosity), 
m_configuration(cfg)
{
    setStandardToolView(KDevelop::IOutputView::TestView);
    QString name = cfg->config().readEntry("TestSuiteName");
    setObjectName("Test: " + name);
    setTitle(name);
}

void CTestRunJob::start()
{
    const KConfigGroup group = m_configuration->config();
    QString executable = group.readEntry("TestExecutable");
    QStringList cases = group.readEntry("TestCases", QStringList());
    QStringList arguments = group.readEntry("TestArguments", QStringList());
    
    kDebug() << "Starting test job" << group.readEntry("TestSuiteName");
    
    if (arguments.isEmpty())
    {
        // In QTestLib-based tests, the names of the functions can be passed as arguments to the test executable
        arguments = cases;
    }
    KDevelop::OutputModel* outputModel = new KDevelop::OutputModel;
    setModel( outputModel, KDevelop::IOutputView::TakeOwnership );
    
    m_process = new KProcess(this);
    m_process->setProgram(executable, arguments);
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    
    kDebug() << m_process->program();
    
    m_lineMaker = new ProcessLineMaker(m_process, this);
    startOutput();
    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)), outputModel, SLOT(appendLines(QStringList)));
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
    kDebug() << line;
    emitResult();
}

void CTestRunJob::processError()
{
    setErrorText(m_process->errorString());
    emitResult();
}

