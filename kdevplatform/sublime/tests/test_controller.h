/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_CONTROLLER_H
#define KDEVPLATFORM_TEST_CONTROLLER_H

#include <QObject>

class TestController: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void documentDeletion();
    void areaDeletion();
    void namedAreas();
};

#endif
