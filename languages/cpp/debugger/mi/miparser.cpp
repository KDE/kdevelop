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
#include <memory>

using namespace GDBMI;

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

Record *MIParser::parse(FileSymbol *file)
{
    lex = 0;

    Record *record = 0;

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
            // Same as result, only differs in start
            // marker.
            parseResultRecord(record);
            break;
        default:
            break;
    }

    return record;
}

bool MIParser::parsePrompt(Record *&record)
{
    ADVANCE('(');
    MATCH(Token_identifier);
    if (lex->currentTokenText() != "gdb")
        return false;
    lex->nextToken();
    ADVANCE(')');

    record = new PromptRecord;
    return true;
}

bool MIParser::parseStreamRecord(Record *&record)
{
    std::auto_ptr<StreamRecord> stream(new StreamRecord);

    switch (lex->lookAhead()) {
        case '~':
        case '@':
        case '&': {
            stream->reason = lex->lookAhead();
            lex->nextToken();
            MATCH(Token_string_literal);
            stream->message = parseStringLiteral();
            record = stream.release();
        }
        return true;

        default:
            break;
    }

    return false;
}

bool MIParser::parseResultRecord(Record *&record)
{
    if (lex->lookAhead() != '^' && lex->lookAhead() != '*')
        return false;
    lex->nextToken();

    MATCH(Token_identifier);
    QString reason = lex->currentTokenText();
    lex->nextToken();

    std::auto_ptr<ResultRecord> res(new ResultRecord);
    res->reason = reason;

    if (lex->lookAhead() != ',') {
        record = res.release();
        return true;
    }

    lex->nextToken();
    
    if (!parseCSV(*res))
        return false;

    record = res.release();
    return true;
}

bool MIParser::parseResult(Result *&result)
{
    MATCH(Token_identifier);
    QString variable = lex->currentTokenText();
    lex->nextToken();

    std::auto_ptr<Result> res(new Result);
    res->variable = variable;

    if (lex->lookAhead() != '=')
        return true;

    lex->nextToken();

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

    switch (lex->lookAhead()) {
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

    std::auto_ptr<ListValue> lst(new ListValue);

    // Note: can't use parseCSV here because of nested
    // "is this Value or Result" guessing. Too lazy to factor
    // that out too using function pointers.
    int tok = lex->lookAhead();
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

        if (lex->lookAhead() == ',')
            lex->nextToken();

        tok = lex->lookAhead();
    }
    ADVANCE(']');

    value = lst.release();

    return true;
}

bool MIParser::parseCSV(TupleValue** value,
                        char start, char end)
{
    std::auto_ptr<TupleValue> tuple(new TupleValue);

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

    int tok = lex->lookAhead();
    while (tok) {
        if (end && tok == end)
            break;

        Result *result;
        if (!parseResult(result))
            return false;

        value.results.append(result);
        value.results_by_name.insert(result->variable, result);      

        if (lex->lookAhead() == ',')
            lex->nextToken();

        tok = lex->lookAhead();
    }

    if (end)
        ADVANCE(end);

    return true;
}

                        
QString MIParser::parseStringLiteral()
{
    QCString message = lex->currentTokenText();

    int length = message.length();
    QString message2;
    message2.setLength(length);
    // The [1,length-1] range removes quotes without extra
    // call to 'mid'
    unsigned target_index = 0;
    for(unsigned i = 1, e = length-1; i != e; ++i)
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
            message2[target_index++] = (char)translated;
            ++i;
        }
        else
        {
            message2[target_index++] = message[i];
        }        
    }
    message2.setLength(target_index);

    lex->nextToken();
    return message2;
}

