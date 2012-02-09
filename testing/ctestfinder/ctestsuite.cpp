/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestsuite.h"
#include <KProcess>

CTestSuite::CTestSuite(const QString& name, const KUrl& executable, const QStringList& args) :
m_url(executable),
m_name(name),
m_args(args)
{
    loadCases();
}

CTestSuite::~CTestSuite()
{

}

void CTestSuite::loadCases()
{
    m_cases.clear();
    if (!m_args.isEmpty())
    {
        m_cases.clear();
        m_cases << QString();
    }
    KProcess process;
    process.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    process.setProgram(m_url.toLocalFile(), QStringList() << "-functions");
    process.start();
    if (!process.waitForFinished())
    {
        return;
    }
    while(!process.atEnd())
    {
        QString line = process.readLine().trimmed();
        line.remove('(');
        line.remove(')');
        m_cases << line;
    }
}

KDevelop::ILaunchConfiguration* CTestSuite::launchCase(const QString& testCase) const
{
    return launchCases(QStringList() << testCase);
}

KDevelop::ILaunchConfiguration* CTestSuite::launchCases(const QStringList& testCases) const
{
    // TODO: Implement
    return 0;
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
