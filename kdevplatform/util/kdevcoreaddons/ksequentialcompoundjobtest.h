/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KSEQUENTIALCOMPOUNDJOBTEST_H
#define KSEQUENTIALCOMPOUNDJOBTEST_H

#include <KJob>

#include <QObject>

class TestJob : public KJob
{
    Q_OBJECT
public:
    void start() override
    {
        Q_EMIT started(this);
    }

    void callEmitResult()
    {
        emitResult();
    }

Q_SIGNALS:
    void started(KJob *job);
};

class KSequentialCompoundJobTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void runOneJob();
    void runTwoJobs();
};

#endif // KSEQUENTIALCOMPOUNDJOBTEST_H
