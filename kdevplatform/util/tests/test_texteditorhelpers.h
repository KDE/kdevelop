/*
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTKTEXTEDITORHELPERS_H
#define TESTKTEXTEDITORHELPERS_H

#include <QObject>

class TestKTextEditorHelpers : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testExtractCursor();
    void testExtractCursor_data();
};

#endif
