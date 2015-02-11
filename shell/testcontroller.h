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

#ifndef KDEVPLATFORM_TESTCONTROLLER_H
#define KDEVPLATFORM_TESTCONTROLLER_H

#include "interfaces/itestcontroller.h"

#include "shellexport.h"

namespace KDevelop {

class KDEVPLATFORMSHELL_EXPORT TestController : public KDevelop::ITestController
{
    Q_OBJECT

public:
    explicit TestController(QObject *parent);
    virtual ~TestController();

    void initialize();
    void cleanup();

    virtual void removeTestSuite(KDevelop::ITestSuite* suite) override;
    virtual void addTestSuite(KDevelop::ITestSuite* suite) override;

    virtual QList< KDevelop::ITestSuite* > testSuites() const override;
    virtual KDevelop::ITestSuite* findTestSuite(KDevelop::IProject* project, const QString& name) const override;
    virtual QList< KDevelop::ITestSuite* > testSuitesForProject(KDevelop::IProject* project) const override;

    virtual void notifyTestRunFinished(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result) override;
    virtual void notifyTestRunStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases) override;

private:
    class TestControllerPrivate;
    TestControllerPrivate* const d;
};

}

#endif // KDEVPLATFORM_TESTCONTROLLER_H
