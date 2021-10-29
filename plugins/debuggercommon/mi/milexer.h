/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MILEXER_H
#define MILEXER_H

#include <QVector>

namespace KDevMI { namespace MI {

class MILexer;
struct TokenStream;

using scan_fun_ptr = void (MILexer::*)(int*);

struct Token
{
    int kind;
    int position;
    int length;
};

struct FileSymbol
{
    QByteArray contents;
    TokenStream *tokenStream = nullptr;

    inline FileSymbol() {}

    inline ~FileSymbol();

private:
    Q_DISABLE_COPY(FileSymbol)
};

struct TokenStream
{
    inline int lookAhead(int n = 0) const
    { return (m_currentToken + n)->kind; }

    inline int currentToken() const
    { return m_currentToken->kind; }

    inline QByteArray currentTokenText() const
    { return tokenText(-1); }

    QByteArray tokenText(int index = 0) const;

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
    QByteArray m_contents;

    QVector<int> m_lines;
    int m_line;

    QVector<Token> m_tokens;
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

    QByteArray m_contents;
    int m_ptr = 0;
    // Cached 'm_contents.length()'
    int m_length = 0;

    QVector<int> m_lines;
    int m_line = 0;

    QVector<Token> m_tokens;
    int m_tokensCount = 0;

    int m_cursor = 0;
};

inline FileSymbol::~FileSymbol()
{
    delete tokenStream;
    tokenStream = nullptr;
}

} // end of MI
} // end of KDevMI

 Q_DECLARE_TYPEINFO(KDevMI::MI::Token, Q_PRIMITIVE_TYPE);

#endif
