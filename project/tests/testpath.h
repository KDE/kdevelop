/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
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

#ifndef TESTPATH_H
#define TESTPATH_H

#include <QObject>

class TestPath : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void bench_kurl();
    void bench_qurl();
    void bench_qstringlist();
    void bench_path();
    void bench_fromLocalPath();
    void bench_fromLocalPath_data();

    void testPath();
    void testPath_data();
    void testPathInvalid();
    void testPathInvalid_data();
    void testPathOperators();
    void testPathOperators_data();
    void testPathAddData();
    void testPathAddData_data();
    void testPathBaseCtor();
    void testPathBaseCtor_data();
};

#endif // TESTPATH_H
