
#include "cool.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#define MAX_BUFF (10 * 1024)

char *_G_contents;
std::size_t _M_token_begin, _M_token_end;
std::size_t _G_current_offset;

static void tokenize(cool &m);
int yylex();

/*int main(int, char *argv[])
{
  if (!*++argv)
    {
      std::cerr << "usage: cool file.f" << std::endl;
      exit(EXIT_FAILURE);
    }

  if (FILE *fp = fopen(*argv, "r"))
    {
      fread(_G_contents = new char[MAX_BUFF], 1, MAX_BUFF, fp);
      fclose(fp);
    }
  else
    {
      std::cerr << "file not found" << std::endl;
      exit(EXIT_FAILURE);
    }

  cool::token_stream_type token_stream;
  cool::memory_pool_type memory_pool;

  // 0) setup
  cool parser;
  parser.set_token_stream(&token_stream);
  parser.set_memory_pool(&memory_pool);

  // 1) tokenize
  tokenize(parser);

  // 2) parse
  program_ast *ast = 0;
  if (parser.parse_program(&ast))
    {
      cool_default_visitor v;
      v.visit_node(ast);
    }
  else
    {
      std::cerr << "** ERROR expected a declaration: token position:" << _M_token_begin << std::endl;
    }

  delete[] _G_contents;

  return EXIT_SUCCESS;
}*/

static void tokenize(cool &m)
{
  // tokenize
  int kind = cool::Token_EOF;
  do
    {
      kind = ::yylex();
      if (!kind)
        kind = cool::Token_EOF;

      cool::token_type &t = m.token_stream->next();
      t.kind = kind;
      t.begin = _M_token_begin;
      t.end = _M_token_end;
      t.text = _G_contents;
    }
  while (kind != cool::Token_EOF);

  m.yylex(); // produce the look ahead token
}
