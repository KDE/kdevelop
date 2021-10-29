/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTPATH_H
#define TESTPATH_H

#include <QObject>

class TestPath : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void bench_qurl();
    void bench_qstringlist();
    void bench_path();
    void bench_fromLocalPath();
    void bench_fromLocalPath_data();
    void bench_swap();
    void bench_hash();

    void testPath();
    void testPath_data();
    void testPathInvalid();
    void testPathInvalid_data();
    void testPathComparison();
    void testPathComparison_data();
    void testPathSwap();
    void testPathAddData();
    void testPathAddData_data();
    void testPathBaseCtor();
    void testPathBaseCtor_data();
    void testPathCd();
    void testPathCd_data();
    void testHasParent_data();
    void testHasParent();

    void QUrl_acceptance();
};

#endif // TESTPATH_H
