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

#include "ctestsuite.h"
#include "ctestlaunchconfigurationtype.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <KProcess>
#include <KDebug>
#include <QFileInfo>
#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>


using namespace KDevelop;

CTestSuite::CTestSuite(const QString& name, const KUrl& executable, IProject* project, const QStringList& args): QObject(),
m_url(executable),
m_name(name),
m_args(args),
m_project(project),
m_controller(0)
{
    m_launchType = new CTestLaunchConfigurationType();
    Q_ASSERT(project);
    kDebug() << name << executable << project->name();
}

CTestSuite::~CTestSuite()
{
    m_controller->removeTestSuite(this);
}

void CTestSuite::loadCases()
{
    kDebug() << "Loading test cases for suite" << m_name << m_url;
    m_cases.clear();
    if (!m_args.isEmpty())
    {
        m_cases << name();
        m_controller->addTestSuite(this);
        return;
    }
    QFileInfo info(m_url.toLocalFile());
    if (info.exists() && info.isExecutable())
    {
        m_process = new KProcess;
        m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        m_process->setProgram(m_url.toLocalFile(), QStringList() << "-functions");
        connect (m_process, SIGNAL(finished(int)), this, SLOT(loadCasesProcessFinished(int)));
        connect (m_process, SIGNAL(started()), this, SLOT(readFromProcess()));
        connect (m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(readFromProcess()));
        connect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromProcess()));
        kDebug() << "Starting a process to determine the test cases for" << m_name;
        m_process->start();
    }
    else
    {
        m_cases << m_url.toLocalFile(); // TODO: Remove
        m_controller->addTestSuite(this);
    }
}

KDevelop::ILaunchConfiguration* CTestSuite::launchCase(const QString& testCase) const
{
    return launchCases(QStringList() << testCase);
}

KDevelop::ILaunchConfiguration* CTestSuite::launchCases(const QStringList& testCases) const
{
    ILaunchConfiguration* launch = ICore::self()->runController()->createLaunchConfiguration(m_launchType, qMakePair<QString, QString>("test", "CTestLauncher"));

    KConfigGroup group = launch->config();
    group.writeEntry("TestExecutable", m_url);
    if (!m_cases.isEmpty())
    {
        group.writeEntry("TestCases", testCases);
    }
    else if (!m_args.isEmpty())
    {
        group.writeEntry("TestRunArguments", m_args);

    }
    group.sync();
    
    return launch;
}

KDevelop::ILaunchConfiguration* CTestSuite::launchAllCases() const
{
    return launchCases(cases());
}

KUrl CTestSuite::url() const
{
    return m_url;
}

QStringList CTestSuite::cases() const
{
    return m_cases;
}

QString CTestSuite::name() const
{
    return m_name;
}

KDevelop::IProject* CTestSuite::project() const
{
    return m_project;
}

QStringList CTestSuite::arguments()
{
    return m_args;
}

void CTestSuite::setTestController(ITestController* controller)
{
    m_controller = controller;
}

void CTestSuite::loadCasesProcessFinished(int exitCode)
{
    kDebug() << exitCode;
    if (!m_process)
    {
        kWarning() << "Loading cases finished but process is 0";
        return;
    }
    while(!m_process->atEnd())
    {
        QString line = m_process->readLine().trimmed();
        line.remove('(');
        line.remove(')');
        m_cases << line;
    }
    kDebug() << m_url << m_cases;
    m_controller->addTestSuite(this);
    
    m_process->deleteLater();
    m_process = 0;
}

void CTestSuite::readFromProcess()
{
    kDebug() << m_process->errorString();
}

