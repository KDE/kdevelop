/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BENCH_INDEXEDSTRING_H
#define BENCH_INDEXEDSTRING_H

#include <QDir>
#include <QObject>

class BenchIndexedString
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void bench_index();
    void bench_length();
    void bench_qstring();
    void bench_kurl();
    void bench_qhashQString();
    void bench_qhashIndexedString();
    void bench_hashString();
    void bench_kdevhash();
    void bench_qSet();

    void bench_create();
    void bench_destroy();
    void bench_swap();

private:
    const QString m_repositoryPath = QDir::tempPath() + QStringLiteral("/bench_indexedstring");
};

#endif // BENCH_INDEXEDSTRING_H
