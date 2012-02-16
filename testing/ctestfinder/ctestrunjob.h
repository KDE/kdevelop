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

class CTestSuite;
class KProcess;

namespace KDevelop {
class ILaunchConfiguration;
class ProcessLineMaker;
}


class CTestRunJob : public KDevelop::OutputJob
{
    Q_OBJECT
public:
    CTestRunJob(const CTestSuite* suite, const QStringList& cases, QObject* parent = 0);
    virtual void start();

protected:
    virtual bool doKill();
    
private:
    const CTestSuite* m_suite;
    QStringList m_cases;
    KProcess* m_process;
    KDevelop::ProcessLineMaker* m_lineMaker;
    
public slots:
    void processFinished(int exitCode);
    void processError();
};

#endif // CTESTRUNJOB_H
