/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_AREAWALKER_H
#define KDEVPLATFORM_TEST_AREAWALKER_H
#include <QObject>

class TestAreaWalker: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void viewWalkerModes();
    void toolViewWalkerModes();
};

#endif
