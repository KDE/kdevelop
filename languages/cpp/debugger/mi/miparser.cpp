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
#include "miparser.h"
#include "tokens.h"
#include <memory>

#define MATCH(tok) \
  do { \
      if (lex->lookAhead(0) != (tok)) \
          return false; \
  } while (0)

#define ADVANCE(tok) \
  do { \
      MATCH(tok); \
      lex->nextToken(); \
  } while (0)

MIParser::MIParser()
    : lex(0)
{
}

MIParser::~MIParser()
{
}

GDBMI::Record *MIParser::parse(FileSymbol *file)
{
    lex = 0;

    GDBMI::Record *record = 0;

    TokenStream *tokenStream = lexer.tokenize(file);
    if (!tokenStream)
        return false;

    lex = file->tokenStream = tokenStream;

    switch (lex->lookAhead()) {
        case '~':
        case '@':
        case '&':
            parseStreamRecord(record);
            break;
        case '(':
            parsePrompt(record);
            break;
        case '^':
            parseResultRecord(record);
            break;
        case '*':
            parseAsyncRecord(record);
            break;
        default:
            break;
    }

    return record;
}

bool MIParser::parsePrompt(GDBMI::Record *&record)
{
    ADVANCE('(');
    MATCH(Token_identifier);
    if (lex->currentTokenText() != "gdb")
        return false;
    lex->nextToken();
    ADVANCE(')');

    record = new GDBMI::PromptRecord;
    return true;
}

bool MIParser::parseStreamRecord(GDBMI::Record *&record)
{
    std::auto_ptr<GDBMI::StreamRecord> stream(new GDBMI::StreamRecord);

    switch (lex->lookAhead()) {
        case '~':
        case '@':
        case '&': {
            stream->reason = lex->lookAhead();
            lex->nextToken();
            MATCH(Token_string_literal);
            QString message = lex->currentTokenText();
            message = message.mid(1, message.length()-2);
            lex->nextToken();
            stream->message = message;
            record = stream.release();
        }
        return true;

        default:
            break;
    }

    return false;
}

bool MIParser::parseResultRecord(GDBMI::Record *&record)
{
    ADVANCE('^');
    MATCH(Token_identifier);
    QString reason = lex->currentTokenText();
    lex->nextToken();

    if (reason == "done") {
        return parseDoneResultRecord(record);
    } else if (reason == "error") {
        return parseErrorResultRecord(record);
    }

    return false;
}

bool MIParser::parseResultRecord(GDBMI::Record *&record)
{
    ADVANCE('^');
    MATCH(Token_identifier);
    QString reason = lex->currentTokenText();
    lex->nextToken();

    if (reason == "done") {
        return parseDoneResultRecord(record);
    } else if (reason == "error") {
        return parseErrorResultRecord(record);
    }

    return false;
}


bool MIParser::parseErrorResultRecord(GDBMI::Record *&record)
{
    std::auto_ptr<GDBMI::ResultRecord> res(new GDBMI::ResultRecord);
    res->reason = "error";

    if (lex->lookAhead() != ',') {
        record = res.release();
        return true;
    }

    lex->nextToken();

    while (lex->lookAhead()) {
        GDBMI::Result *result = 0;
        if (!parseResult(result))
            return false;

        res->results.append(result);

        if (lex->lookAhead() == ',')
            lex->nextToken();
    }

    record = res.release();
    return true;
}

bool MIParser::parseDoneResultRecord(GDBMI::Record *&record)
{
    std::auto_ptr<GDBMI::ResultRecord> res(new GDBMI::ResultRecord);
    res->reason = "done";

    if (lex->lookAhead() != ',') {
        record = res.release();
        return true;
    }

    lex->nextToken();

    while (lex->lookAhead()) {
        GDBMI::Result *result = 0;
        if (!parseResult(result))
            return false;

        res->results.append(result);

        if (lex->lookAhead() == ',')
            lex->nextToken();
    }

    record = res.release();
    return true;
}

bool MIParser::parseResult(GDBMI::Result *&result)
{
    MATCH(Token_identifier);
    QString variable = lex->currentTokenText();
    lex->nextToken();

    std::auto_ptr<GDBMI::Result> res(new GDBMI::Result);
    res->variable = variable;

    if (lex->lookAhead() != '=')
        return true;

    lex->nextToken();

    GDBMI::Value *value = 0;
    if (!parseValue(value))
        return false;

    res->value = value;
    result = res.release();

    return true;
}

bool MIParser::parseValue(GDBMI::Value *&value)
{
    value = 0;

    switch (lex->lookAhead()) {
        case Token_string_literal: {
            QString message = lex->currentTokenText();
            message = message.mid(1, message.length()-2);
            lex->nextToken();
            value = new GDBMI::StringLiteralValue(message);
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

bool MIParser::parseTuple(GDBMI::Value *&value)
{
    ADVANCE('{');

    std::auto_ptr<GDBMI::TupleValue> tuple(new GDBMI::TupleValue());

    int tok = lex->lookAhead();
    while (tok && tok != '}') {
        GDBMI::Result *result;
        if (!parseResult(result))
            return false;

        tuple->results.append(result);

        if (lex->lookAhead() == ',')
            lex->nextToken();

        tok = lex->lookAhead();
    }

    ADVANCE('}');

    value = tuple.release();

    return true;
}

bool MIParser::parseList(GDBMI::Value *&value)
{
    ADVANCE('[');

    std::auto_ptr<GDBMI::ListValue> lst(new GDBMI::ListValue);

    int tok = lex->lookAhead();
    while (tok && tok != ']') {
        GDBMI::Result *result = 0;
        GDBMI::Value *val = 0;

        if (tok == Token_identifier && !parseResult(result))
            return false;
        else if (!parseValue(val))
            return false;

        Q_ASSERT(result || val);

        if (!result) {
            result = new GDBMI::Result;
            result->value = val;
        }
        lst->results.append(result);

        if (lex->lookAhead() == ',')
            lex->nextToken();

        tok = lex->lookAhead();
    }
    ADVANCE(']');

    value = lst.release();

    return true;
}

