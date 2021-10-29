/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MIPARSER_H
#define MIPARSER_H

#include <memory>

#include "mi.h"
#include "milexer.h"

class QString;

namespace KDevMI { namespace MI {

/**
@author Roberto Raggi
*/
class MIParser
{
public:
    MIParser();
    ~MIParser();

    std::unique_ptr<Record> parse(FileSymbol *file);

protected: // rules
    std::unique_ptr<Record> parseResultOrAsyncRecord();
    std::unique_ptr<Record> parsePrompt();
    std::unique_ptr<Record> parseStreamRecord();

    bool parseResult(Result *&result);
    bool parseValue(Value *&value);
    bool parseTuple(Value *&value);
    bool parseList(Value *&value);

    /** Creates new TupleValue object, writes its address
        into *value, parses a comma-separated set of values,
        and adds each new value into (*value)->results.
        If 'start' and 'end' are not zero, they specify
        start and end delimiter of the list.
        Parsing stops when we see 'end' character, or, if
        'end' is zero, at the end of input.
    */
    bool parseCSV(TupleValue** value,
                  char start = 0, char end = 0);

    /** @overload
        Same as above, but writes into existing tuple.
    */
    bool parseCSV(TupleValue& value,
                  char start = 0, char end = 0);

    /** Parses a string literal and returns it. Advances
        the lexer past the literal. Processes C escape sequences
        in the string.
        @pre lex->lookAhead(0) == Token_string_literal
    */
    QString parseStringLiteral();

private:
    MILexer m_lexer;
    TokenStream *m_lex = nullptr;
};

} // end of namespace MI
} // end of namespace KDevMI

#endif
