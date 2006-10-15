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

#include "ruby_parser.h"
#include "ruby_default_visitor.h"
#include "ruby_debug_visitor.h"
#include "decoder.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace ruby;

char *_G_contents;

static void usage(char const* argv0);
static bool parse_file(char const* filename);


void print_token_environment(ruby::parser* parser)
{
    static bool done = false;
    if (done)
      return; // don't print with each call when going up the error path

    decoder dec(parser->token_stream);

    std::size_t current_index = parser->token_stream->index() - 1;
    for (std::size_t i = current_index - 5; i < current_index + 5; i++)
    {
        if (i < 0 || i >= parser->token_stream->size())
          continue;

        if (i == current_index)
          std::cerr << ">>";

        std::cerr << dec.decode_string(i); // print out currently processed token

        if (i == current_index)
          std::cerr << "<<";

        std::cerr << " ";
    }
    std::cerr << std::endl;

    done = true;
}


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    while (char const *arg = *++argv)
    {
        if (!strncmp(arg, "--", 2))
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        else if(!parse_file(arg))
        {
            exit(EXIT_FAILURE);
        }
    }

  return EXIT_SUCCESS;
}

bool parse_file(char const *filename)
{
    char *contents;
    std::ifstream filestr(filename);

    if (filestr.is_open())
    {
        std::filebuf *pbuf;
        long size;

        // get pointer to associated buffer object
        pbuf = filestr.rdbuf();

        // get file size using buffer's members
        size = pbuf->pubseekoff(0,std::ios::end,std::ios::in);
        pbuf->pubseekpos(0,std::ios::in);

        // allocate memory to contain file data
        contents = new char[size+1];

        // get file data
        pbuf->sgetn(contents, size);

        contents[size] = '\0';

        filestr.close();
    }
    else
    {
        std::cerr << filename << ": file not found" << std::endl;
        return false;
    }

    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

    // 0) setup
    parser ruby_parser;
    ruby_parser.set_token_stream(&token_stream);
    ruby_parser.set_memory_pool(&memory_pool);

    // 1) tokenize
    ruby_parser.tokenize(contents);

    // 2) parse
    program_ast *ast = 0;
    bool matched = ruby_parser.parse_program(&ast);
    if (matched)
    {
        debug_visitor v;
        std::cout << std::endl << std::endl << "====== AST ======" << std::endl;
        v.visit_node(ast);
    }
    else
    {
        ruby_parser.yy_expected_symbol(ast_node::Kind_program, "program"); // ### remove me
    }

    delete[] contents;

    return matched;
}

static void usage(char const* argv0)
{
    std::cerr << "usage: " << argv0 << " [options] file.rb [file2.rb ...]"
        << std::endl << std::endl
        << "Options:" << std::endl
        << "  none so far" << std::endl;
}
