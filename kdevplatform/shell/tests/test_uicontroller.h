/*
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_UICONTROLLER_H
#define KDEVPLATFORM_TEST_UICONTROLLER_H

#include <QObject>

class TestUiController : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void showErrorMessage();
};


#endif // KDEVPLATFORM_TEST_UICONTROLLER_H
