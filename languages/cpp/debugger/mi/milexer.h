/***************************************************************************
 *   Copyright (C) 2004 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MILEXER_H
#define MILEXER_H

#include <qmemarray.h>
#include <qmap.h>
#include <qstring.h>

class MILexer;
class TokenStream;

typedef void (MILexer::*scan_fun_ptr)(int *kind);

struct Token
{
    int kind;
    int position;
    int length;
};

struct FileSymbol
{
    QCString contents;
    TokenStream *tokenStream;

    inline FileSymbol()
        : tokenStream(0) {}

    inline ~FileSymbol();
};

struct TokenStream
{
    inline int lookAhead(int n = 0) const
    { return (m_currentToken + n)->kind; }

    inline int currentToken() const
    { return m_currentToken->kind; }

    inline QCString currentTokenText() const
    { return tokenText(-1); }

    inline QCString tokenText(int index = 0) const
    {
        Token *t = index < 0 ? m_currentToken : m_firstToken + index;
        return m_contents.mid(t->position, t->length);
    }

    inline int lineOffset(int line) const
    { return m_lines.at(line); }

    void positionAt(int position, int *line, int *column) const;

    inline void getTokenStartPosition(int index, int *line, int *column) const
    { positionAt((m_firstToken + index)->position, line, column); }

    inline void getTokenEndPosition(int index, int *line, int *column) const
    {
        Token *tk = m_firstToken + index;
        positionAt(tk->position + tk->length, line, column);
    }

    inline void rewind(int index)
    { m_currentToken = m_firstToken + index; }

    inline int cursor() const
    { return m_currentToken - m_firstToken; }

    inline void nextToken()
    { m_currentToken++; m_cursor++; }

//private:
    QCString m_contents;

    QMemArray<int> m_lines;
    int m_line;

    QMemArray<Token> m_tokens;
    int m_tokensCount;

    Token *m_firstToken;
    Token *m_currentToken;

    int m_cursor;
};

class MILexer
{
public:
    MILexer();
    ~MILexer();

    TokenStream *tokenize(const FileSymbol *fileSymbol);

private:
    int nextToken(int &position, int &len);

    void scanChar(int *kind);
    void scanUnicodeChar(int *kind);
    void scanNewline(int *kind);
    void scanWhiteSpaces(int *kind);
    void scanStringLiteral(int *kind);
    void scanNumberLiteral(int *kind);
    void scanIdentifier(int *kind);

    void setupScanTable();

private:
    static bool s_initialized;
    static scan_fun_ptr s_scan_table[128 + 1];

    QCString m_contents;
    int m_ptr;

    QMemArray<int> m_lines;
    int m_line;

    QMemArray<Token> m_tokens;
    int m_tokensCount;

    int m_cursor;
};

inline FileSymbol::~FileSymbol()
{
    delete tokenStream;
    tokenStream = 0;
}


#endif
