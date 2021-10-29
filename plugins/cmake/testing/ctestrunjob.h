/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CTESTRUNJOB_H
#define CTESTRUNJOB_H

#include <outputview/outputjob.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>

#include <QPointer>

class CTestSuite;

namespace KDevelop {
    class OutputModel;
}

class CTestRunJob : public KJob
{
    Q_OBJECT
public:
    CTestRunJob(CTestSuite* suite, const QStringList& cases, KDevelop::OutputJob::OutputJobVerbosity verbosity, QObject* parent = nullptr);
    void start() override;

protected:
    bool doKill() override;
    
private Q_SLOTS:
    void processFinished(KJob* job);
    void rowsInserted(const QModelIndex &parent, int startRow, int endRow);

private:
    CTestSuite* m_suite;
    QStringList m_cases;
    QHash<QString, KDevelop::TestResult::TestCaseResult> m_caseResults;
    QPointer<KJob> m_job;
    KDevelop::OutputModel* m_outputModel;
    KDevelop::OutputJob::OutputJobVerbosity m_verbosity;
};

#endif // CTESTRUNJOB_H
