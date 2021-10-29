/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BENCH_HASHES_H
#define KDEVPLATFORM_BENCH_HASHES_H

#include <QObject>

class BenchHashes
    : public QObject
{
    Q_OBJECT

private:
    void feedData();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void insert();
    void insert_data();
    void find();
    void find_data();
    void constFind();
    void constFind_data();
    void remove();
    void remove_data();
    void typeRepo();
    void typeRepo_data();
};

#endif // KDEVPLATFORM_BENCH_HASHES_H
