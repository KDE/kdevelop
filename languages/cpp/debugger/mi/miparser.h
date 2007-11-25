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
#ifndef MIPARSER_H
#define MIPARSER_H

#include "milexer.h"
#include "gdbmi.h"

#include <qstring.h>
#include <qvaluelist.h>

/**
@author Roberto Raggi
*/
class MIParser
{
public:
    MIParser();
    ~MIParser();

    GDBMI::Record *parse(FileSymbol *file);

protected: // rules
    bool parseResultRecord(GDBMI::Record *&record);
    bool parsePrompt(GDBMI::Record *&record);
    bool parseStreamRecord(GDBMI::Record *&record);

    bool parseResult(GDBMI::Result *&result);
    bool parseValue(GDBMI::Value *&value);
    bool parseTuple(GDBMI::Value *&value);
    bool parseList(GDBMI::Value *&value);

    /** Creates new TupleValue object, writes its address
        into *value, parses a comma-separated set of values,
        and adds each new value into (*value)->results.
        If 'start' and 'end' are not zero, they specify
        start and end delimiter of the list.
        Parsing stops when we see 'end' character, or, if
        'end' is zero, at the end of input.
    */
    bool parseCSV(GDBMI::TupleValue** value,
                  char start = 0, char end = 0);

    /** @overload
        Same as above, but writes into existing tuple.
    */
    bool parseCSV(GDBMI::TupleValue& value,
                  char start = 0, char end = 0);


    /** Parses a string literal and returns it. Advances
        the lexer past the literal. Processes C escape sequences
        in the string.
        @pre lex->lookAhead(0) == Token_string_literal
    */
    QString parseStringLiteral();



private:
    MILexer lexer;
    TokenStream *lex;
};

#endif
