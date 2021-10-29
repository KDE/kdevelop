/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H

#include <QObject>

#include <KJob>

class TestJob : public KJob
{
    Q_OBJECT

public:
    void start() override
    {
        emit started(this);
    }

    void callEmitResult()
    {
        emitResult();
    }

Q_SIGNALS:
    void started(KJob* job);
};

class TestExecuteCompositeJob : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void runOneJob();
    void runTwoJobs();
};

#endif // KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
