/*
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_MODELS_H
#define KDEVPLATFORM_TEST_MODELS_H

#include <QObject>


class TestModels : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testVcsFileChangesModel();
};

#endif // KDEVPLATFORM_TEST_MODELS_H
