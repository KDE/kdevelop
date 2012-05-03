/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVELOP_PROJECTTESTJOB_H
#define KDEVELOP_PROJECTTESTJOB_H

#include <KJob>
#include <QList>

#include "utilexport.h"

namespace KDevelop {

class TestResult;
    
class IProject;
class ITestSuite;

struct KDEVPLATFORMUTIL_EXPORT ProjectTestResult
{
    int total;
    int passed;
    int failed;
    int error;
};

class KDEVPLATFORMUTIL_EXPORT ProjectTestJob : public KJob
{
    Q_OBJECT
public:
    explicit ProjectTestJob(IProject* project, QObject* parent = 0);
    virtual ~ProjectTestJob();
    virtual void start();
    
    ProjectTestResult testResult();
    
private Q_SLOTS:
    void runNext();
    void gotResult(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result);
    
protected:
    virtual bool doKill();
    
private:
    QList<ITestSuite*> m_suites;
    KJob* m_currentJob;
    ITestSuite* m_currentSuite;
    ProjectTestResult m_result;
};

}

#endif // KDEVELOP_PROJECTTESTJOB_H
