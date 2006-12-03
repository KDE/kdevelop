// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_pp_H_INCLUDED
#define csharp_pp_H_INCLUDED

#include "csharp_pp_ast.h"
#include <kdev-pg-memory-pool.h>
#include <kdev-pg-allocator.h>
#include <kdev-pg-token-stream.h>

namespace csharp_pp
  {

  class parser
    {
    public:
      typedef kdev_pg_token_stream token_stream_type;
      typedef kdev_pg_token_stream::token_type token_type;
      kdev_pg_token_stream *token_stream;
      int yytoken;

      inline token_type LA(std::size_t k = 1) const
        {
          return token_stream->token(token_stream->index() - 1 + k - 1);
        }
      inline int yylex()
      {
        return (yytoken = token_stream->next_token());
      }
      inline void rewind(std::size_t index)
      {
        token_stream->rewind(index);
        yylex();
      }

      // token stream
      void set_token_stream(kdev_pg_token_stream *s)
      {
        token_stream = s;
      }

      // error handling
      void yy_expected_symbol(int kind, char const *name);
      void yy_expected_token(int kind, std::size_t token, char const *name);

      bool yy_block_errors;
      inline bool block_errors(bool block)
      {
        bool previous = yy_block_errors;
        yy_block_errors = block;
        return previous;
      }

      // memory pool
      typedef kdev_pg_memory_pool memory_pool_type;

      kdev_pg_memory_pool *memory_pool;
      void set_memory_pool(kdev_pg_memory_pool *p)
      {
        memory_pool = p;
      }
      template <class T>
      inline T *create()
      {
        T *node = new (memory_pool->allocate(sizeof(T))) T();
        node->kind = T::KIND;
        return node;
      }

      enum token_type_enum
      {
        Token_EOF = 1000,
        Token_PP_BANG = 1001,
        Token_PP_CONDITIONAL_SYMBOL = 1002,
        Token_PP_DEFAULT = 1003,
        Token_PP_DEFINE = 1004,
        Token_PP_ELIF = 1005,
        Token_PP_ELSE = 1006,
        Token_PP_ENDIF = 1007,
        Token_PP_ENDREGION = 1008,
        Token_PP_EQUAL = 1009,
        Token_PP_ERROR = 1010,
        Token_PP_FALSE = 1011,
        Token_PP_FILE_NAME = 1012,
        Token_PP_IDENTIFIER_OR_KEYWORD = 1013,
        Token_PP_IF = 1014,
        Token_PP_INVALID = 1015,
        Token_PP_LINE = 1016,
        Token_PP_LINE_NUMBER = 1017,
        Token_PP_LOG_AND = 1018,
        Token_PP_LOG_OR = 1019,
        Token_PP_LPAREN = 1020,
        Token_PP_MESSAGE = 1021,
        Token_PP_NEW_LINE = 1022,
        Token_PP_NOT_EQUAL = 1023,
        Token_PP_PRAGMA = 1024,
        Token_PP_PRAGMA_TEXT = 1025,
        Token_PP_REGION = 1026,
        Token_PP_RPAREN = 1027,
        Token_PP_TRUE = 1028,
        Token_PP_UNDEF = 1029,
        Token_PP_WARNING = 1030,
        token_type_size
      }; // token_type_enum

      // user defined declarations:
    public:

      enum pp_parse_result {
        result_ok,
        result_invalid,
        result_eof
      };

      /**
       * Process a pre-processor line.
       * This method occupies the lexer until a PP_NEW_LINE token is found,
       * and carries out the appropriate action. For example, when processing
       * a #define statement it stores the defined symbol, or when processing
       * an #if statement it evaluates the expression and sets the correct value
       * for the ppIsSectionSkipped() method.
       *
       * @param first_token  The first token of the pre-processor line.
       * @param scope  The currently active pre-processor state, stored as a scope.
       * @param lexer  The lexer object which is currently processing the source file.
       * @return  csharp_pp::parser::result_ok if the line was processed correctly,
       *          csharp_pp::parser::result_invalid if there was a parsing error,
       *          or csharp_pp::parser::result_eof if the end of file was found (unexpectedly).
       */
      parser::pp_parse_result pp_parse_line(
        parser::token_type_enum first_token, scope *scope, csharp::Lexer *lexer );

    private:

      scope *_M_scope;
      csharp::Lexer *_M_lexer;

      /**
       * Transform the raw input into tokens.
       * When this method returns, the parser's token stream has been filled
       * and any parse_*() method can be called. Alternatively, the end of file
       * has been reached and we have to issue an error.
       */
      void tokenize(bool &encountered_eof);

      /**
       * Add the currently lexed token into the token stream, using the
       * given token kind. Used by the pre-processor that has to bypass
       * the normal tokenizing process.
       */
      void add_token(parser::token_type_enum token_kind);

      token_stream_type _M_token_stream;
      memory_pool_type _M_memory_pool;


    public:
      parser()
      {
        memory_pool = 0;
        token_stream = 0;
        yytoken = Token_EOF;
        yy_block_errors = false;

        // user defined constructor code:

        set_token_stream(&_M_token_stream);
        set_memory_pool(&_M_memory_pool);

      }

      virtual ~parser()
      {}

      bool parse_pp_and_expression(pp_and_expression_ast **yynode);
      bool parse_pp_declaration(pp_declaration_ast **yynode);
      bool parse_pp_diagnostic(pp_diagnostic_ast **yynode);
      bool parse_pp_directive(pp_directive_ast **yynode);
      bool parse_pp_elif_clause(pp_elif_clause_ast **yynode);
      bool parse_pp_else_clause(pp_else_clause_ast **yynode);
      bool parse_pp_endif_clause(pp_endif_clause_ast **yynode);
      bool parse_pp_equality_expression(pp_equality_expression_ast **yynode);
      bool parse_pp_equality_expression_rest(pp_equality_expression_rest_ast **yynode);
      bool parse_pp_expression(pp_expression_ast **yynode);
      bool parse_pp_if_clause(pp_if_clause_ast **yynode);
      bool parse_pp_line(pp_line_ast **yynode);
      bool parse_pp_pragma(pp_pragma_ast **yynode);
      bool parse_pp_primary_expression(pp_primary_expression_ast **yynode);
      bool parse_pp_region(pp_region_ast **yynode);
      bool parse_pp_unary_expression(pp_unary_expression_ast **yynode);
    };

} // end of namespace csharp_pp

#endif


