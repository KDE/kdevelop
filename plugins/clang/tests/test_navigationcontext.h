/*
    SPDX-FileCopyrightText: 2026 Yin Zhijie <depressedjie@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTNAVIGATIONCONTEXT_H
#define TESTNAVIGATIONCONTEXT_H

#include <QObject>

class TestNavigationContext : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testDeclarationDetailsVirtualFinalAbstract();
};

#endif // TESTNAVIGATIONCONTEXT_H
