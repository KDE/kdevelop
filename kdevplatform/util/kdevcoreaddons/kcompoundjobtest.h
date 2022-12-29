/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCOMPOUNDJOBTEST_H
#define KCOMPOUNDJOBTEST_H

#include <QEventLoop>
#include <QObject>

#include "kcompoundjob.h"

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

class CompositeJob : public KCompoundJob
{
    Q_OBJECT

public:
    explicit CompositeJob(QObject *parent = nullptr)
        : KCompoundJob(parent)
    {
    }

    void start() override;
    bool addSubjob(KJob *job) override;

protected Q_SLOTS:
    void slotResult(KJob *job) override;
};

class KCompoundJobTest : public QObject
{
    Q_OBJECT

public:
    KCompoundJobTest();

private Q_SLOTS:
    void testDeletionDuringExecution();

private:
    QEventLoop loop;
};

#endif // KCOMPOUNDJOBTEST_H
