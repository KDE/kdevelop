/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BENCH_INDEXEDSTRING_H
#define BENCH_INDEXEDSTRING_H

#include "itemrepositorytestbase.h"

class BenchIndexedString
    : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
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

    void bench_string_vector_data();
    void bench_string_vector();
};

#endif // BENCH_INDEXEDSTRING_H
