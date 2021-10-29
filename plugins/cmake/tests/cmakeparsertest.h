/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKELEXERTEST_H
#define CMAKELEXERTEST_H

#include <QTest>

class CMakeAst;

/**
 * A test for the CMake parser and lexer
 * @author Matt Rogers <mattr@kde.org>
 */
class CMakeParserTest : public QObject
{
Q_OBJECT
public:
    CMakeParserTest();
    ~CMakeParserTest() override;

private Q_SLOTS:
    void testLexerCreation();
    void testLexerWithFile();

    void testParserWithGoodData();
    void testParserWithGoodData_data();

    void testParserWithBadData();
    void testParserWithBadData_data();

    //void testAstCreation();

    // void testWhitespaceHandling();
};

#endif
