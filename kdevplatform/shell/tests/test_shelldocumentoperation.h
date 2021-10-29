/*
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_SHELLDOCUMENTOPERATION_H
#define KDEVPLATFORM_TEST_SHELLDOCUMENTOPERATION_H

#include <QObject>

class TestShellDocumentOperation: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testOpenDocumentFromText();
    void testClosing();
    void testKateDocumentAndViewCreation();

};

#endif
