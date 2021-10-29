/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEV_TESTMIPARSER_H
#define KDEV_TESTMIPARSER_H

#include <QObject>

namespace KDevMI { namespace MI { struct Value; }}


class TestMIParser : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testParseLine_data();
    void testParseLine();

private:
    void doTestResult(const KDevMI::MI::Value& actualValue, const QVariant& expectedValue);
};

#endif
