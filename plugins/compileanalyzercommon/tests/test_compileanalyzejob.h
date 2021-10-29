/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPILEANALYZER_COMPILEANALYZEJOB_TEST_H
#define COMPILEANALYZER_COMPILEANALYZEJOB_TEST_H

// Qt
#include <QObject>

class TestCompileAnalyzeJob : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testJob();
};

#endif
