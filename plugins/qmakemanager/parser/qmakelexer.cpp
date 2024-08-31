/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakelexer.h"

#include <QString>
#include "qmakeparser.h"
#include <kdev-pg-location-table.h>
#include <kdev-pg-token-stream.h>

namespace QMake {

bool isIdentifierCharacter(QChar* c, bool canLookAhead)
{
    return (c->isLetter() || c->isDigit() || c->unicode() == '_' || c->unicode() == '.' || c->unicode() == '-'
            || c->unicode() == '$' || c->unicode() == '*'
            || (canLookAhead && (c + 1)->unicode() != '=' && (c->unicode() == '+')));
}

bool isBeginIdentifierCharacter(QChar* c)
{
    return (c->isLetter() || c->isDigit() || c->unicode() == '_' || c->unicode() == '.' || c->unicode() == '$');
}

bool isEndIdentifierCharacter(QChar* c)
{
    return (c->isLetter() || c->isDigit() || c->unicode() == '_' || c->unicode() == '.' || c->unicode() == '$'
            || c->unicode() == '*');
}

bool isCont(QChar* c)
{
    if (c->unicode() == '\\') {
        c++;
        while (c->isSpace() && c->unicode() != '\n') {
            c++;
        }
        if (c->unicode() == '\n' || c->unicode() == '#') {
            return true;
        }
    }
    return false;
}

Lexer::Lexer(Parser* _parser, QString content)
    : m_content(std::move(content))
    , m_parser(_parser)
    , m_curpos(0)
    , m_contentSize(m_content.size())
    , m_tokenBegin(0)
    , m_tokenEnd(0)
{
    pushState(ErrorState);
    pushState(DefaultState);
}

int Lexer::state() const
{
    return mState.top();
}

void Lexer::pushState(int state)
{
    mState.push(state);
}

void Lexer::popState()
{
    mState.pop();
}

int Lexer::nextTokenKind()
{
    int token = Parser::Token_INVALID;
    if (m_curpos >= m_contentSize) {
        return 0;
    }
    QChar* it = m_content.data();
    it += m_curpos;
    switch (state()) {
    case VariableValueState:
        it = ignoreWhitespaceAndComment(it);
        m_tokenBegin = m_curpos;
        if (m_curpos < m_contentSize) {
            if (it->unicode() == '}') {
                popState();
                token = Parser::Token_RBRACE;
            } else if (it->unicode() == '\n') {
                popState();
                createNewline(m_curpos);
                token = Parser::Token_NEWLINE;
            } else if (it->unicode() == '\\' && isCont(it)) {
                pushState(ContState);
                token = Parser::Token_CONT;
            } else if (it->unicode() == '"') {
                it++;
                m_curpos++;
                QChar* lastit = it;
                while ((it->unicode() != '"' || (lastit->unicode() == '\\' && it->unicode() == '"'))
                       && it->unicode() != '\n' && it->unicode() != '#' && !isCont(it) && m_curpos < m_contentSize) {
                    lastit = it;
                    it++;
                    m_curpos++;
                }
                if (it->unicode() != '"' && it->unicode() != '#') {
                    m_curpos--;
                }
                token = Parser::Token_VALUE;
                if (it->unicode() == '#') {
                    m_tokenEnd = m_curpos - 1;
                    do {
                        it++;
                        m_curpos++;
                    } while (it->unicode() != '\n' && m_curpos < m_contentSize);
                    if (it->unicode() == '\n') {
                        m_curpos--;
                    }
                    return token;
                }
            } else if (it->unicode() == '(') {
                unsigned int bracecount = 0;
                while ((it->unicode() != ';' || bracecount > 0) && it->unicode() != '\n' && !isCont(it)
                       && m_curpos < m_contentSize) {
                    if (it->unicode() == '(') {
                        bracecount++;
                    } else if (it->unicode() == ')' && bracecount > 0) {
                        bracecount--;
                    }
                    ++it;
                    ++m_curpos;
                }
                if (it->unicode() != ';') {
                    m_curpos--;
                }
                token = Parser::Token_VALUE;
            } else {
                while (!it->isSpace() && !isCont(it) && it->unicode() != '#' && m_curpos < m_contentSize) {
                    it++;
                    m_curpos++;
                }
                m_curpos--;
                token = Parser::Token_VALUE;
            }
        }
        break;
    case FunctionArgState:
        m_tokenBegin = m_curpos;
        if (it->unicode() == '\n') {
            createNewline(m_curpos);
            token = Parser::Token_NEWLINE;
        } else if (it->unicode() == '\\' && isCont(it)) {
            pushState(ContState);
            token = Parser::Token_CONT;
        } else if (it->unicode() == ',') {
            token = Parser::Token_COMMA;
        } else if (it->unicode() == ')') {
            popState();
            token = Parser::Token_RPAREN;
        } else {
            unsigned int parentCount = 0;
            while (parentCount > 0 || (it->unicode() != ')' && it->unicode() != ',' && m_curpos < m_contentSize)) {
                if (it->unicode() == ')') {
                    parentCount--;
                } else if (it->unicode() == '(') {
                    parentCount++;
                }
                ++it;
                ++m_curpos;
            }
            m_curpos--;
            token = Parser::Token_VALUE;
        }
        break;
    case ContState:
        it = ignoreWhitespaceAndComment(it);
        m_tokenBegin = m_curpos;
        if (m_curpos < m_contentSize) {
            if (it->unicode() == '\n') {
                createNewline(m_curpos);
                token = Parser::Token_NEWLINE;
                m_tokenEnd = m_curpos;
                popState();
                QChar* temp = it;
                int newpos = m_curpos;
                do {
                    temp++;
                    newpos++;
                    if (temp->unicode() == '#') {
                        while (temp->unicode() != '\n' && newpos < m_contentSize) {
                            temp++;
                            newpos++;
                        }
                        createNewline(m_curpos);
                        temp++;
                        m_curpos = newpos;
                        newpos++;
                    }
                } while (m_curpos < m_contentSize && temp->isSpace() && temp->unicode() != '\n');
                m_curpos++;
                return token;
            }
        }
        break;
    case DefaultState:
        it = ignoreWhitespaceAndComment(it);
        m_tokenBegin = m_curpos;
        if (m_curpos < m_contentSize) {
            if (isBeginIdentifierCharacter(it)) {
                token = Parser::Token_IDENTIFIER;
                while (!it->isSpace() && isIdentifierCharacter(it, m_curpos + 1 < m_contentSize)
                       && m_curpos < m_contentSize) {
                    it++;
                    m_curpos++;
                }
                if (!isEndIdentifierCharacter((it - 1))) {
                    token = Parser::Token_INVALID;
                } else if (QStringView{m_content}.sliced(m_tokenBegin, m_curpos - m_tokenBegin)
                           == QLatin1String("else")) {
                    token = Parser::Token_ELSE;
                }
                m_curpos--;
            } else {
                // Now the stuff that will generate a proper token
                QChar* c2 = m_curpos < m_contentSize ? it + 1 : nullptr;
                switch (it->unicode()) {
                case '|':
                    token = Parser::Token_OR;
                    break;
                case '!':
                    token = Parser::Token_EXCLAM;
                    break;
                case '(':
                    pushState(FunctionArgState);
                    token = Parser::Token_LPAREN;
                    break;
                case '{':
                    token = Parser::Token_LBRACE;
                    break;
                case '}':
                    token = Parser::Token_RBRACE;
                    break;
                case ':':
                    token = Parser::Token_COLON;
                    break;
                case '~':
                    if (c2 && c2->unicode() == '=') {
                        pushState(VariableValueState);
                        m_curpos++;
                        token = Parser::Token_TILDEEQ;
                    }
                    break;
                case '*':
                    if (c2 && c2->unicode() == '=') {
                        pushState(VariableValueState);
                        m_curpos++;
                        token = Parser::Token_STAREQ;
                    }
                    break;
                case '-':
                    if (c2 && c2->unicode() == '=') {
                        pushState(VariableValueState);
                        m_curpos++;
                        token = Parser::Token_MINUSEQ;
                    }
                    break;
                case '+':
                    if (c2 && c2->unicode() == '=') {
                        pushState(VariableValueState);
                        m_curpos++;
                        token = Parser::Token_PLUSEQ;
                    }
                    break;
                case '=':
                    pushState(VariableValueState);
                    token = Parser::Token_EQUAL;
                    break;
                case '\n':
                    createNewline(m_curpos);
                    token = Parser::Token_NEWLINE;
                    break;
                default:
                    break;
                }
            }
        }
        break;
    default:
        token = Parser::Token_INVALID;
        break;
    }
    if (m_curpos >= m_contentSize) {
        return 0;
    }
    m_tokenEnd = m_curpos;
    m_curpos++;
    return token;
}

qint64 Lexer::tokenBegin() const
{
    return m_tokenBegin;
}

qint64 Lexer::tokenEnd() const
{
    return m_tokenEnd;
}

QChar* Lexer::ignoreWhitespaceAndComment(QChar* it)
{
    // Ignore whitespace, but preserve the newline
    bool comment = false;
    while (m_curpos < m_contentSize && (it->isSpace() || comment || it->unicode() == '#') && it->unicode() != '\n') {
        if (it->unicode() == '#') {
            comment = true;
        }
        ++it;
        ++m_curpos;
    }
    return it;
}

void Lexer::createNewline(int pos)
{
    if (m_parser)
        m_parser->tokenStream->locationTable()->newline(pos);
}
}
