/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTIDENTIFIER_H
#define KDEVPLATFORM_TESTIDENTIFIER_H

#include <QObject>

class TestIdentifier
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testIdentifier();
    void testIdentifier_data();

    void testQualifiedIdentifier();
    void testQualifiedIdentifier_data();

    void benchIdentifierCopyConstant();
    void benchIdentifierCopyDynamic();
    void benchQidCopyPush();
};

#endif // KDEVPLATFORM_TESTIDENTIFIER_H
