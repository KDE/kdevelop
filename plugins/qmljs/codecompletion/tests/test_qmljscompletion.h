/*
    SPDX-FileCopyrightText: 2011-2012 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMLCOMPLETIONTEST_H
#define QMLCOMPLETIONTEST_H

#include <QObject>

namespace QmlJS {

class QmlCompletionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testContainsDeclaration();
    void testContainsDeclaration_data();
    void testDoesNotContainDeclaration();
    void testDoesNotContainDeclaration_data();
    void testContainsText();
    void testContainsText_data();
};

}
#endif
