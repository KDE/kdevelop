/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_VIEW_H
#define KDEVPLATFORM_TEST_VIEW_H

#include <QObject>

class TestView: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void widgetDeletion();
    void viewReimplementation();
};

#endif
