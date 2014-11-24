/*
 * This file is part of KDevelop
 * Copyright 2012-2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TESTINDEXEDSTRING_H
#define TESTINDEXEDSTRING_H

#include <QObject>

class TestIndexedString : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void testUrl_data();
    void testUrl();

    void bench_index();
    void bench_length();
    void bench_qstring();
    void bench_kurl();
    void bench_qhashQString();
    void bench_qhashIndexedString();
    void bench_hashString();
    void bench_kdevhash();
    void bench_qSet();

    void test();
    void test_data();
};

#endif // TESTINDEXEDSTRING_H
