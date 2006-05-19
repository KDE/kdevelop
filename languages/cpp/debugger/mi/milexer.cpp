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

#include "milexer.h"
#include "tokens.h"
#include <cctype>
#include <iostream>

bool MILexer::s_initialized = false;
scan_fun_ptr MILexer::s_scan_table[];


MILexer::MILexer()
{
    if (!s_initialized)
        setupScanTable();
}

MILexer::~MILexer()
{
}

void MILexer::setupScanTable()
{
    s_initialized = true;

    for (int i=0; i<128; ++i) {
        switch (i) {
        case '\n':
            s_scan_table[i] = &MILexer::scanNewline;
            break;

        case '"':
            s_scan_table[i] = &MILexer::scanStringLiteral;
            break;

        default:
            if (isspace(i))
                s_scan_table[i] = &MILexer::scanWhiteSpaces;
            else if (isalpha(i) || i == '_')
                s_scan_table[i] = &MILexer::scanIdentifier;
            else if (isdigit(i))
                s_scan_table[i] = &MILexer::scanNumberLiteral;
            else
                s_scan_table[i] = &MILexer::scanChar;
        }
    }

    s_scan_table[128] = &MILexer::scanUnicodeChar;
}

/*

    m_firstToken = m_tokens.data();
    m_currentToken = 0;

    m_firstToken = m_tokens.data();
    m_currentToken = m_firstToken;
 */

TokenStream *MILexer::tokenize(const FileSymbol *fileSymbol)
{
    m_tokensCount = 0;
    m_tokens.resize(64);

    m_contents = fileSymbol->contents;
    m_length = m_contents.length();
    m_ptr = 0;

    m_lines.resize(8);
    m_line = 0;

    m_lines[m_line++] = 0;

    m_cursor = 0;

    // tokenize
    int pos, len;

    for (;;) {
        if (m_tokensCount == (int)m_tokens.size())
            m_tokens.resize(m_tokensCount * 2);

        Token &tk = m_tokens[m_tokensCount++];
        tk.kind = nextToken(pos, len);
        tk.position = pos;
        tk.length = len;

        if (tk.kind == 0)
            break;
    }

    TokenStream *tokenStream = new TokenStream;
    tokenStream->m_contents = m_contents;

    tokenStream->m_lines = m_lines;
    tokenStream->m_line = m_line;

    tokenStream->m_tokens = m_tokens;
    tokenStream->m_tokensCount = m_tokensCount;

    tokenStream->m_firstToken = tokenStream->m_tokens.data();
    tokenStream->m_currentToken = tokenStream->m_firstToken;;

    tokenStream->m_cursor = m_cursor;

    return tokenStream;
}

int MILexer::nextToken(int &pos, int &len)
{
    int start = 0;
    int kind = 0;
    unsigned char ch = 0;

    while (m_ptr < m_length) {
        start = m_ptr;

        ch = (unsigned char)m_contents[m_ptr];
        (this->*s_scan_table[ch < 128 ? ch : 128])(&kind);

        switch (kind) {
            case Token_whitespaces:
            case '\n':
                break;

            default:
                pos = start;
                len = m_ptr - start;
                return kind;
        }

        if (kind == 0)
            break;
    }

    return 0;
}

void MILexer::scanChar(int *kind)
{
    *kind = m_contents[m_ptr++];
}

void MILexer::scanWhiteSpaces(int *kind)
{
    *kind = Token_whitespaces;

    char ch;
    while (m_ptr < m_length) {
        ch = m_contents[m_ptr];
        if (!(isspace(ch) && ch != '\n'))
            break;

        ++m_ptr;
    }
}

void MILexer::scanNewline(int *kind)
{
    if (m_line == (int)m_lines.size())
        m_lines.resize(m_lines.size() * 2);

    if (m_lines.at(m_line) < m_ptr)
        m_lines[m_line++] = m_ptr;

    *kind = m_contents[m_ptr++];
}

void MILexer::scanUnicodeChar(int *kind)
{
    *kind = m_contents[m_ptr++];
}

void MILexer::scanStringLiteral(int *kind)
{
    ++m_ptr;
    while (char c = m_contents[m_ptr]) {
        switch (c) {
        case '\n':
            // ### error
            *kind = Token_string_literal;
            return;
        case '\\':
            {
                char next = m_contents.at(m_ptr+1);
                if (next == '"' || next == '\\')
                    m_ptr += 2;
                else
                    ++m_ptr;
            }
            break;
        case '"':
            ++m_ptr;
            *kind = Token_string_literal;
            return;
        default:
            ++m_ptr;
            break;
        }
    }

    // ### error
    *kind = Token_string_literal;
}

void MILexer::scanIdentifier(int *kind)
{
    char ch;
    while (m_ptr < m_length) {
        ch = m_contents[m_ptr];
        if (!(isalnum(ch) || ch == '-' || ch == '_'))
            break;

        ++m_ptr;
    }

    *kind = Token_identifier;
}

void MILexer::scanNumberLiteral(int *kind)
{
    char ch;
    while (m_ptr < m_length) {
        ch = m_contents[m_ptr];
        if (!(isalnum(ch) || ch == '.'))
            break;

        ++m_ptr;
    }

    // ### finish to implement me!!
    *kind = Token_number_literal;
}

void TokenStream::positionAt(int position, int *line, int *column) const
{
    if (!(line && column && !m_lines.isEmpty()))
        return;

    int first = 0;
    int len = m_line;
    int half;
    int middle;

    while (len > 0) {
        half = len >> 1;
        middle = first;

        middle += half;

        if (m_lines[middle] < position) {
            first = middle;
            ++first;
            len = len - half - 1;
        }
        else
            len = half;
    }

    *line = QMAX(first - 1, 0);
    *column = position - m_lines.at(*line);

    Q_ASSERT( *column >= 0 );
}

QCString TokenStream::tokenText(int index) const
{
    Token *t = index < 0 ? m_currentToken : m_firstToken + index;
    const char* data = m_contents;
    return QCString(data + t->position, t->length+1);
}

