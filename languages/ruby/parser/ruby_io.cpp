/*****************************************************************************
 * Copyright (c) 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/

// This file is meant to be specific to the framework in which the parser
// operates, and is likely to be adapted for different environments.
// For example, the text source in KDevelop might be different to the one in
// a command line parser, and error output might not always go to std::cerr.

#include "ruby_parser.h"
#include "ruby_lexer.h"

#include <iostream>
#include <sstream>

namespace ruby
{

void parser::report_problem( parser::problem_type type, std::string message )
{
    report_problem( type, message.c_str() );
}

void parser::report_problem( parser::problem_type type, const char* message )
{
    if (type == error)
        std::cerr << "** ERROR: " << message << std::endl;
    else if (type == warning)
        std::cerr << "** WARNING: " << message << std::endl;
    else if (type == info)
        std::cerr << "** Info: " << message << std::endl;
}


// custom error recovery
void parser::yy_expected_token(int /*expected*/, std::size_t /*where*/, char const *name)
{
    report_problem(
        parser::error,
        std::string("Expected token ``") + name
        //+ "'' instead of ``" + current_token_text
        + "''"
    );
}

void parser::yy_expected_symbol(int /*expected_symbol*/, char const *name)
{
    int line;
    int col;
    size_t index = token_stream->index();
    token_type &token = token_stream->token(index);
    token_stream->start_position(index, &line, &col);
    size_t tokenLength = token.end - token.begin;
    char tokenValue[tokenLength+1];
    strncpy(tokenValue, token.text + token.begin, tokenLength);
    std::stringstream s;
    s << " (current token: \"" << tokenValue <<
        "\" [" << token.kind << "] at line: " << line+1 << " col: " << col+1 << ")";
    report_problem(
        parser::error,
        std::string("Expected symbol ``") + name
        //+ "'' instead of ``" + current_token_text
        + "''" + s.str()
    );
}

} // end of namespace ruby

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
