/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
