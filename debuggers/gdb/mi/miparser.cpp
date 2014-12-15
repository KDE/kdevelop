/***************************************************************************
 *   Copyright (C) 2004 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2005-2006 by Vladimir Prus                              *
 *   ghost@cs.msu.su                                                       *
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
#include "miparser.h"
#include "tokens.h"

using namespace GDBMI;

#define MATCH(tok) \
  do { \
      if (m_lex->lookAhead(0) != (tok)) \
          return false; \
  } while (0)

#define ADVANCE(tok) \
  do { \
      MATCH(tok); \
      m_lex->nextToken(); \
  } while (0)

#define MATCH_PTR(tok) \
  do { \
      if (m_lex->lookAhead(0) != (tok)) \
          return {}; \
  } while (0)

#define ADVANCE_PTR(tok) \
  do { \
      MATCH_PTR(tok); \
      m_lex->nextToken(); \
  } while (0)

MIParser::MIParser()
    : m_lex(0)
{
}

MIParser::~MIParser()
{
}

std::unique_ptr<Record> MIParser::parse(FileSymbol *file)
{
    m_lex = 0;

    TokenStream *tokenStream = m_lexer.tokenize(file);
    if (!tokenStream)
        return 0;

    m_lex = file->tokenStream = tokenStream;

    uint32_t token = 0;
    if (m_lex->lookAhead() == Token_number_literal) {
        token = QString::fromUtf8(m_lex->currentTokenText()).toUInt();
        m_lex->nextToken();
    }

    std::unique_ptr<Record> record;

    switch (m_lex->lookAhead()) {
        case '~':
        case '@':
        case '&':
            record = parseStreamRecord();
            break;
        case '(':
            record = parsePrompt();
            break;
        case '^':
        case '*':
        case '=':
        case '+':
            record = parseResultOrAsyncRecord();
            break;
        default:
            break;
    }

    if (record && record->kind == Record::Result) {
        ResultRecord * result = static_cast<ResultRecord *>(record.get());
        result->token = token;
    } else {
        Q_ASSERT(token == 0);
    }

    return record;
}

std::unique_ptr<Record> MIParser::parsePrompt()
{
    ADVANCE_PTR('(');
    MATCH_PTR(Token_identifier);
    if (m_lex->currentTokenText() != "gdb")
        return {};
    m_lex->nextToken();
    ADVANCE_PTR(')');

    return std::unique_ptr<Record>(new PromptRecord);
}

std::unique_ptr<Record> MIParser::parseStreamRecord()
{
    StreamRecord::Subkind subkind;

    switch (m_lex->lookAhead()) {
    case '~': subkind = StreamRecord::Console; break;
    case '@': subkind = StreamRecord::Target; break;
    case '&': subkind = StreamRecord::Log; break;
    default:
        Q_ASSERT(false);
        return {};
    }

    std::unique_ptr<StreamRecord> stream(new StreamRecord(subkind));

    m_lex->nextToken();
    MATCH_PTR(Token_string_literal);
    stream->message = parseStringLiteral();
    return std::move(stream);
}

std::unique_ptr<Record> MIParser::parseResultOrAsyncRecord()
{
    std::unique_ptr<TupleRecord> result;

    char c = m_lex->lookAhead();
    m_lex->nextToken();
    MATCH_PTR(Token_identifier);
    QString reason = m_lex->currentTokenText();
    m_lex->nextToken();

    if (c == '^') {
        result.reset(new ResultRecord(reason));
    } else {
        AsyncRecord::Subkind subkind;
        switch (c) {
        case '*': subkind = AsyncRecord::Exec; break;
        case '=': subkind = AsyncRecord::Notify; break;
        case '+': subkind = AsyncRecord::Status; break;
        default:
            Q_ASSERT(false);
            return {};
        }
        result.reset(new AsyncRecord(subkind, reason));
    }

    if (m_lex->lookAhead() == ',') {
        m_lex->nextToken();

        if (!parseCSV(*result))
            return {};
    }

    return std::move(result);
}

bool MIParser::parseResult(Result *&result)
{
    // be less strict about the format, see e.g.:
    // https://bugs.kde.org/show_bug.cgi?id=304730
    // http://sourceware.org/bugzilla/show_bug.cgi?id=9659

    std::unique_ptr<Result> res(new Result);

    if (m_lex->lookAhead() == Token_identifier) {
        res->variable = m_lex->currentTokenText();
        m_lex->nextToken();

        if (m_lex->lookAhead() != '=') {
            result = res.release();
            return true;
        }

        m_lex->nextToken();
    }

    Value *value = 0;
    if (!parseValue(value))
        return false;

    res->value = value;
    result = res.release();

    return true;
}

bool MIParser::parseValue(Value *&value)
{
    value = 0;

    switch (m_lex->lookAhead()) {
        case Token_string_literal: {
            value = new StringLiteralValue(parseStringLiteral());
        }
        return true;

        case '{':
            return parseTuple(value);

        case '[':
            return parseList(value);

        default:
            break;
    }

    return false;
}

bool MIParser::parseTuple(Value *&value)
{
    TupleValue* val;
    
    if (!parseCSV(&val, '{', '}'))
        return false;

    value = val;
    return true;
}

bool MIParser::parseList(Value *&value)
{
    ADVANCE('[');

    std::unique_ptr<ListValue> lst(new ListValue);

    // Note: can't use parseCSV here because of nested
    // "is this Value or Result" guessing. Too lazy to factor
    // that out too using function pointers.
    int tok = m_lex->lookAhead();
    while (tok && tok != ']') {
        Result *result = 0;
        Value *val = 0;

        if (tok == Token_identifier)
        {
            if (!parseResult(result))
                return false;
        }
        else if (!parseValue(val))
            return false;

        Q_ASSERT(result || val);

        if (!result) {
            result = new Result;
            result->value = val;
        }
        lst->results.append(result);

        if (m_lex->lookAhead() == ',')
            m_lex->nextToken();

        tok = m_lex->lookAhead();
    }
    ADVANCE(']');

    value = lst.release();

    return true;
}

bool MIParser::parseCSV(TupleValue** value,
                        char start, char end)
{
    std::unique_ptr<TupleValue> tuple(new TupleValue);

    if (!parseCSV(*tuple, start, end))
        return false;
 
    *value = tuple.get();
    tuple.release();
    return true;
}

bool MIParser::parseCSV(GDBMI::TupleValue& value,
                        char start, char end)
{
   if (start)
        ADVANCE(start);

    int tok = m_lex->lookAhead();
    while (tok) {
        if (end && tok == end)
            break;

        Result *result;
        if (!parseResult(result))
            return false;

        value.results.append(result);
        value.results_by_name.insert(result->variable, result);      

        if (m_lex->lookAhead() == ',')
            m_lex->nextToken();

        tok = m_lex->lookAhead();
    }

    if (end)
        ADVANCE(end);

    return true;
}

                        
QString MIParser::parseStringLiteral()
{
    QByteArray messageByteArray = m_lex->currentTokenText();
    QString message = QString::fromUtf8(messageByteArray.constData());

    int length = message.length();
    QString message2;
    message2.reserve(length);
    // The [1,length-1] range removes quotes without extra
    // call to 'mid'
    int target_index = 0;
    for(int i = 1, e = length-1; i != e; ++i)
    {
        int translated = -1;
        if (message[i] == '\\')
        {
            if (i+1 < length)
            {
                // TODO: implement all the other escapes, maybe
                if (message[i+1] == 'n')
                {
                    translated = '\n';
                }
                else if (message[i+1] == '\\')
                {
                    translated = '\\';
                }
                else if (message[i+1] == '"')
                {
                    translated = '"';
                }
                else if (message[i+1] == 't')
                {
                    translated = '\t';
                }

            }
        }

        if (translated != -1)
        {
            message2[target_index++] = translated;
            ++i;
        }
        else
        {
            message2[target_index++] = message[i];
        }        
    }

    m_lex->nextToken();
    return message2;
}

