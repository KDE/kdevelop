/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef TEST_CHECKGROUP_H
#define TEST_CHECKGROUP_H

#include <QObject>

namespace ClangTidy {
class CheckGroup;
}

struct CheckGroupData;
struct CheckStateGroupData;

class TestCheckGroup : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testFromPlainList_data();
    void testFromPlainList();
    void testSetEnabledChecks_data();
    void testSetEnabledChecks();

private:
    void doTestResult(const ClangTidy::CheckGroup* actualValue, const CheckGroupData& expectedValue);
    void doTestResult(const ClangTidy::CheckGroup* actualValue, const CheckStateGroupData& expectedValue);
};

#endif
