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

#ifndef CTESTRUNJOB_H
#define CTESTRUNJOB_H

#include <outputview/outputjob.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>

class CTestSuite;
class KProcess;

namespace KDevelop {
class ILaunchConfiguration;
class ProcessLineMaker;
}


class CTestRunJob : public KJob
{
    Q_OBJECT
public:
    CTestRunJob(CTestSuite* suite, const QStringList& cases, KDevelop::OutputJob::OutputJobVerbosity verbosity, bool expectFail, QObject* parent = 0);
    virtual void start();

protected:
    virtual bool doKill();
    
private slots:
    void processFinished(KJob* job);
    void rowsInserted(const QModelIndex &parent, int startRow, int endRow);

private:
    CTestSuite* m_suite;
    QStringList m_cases;
    QHash<QString, KDevelop::TestResult::TestCaseResult> m_caseResults;
    KJob* m_job;
    KDevelop::OutputJob* m_outputJob;
    KDevelop::OutputJob::OutputJobVerbosity m_verbosity;
    bool m_expectFail;
};

#endif // CTESTRUNJOB_H
