/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEXERTEST_H
#define LEXERTEST_H

#include <QObject>
#include <QTest>

class LexerTest : public QObject
{
        Q_OBJECT
    public:
        explicit LexerTest( QObject* parent = nullptr );
        ~LexerTest() override;
    private Q_SLOTS:
        void varAssignment();
        void varAssignment_data();
        void operators();
        void operators_data();
        void scope();
        void scope_data();
        void functions();
        void functions_data();

};

#endif

