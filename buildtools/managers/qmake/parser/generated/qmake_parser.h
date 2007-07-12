// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_H_INCLUDED
#define qmake_H_INCLUDED

#include "qmake_ast.h"
#include <kdev-pg-memory-pool.h>
#include <kdev-pg-allocator.h>
#include <kdev-pg-token-stream.h>

namespace QMake
  {

  class parser
    {

    public:
      typedef kdev_pg_token_stream token_stream_type;
      typedef kdev_pg_token_stream::token_type token_type;
      kdev_pg_token_stream *token_stream;
      int yytoken;

      inline token_type LA(std::size_t k =  1) const
        {
          return  token_stream->token(token_stream->index() -  1 +  k -  1);
        }

      inline int yylex()
      {
        return  (yytoken =  token_stream->next_token());
      }

      inline void rewind(std::size_t index)
      {
        token_stream->rewind(index);
        yylex();
      }

      // token stream
      void set_token_stream(kdev_pg_token_stream *s)
      {
        token_stream =  s;
      }

      // error handling
      void yy_expected_symbol(int kind,  char const *name);
      void yy_expected_token(int kind,  std::size_t token,  char const *name);

      bool yy_block_errors;
      inline bool block_errors(bool block)
      {
        bool previous =  yy_block_errors;
        yy_block_errors =  block;
        return  previous;
      }

      // memory pool
      typedef kdev_pg_memory_pool memory_pool_type;

      kdev_pg_memory_pool *memory_pool;
      void set_memory_pool(kdev_pg_memory_pool *p)
      {
        memory_pool =  p;
      }

      template  <class T>
      inline T *create()
      {
        T *node =  new (memory_pool->allocate(sizeof(T))) T();
        node->kind =  T::KIND;
        return  node;
      }

      enum token_type_enum
      {
        Token_COLON =  1000,
        Token_COMMA =  1001,
        Token_CONT =  1002,
        Token_DOUBLEDOLLAR =  1003,
        Token_EOF =  1004,
        Token_EQUAL =  1005,
        Token_ERROR =  1006,
        Token_IDENTIFIER =  1007,
        Token_INVALID =  1008,
        Token_LBRACE =  1009,
        Token_LBRACKET =  1010,
        Token_LPAREN =  1011,
        Token_MINUSEQ =  1012,
        Token_NEWLINE =  1013,
        Token_PLUSEQ =  1014,
        Token_QUOTEDIDENTIFIER =  1015,
        Token_QUOTEDSPACE =  1016,
        Token_RBRACE =  1017,
        Token_RBRACKET =  1018,
        Token_RPAREN =  1019,
        Token_SINGLEDOLLAR =  1020,
        Token_STAREQ =  1021,
        Token_TILDEEQ =  1022,
        token_type_size
      }; // token_type_enum

      // user defined declarations:

    public:

      /**
       * Transform the raw input into tokens.
       * When this method returns, the parser's token stream has been filled
       * and any parse_*() method can be called.
       */
      void tokenize( const QString& contents );

      enum ProblemType {
        Error,
        Warning,
        Info
      };
      void reportProblem( parser::ProblemType type,  const QString& message );

      QString tokenText(std::size_t begin,  std::size_t end) const;


    private:

      QString m_contents;


    public:
      parser()
      {
        memory_pool =  0;
        token_stream =  0;
        yytoken =  Token_EOF;
        yy_block_errors =  false;
      }

      virtual ~parser()
      {}

      bool parse_project(project_ast **yynode);
    };

} // end of namespace QMake

#endif


