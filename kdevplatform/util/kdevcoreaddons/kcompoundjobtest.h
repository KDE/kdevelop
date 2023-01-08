/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCOMPOUNDJOBTEST_H
#define KCOMPOUNDJOBTEST_H

#include <QObject>

#include "kcompoundjob.h"

using namespace KDevCoreAddons;

class TestJob : public KJob
{
    Q_OBJECT

public:
    explicit TestJob(QObject *parent = nullptr);

    /// Takes 1 second to finish
    void start() override;

    using KJob::emitResult;
};

class KillableTestJob : public TestJob
{
    Q_OBJECT

public:
    explicit KillableTestJob(QObject *parent = nullptr);

protected:
    bool doKill() override;
};

class TestCompoundJob : public KCompoundJob
{
    Q_OBJECT

public:
    explicit TestCompoundJob(QObject *parent = nullptr)
        : KCompoundJob(parent)
    {
    }

    void start() override;

    using KCompoundJob::addSubjob;
    using KCompoundJob::clearSubjobs;

protected Q_SLOTS:
    void subjobFinished(KJob *job) override;
};

class KCompoundJobTest : public QObject
{
    Q_OBJECT

public:
    enum class Action { Finish, KillVerbosely, KillQuietly, Destroy };
    Q_ENUM(Action)

    KCompoundJobTest();

private Q_SLOTS:
    void initTestCase();

    void testDeletionDuringExecution_data();
    void testDeletionDuringExecution();

    void testFinishingSubjob_data();
    void testFinishingSubjob();
};

#endif // KCOMPOUNDJOBTEST_H
