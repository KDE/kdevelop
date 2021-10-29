/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTCONTROLLER_H
#define KDEVPLATFORM_TESTCONTROLLER_H

#include "interfaces/itestcontroller.h"

#include "shellexport.h"

namespace KDevelop {
class TestControllerPrivate;

class KDEVPLATFORMSHELL_EXPORT TestController : public KDevelop::ITestController
{
    Q_OBJECT

public:
    explicit TestController(QObject *parent);
    ~TestController() override;

    void initialize();
    void cleanup();

    void removeTestSuite(KDevelop::ITestSuite* suite) override;
    void addTestSuite(KDevelop::ITestSuite* suite) override;

    QList< KDevelop::ITestSuite* > testSuites() const override;
    KDevelop::ITestSuite* findTestSuite(KDevelop::IProject* project, const QString& name) const override;
    QList< KDevelop::ITestSuite* > testSuitesForProject(KDevelop::IProject* project) const override;

    void notifyTestRunFinished(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result) override;
    void notifyTestRunStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases) override;

private:
    const QScopedPointer<class TestControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(TestController)
};

}

#endif // KDEVPLATFORM_TESTCONTROLLER_H
