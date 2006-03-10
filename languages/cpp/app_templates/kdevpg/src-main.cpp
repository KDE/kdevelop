%{CPP_TEMPLATE}

#include "%{LANGUAGE}.h"
#include "decoder.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

char *_G_contents;
std::size_t _M_token_begin, _M_token_end;
std::size_t _G_current_offset;
extern char* yytext;

static void tokenize(%{LANGUAGE} &m);
static void usage(char const* argv0);
static bool parse_file(char const* filename);
int yylex();
void lexer_restart();


void print_token_environment(%{LANGUAGE}* parser)
{
    static bool done = false;
    if (done)
      return; // don't print with each call when going up the error path

    decoder dec(parser->token_stream);

    int current_index = parser->token_stream->index() - 1;
    for (int i = current_index - 5; i < current_index + 5; i++)
      {
        if (i < 0 || i >= parser->token_stream->size())
          continue;

        if (i == current_index)
          std::cerr << ">>";

        std::cerr << dec.decode_id(i); // print out currently processed token

        if (i == current_index)
          std::cerr << "<<";

        std::cerr << " ";
      }
    std::cerr << std::endl;

    done = true;
}

// custom error recovery
bool %{LANGUAGE}::yy_expected_token(int /*expected*/, std::size_t where, char const *name)
{
    print_token_environment(this);
    std::cerr << "** ERROR expected token ``" << name
              //<< "'' instead of ``" << current_token_text
              << "''" << std::endl;

    return false;
}

bool %{LANGUAGE}::yy_expected_symbol(int /*expected_symbol*/, char const *name)
{
    print_token_environment(this);
    std::cerr << "** ERROR expected symbol ``" << name
              << "''" << std::endl;
    return false;
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
      /*if (!strncmp(arg, "--option=", 9))
        {
          char const* option = arg + 9;

          std::cerr << "--option=" << option
                    << " has been given!" << std::endl;
        }
      else */
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
}

bool parse_file(char const *filename)
{
  std::ifstream filestr(filename);

  if (filestr.is_open())
    {
      std::filebuf *pbuf;
      long size;

      // get pointer to associated buffer object
      pbuf=filestr.rdbuf();

      // get file size using buffer's members
      size=pbuf->pubseekoff(0,std::ios::end,std::ios::in);
      pbuf->pubseekpos(0,std::ios::in);

      // allocate memory to contain file data
      _G_contents=new char[size];

      // get file data
      pbuf->sgetn(_G_contents, size);

      filestr.close();
    }
  else
    {
      std::cerr << filename << ": file not found" << std::endl;
      return false;
    }

  %{LANGUAGE}::token_stream_type token_stream;
  %{LANGUAGE}::memory_pool_type memory_pool;

  // 0) setup
  %{LANGUAGE} parser;
  parser.set_token_stream(&token_stream);
  parser.set_memory_pool(&memory_pool);

  // 1) tokenize
  tokenize(parser);

  // 2) parse
  %{STARTRULE}_ast *ast = 0;
  bool matched = parser.parse_%{STARTRULE}(&ast);
  if (matched)
    {
      %{LANGUAGE}_default_visitor v;
      v.visit_node(ast);
    }
  else
    {
      parser.yy_expected_symbol(%{LANGUAGE}_ast_node::Kind_%{STARTRULE}, "%{STARTRULE}");
    }

  delete[] _G_contents;

  return matched;
}

static void tokenize(%{LANGUAGE} &m)
{
  ::lexer_restart();
  int kind = %{LANGUAGE}::Token_EOF;
  do
    {
      kind = ::yylex();
      //std::cerr << yytext << std::endl; // debug output: recognized tokens
      if (!kind)
        kind = %{LANGUAGE}::Token_EOF;

      %{LANGUAGE}::token_type &t = m.token_stream->next();
      t.kind = kind;
      t.begin = _M_token_begin;
      t.end = _M_token_end;
      t.text = _G_contents;
    }
  while (kind != %{LANGUAGE}::Token_EOF);

  m.yylex(); // produce the look ahead token
}

static void usage(char const* argv0)
{
  std::cerr << "usage: " << argv0 << " [options] file1 [file2...]"
    << std::endl; /*<< std::endl
    << "Options:" << std::endl
    << "  --option=BLA: Do BLA while parsing." << std::endl
    << "                BLA is one of FOO, BAR or KUNG, default is FOO."
    << std::endl;
    */
}
