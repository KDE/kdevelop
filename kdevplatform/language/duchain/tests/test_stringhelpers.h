/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_STRINGHELPERS_H
#define KDEVPLATFORM_TEST_STRINGHELPERS_H

#include <QObject>

class TestStringHelpers
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testFormatComment_data();
    void testFormatComment();

    void benchFormatComment();

    void testParamIterator_data();
    void testParamIterator();
};

#endif // KDEVPLATFORM_TEST_STRINGHELPERS_H
