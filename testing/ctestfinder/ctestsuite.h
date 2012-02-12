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

#ifndef CTESTSUITE_H
#define CTESTSUITE_H

#include <interfaces/itestsuite.h>

#include <KUrl>

class KProcess;
namespace KDevelop {
class ITestController;
}

class CTestLaunchConfigurationType;

class CTestSuite : public QObject, public KDevelop::ITestSuite
{
    Q_OBJECT
public:
    CTestSuite(const QString& name, const KUrl& executable, KDevelop::IProject* project, const QStringList& args = QStringList());
    virtual ~CTestSuite();
    
    virtual KDevelop::ILaunchConfiguration* launchCase(const QString& testCase) const;
    virtual KDevelop::ILaunchConfiguration* launchCases(const QStringList& testCases) const;
    virtual KDevelop::ILaunchConfiguration* launchAllCases() const;
    virtual KUrl url() const;
    virtual QStringList cases() const;
    virtual QString name() const;
    virtual KDevelop::IProject* project() const;

    void loadCases();
    void setTestController(KDevelop::ITestController* controller);
    QStringList arguments();
    
private:
    KUrl m_url;
    QString m_name;
    QStringList m_cases;
    QStringList m_args;
    KDevelop::IProject* m_project;
    CTestLaunchConfigurationType* m_launchType;
    KDevelop::ITestController* m_controller;
    KProcess* m_process;

private slots:
    void loadCasesProcessFinished(int exitCode);
public slots:
    void readFromProcess();
};

#endif // CTESTSUITE_H
