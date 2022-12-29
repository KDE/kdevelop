/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCOMPOSITEJOBTEST_H
#define KCOMPOSITEJOBTEST_H

#include <QEventLoop>
#include <QObject>

#include "kcompositejob.h"

class TestJob : public KJob
{
    Q_OBJECT

public:
    explicit TestJob(QObject *parent = nullptr);

    /// Takes 1 second to finish
    void start() override;

private Q_SLOTS:
    void doEmit();
};

class CompositeJob : public KCompositeJob
{
    Q_OBJECT

public:
    explicit CompositeJob(QObject *parent = nullptr)
        : KCompositeJob(parent)
    {
    }

    void start() override;
    bool addSubjob(KJob *job) override;

protected Q_SLOTS:
    void slotResult(KJob *job) override;
};

class KCompositeJobTest : public QObject
{
    Q_OBJECT

public:
    KCompositeJobTest();

private Q_SLOTS:
    void testDeletionDuringExecution();

private:
    QEventLoop loop;
};

#endif // KCOMPOSITEJOBTEST_H
