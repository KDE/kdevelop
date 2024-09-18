/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_DOCUMENT_H
#define KDEVPLATFORM_TEST_DOCUMENT_H

#include <QObject>

class TestDocument: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testUrlDocument_data();
    void testUrlDocument();

    void viewDeletion();
};

#endif
