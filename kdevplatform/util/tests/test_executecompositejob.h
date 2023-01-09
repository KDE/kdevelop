/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H

#include <QObject>

class TestExecuteCompositeJob : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void create_data();
    void create();
};

#endif // KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
