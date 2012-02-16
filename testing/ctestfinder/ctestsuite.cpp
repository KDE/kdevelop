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
#include "ctestrunjob.h"

#include <KProcess>
#include <KDebug>
#include <QFileInfo>
#include <interfaces/itestcontroller.h>
#include <interfaces/iproject.h>


using namespace KDevelop;

CTestSuite::CTestSuite(const QString& name, const KUrl& executable, IProject* project, const QStringList& args): 
m_url(executable),
m_name(name),
m_args(args),
m_project(project)
{
    m_url.cleanPath();
    Q_ASSERT(project);
    kDebug() << m_name << m_url << m_project->name();
}

CTestSuite::~CTestSuite()
{

}

void CTestSuite::loadCases()
{
    kDebug() << "Loading test cases for suite" << m_name << m_url;
    m_cases.clear();
    QFileInfo info(m_url.toLocalFile());
    
    if (m_args.isEmpty() && info.exists() && info.isExecutable())
    {
        KProcess process;
        process.setOutputChannelMode(KProcess::OnlyStdoutChannel);
        process.setProgram(m_url.toLocalFile(), QStringList() << "-functions");
        process.start();
        if (process.waitForFinished())
        {       
            while(!process.atEnd())
            {
                QString line = process.readLine().trimmed();
                line.remove('(');
                line.remove(')');
                m_cases << line;
            }
        }
    }
}

KJob* CTestSuite::launchCase(const QString& testCase) const
{
    return launchCases(QStringList() << testCase);
}

KJob* CTestSuite::launchCases(const QStringList& testCases) const
{
    kDebug() << "Launching test run" << m_name << "with cases" << testCases;
    
    return new CTestRunJob(this, testCases);
}

KJob* CTestSuite::launchAllCases() const
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

QStringList CTestSuite::arguments() const
{
    return m_args;
}

