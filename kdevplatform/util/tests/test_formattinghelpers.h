/*
    SPDX-FileCopyrightText: 2019 Bernd Buschinski <b.buschinski@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

class TestFormattingHelpers : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testFuzzyMatching();
    void testFuzzyMatching_data();
};
