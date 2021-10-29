/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKELEXER_H
#define QMAKELEXER_H

#include <util/stack.h>
#include <QString>
#include "parser_export.h"

class QString;
class kdev_pg_location_table;

namespace QMake
{

class Parser;

class KDEVQMAKEPARSER_EXPORT Lexer {
public:
    Lexer(Parser* _parser, QString  contents);

    int nextTokenKind();
    qint64 tokenBegin() const;
    qint64 tokenEnd() const;

private:
    QString m_content;
    Parser* m_parser;
    int m_curpos;
    int m_contentSize;
    qint64 m_tokenBegin;
    qint64 m_tokenEnd;

    int state() const;
    void pushState(int state);
    void popState();

    QChar* ignoreWhitespaceAndComment(QChar* it);
    void createNewline( int pos );

    KDevelop::Stack<int> mState;
    enum State
    {
        ErrorState = -1,
        DefaultState = 0,
        ContState = 2,
        VariableValueState = 4,
        FunctionArgState = 5
    };

};

}

#endif

