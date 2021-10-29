/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTDOCUMENTCHANGESET_H
#define TESTDOCUMENTCHANGESET_H

#include <QObject>

class TestDocumentchangeset : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testReplaceSameLine();
};

#endif // TESTDOCUMENTCHANGESET_H
