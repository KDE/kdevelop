/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_QMAKEFILE_H
#define TEST_QMAKEFILE_H

#include <QObject>

class TestQMakeFile : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void varResolution_data();
    void varResolution();

    void referenceParser_data();
    void referenceParser();

    void libTarget_data();
    void libTarget();

    void defines_data();
    void defines();

    void replaceFunctions_data();
    void replaceFunctions();

    void qtIncludeDirs_data();
    void qtIncludeDirs();

    void testInclude();

    void globbing_data();
    void globbing();

    void benchGlobbing();
    void benchGlobbingNoPattern();
};

#endif // TEST_QMAKEFILE_H
