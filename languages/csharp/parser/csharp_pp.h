// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_pp_h_INCLUDED
#define csharp_pp_h_INCLUDED

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-list.h"
#include "kdev-pg-token-stream.h"
#include <cassert>

namespace csharp_pp
  {

  struct pp_and_expression_ast;
  struct pp_declaration_ast;
  struct pp_diagnostic_ast;
  struct pp_directive_ast;
  struct pp_elif_clause_ast;
  struct pp_else_clause_ast;
  struct pp_endif_clause_ast;
  struct pp_equality_expression_ast;
  struct pp_equality_expression_rest_ast;
  struct pp_expression_ast;
  struct pp_if_clause_ast;
  struct pp_line_ast;
  struct pp_pragma_ast;
  struct pp_primary_expression_ast;
  struct pp_region_ast;
  struct pp_unary_expression_ast;

  namespace pp_declaration
    {
    enum pp_declaration_type_enum {
      type_define,
      type_undef
    };
  }

  namespace pp_diagnostic
    {
    enum pp_diagnostic_type_enum {
      type_error,
      type_warning
    };
  }

  namespace pp_equality_expression_rest
    {
    enum pp_equality_operator_enum {
      op_equal,
      op_not_equal
    };
  }

  namespace pp_primary_expression
    {
    enum pp_primary_expression_type_enum {
      type_true,
      type_false,
      type_conditional_symbol,
      type_parenthesis_expression
    };
  }

  namespace pp_region
    {
    enum pp_region_type_enum {
      type_region,
      type_endregion
    };
  }


  struct ast_node
    {
      enum ast_node_kind_enum {
        Kind_pp_and_expression = 1000,
        Kind_pp_declaration = 1001,
        Kind_pp_diagnostic = 1002,
        Kind_pp_directive = 1003,
        Kind_pp_elif_clause = 1004,
        Kind_pp_else_clause = 1005,
        Kind_pp_endif_clause = 1006,
        Kind_pp_equality_expression = 1007,
        Kind_pp_equality_expression_rest = 1008,
        Kind_pp_expression = 1009,
        Kind_pp_if_clause = 1010,
        Kind_pp_line = 1011,
        Kind_pp_pragma = 1012,
        Kind_pp_primary_expression = 1013,
        Kind_pp_region = 1014,
        Kind_pp_unary_expression = 1015,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct pp_and_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_and_expression
      };

      const list_node<pp_equality_expression_ast *> *expression_sequence;
    };

  struct pp_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_declaration
      };

      pp_declaration::pp_declaration_type_enum type;
      std::size_t conditional_symbol;
    };

  struct pp_diagnostic_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_diagnostic
      };

      pp_diagnostic::pp_diagnostic_type_enum type;
      std::size_t message;
    };

  struct pp_directive_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_directive
      };

      pp_declaration_ast *declaration;
      pp_if_clause_ast *if_clause;
      pp_elif_clause_ast *elif_clause;
      pp_else_clause_ast *else_clause;
      pp_endif_clause_ast *endif_clause;
      pp_diagnostic_ast *diagnostic;
      pp_region_ast *region;
      pp_line_ast *line;
      pp_pragma_ast *pragma;
    };

  struct pp_elif_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_elif_clause
      };

      pp_expression_ast *expression;
    };

  struct pp_else_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_else_clause
      };

    };

  struct pp_endif_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_endif_clause
      };

    };

  struct pp_equality_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_equality_expression
      };

      pp_unary_expression_ast *expression;
      const list_node<pp_equality_expression_rest_ast *> *additional_expression_sequence;
    };

  struct pp_equality_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_equality_expression_rest
      };

      pp_equality_expression_rest::pp_equality_operator_enum equality_operator;
      pp_unary_expression_ast *expression;
    };

  struct pp_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_expression
      };

      const list_node<pp_and_expression_ast *> *expression_sequence;
    };

  struct pp_if_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_if_clause
      };

      pp_expression_ast *expression;
    };

  struct pp_line_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_line
      };

      std::size_t line_number;
      std::size_t file_name;
      std::size_t token_default;
      std::size_t identifier_or_keyword;
    };

  struct pp_pragma_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_pragma
      };

      std::size_t pragma_text;
    };

  struct pp_primary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_primary_expression
      };

      pp_primary_expression::pp_primary_expression_type_enum type;
      std::size_t conditional_symbol;
      pp_expression_ast *parenthesis_expression;
    };

  struct pp_region_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_region
      };

      pp_region::pp_region_type_enum type;
      std::size_t label;
    };

  struct pp_unary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pp_unary_expression
      };

      bool negated;
      pp_primary_expression_ast *expression;
    };



} // end of namespace csharp_pp


#include "csharp_pp_scope.h"

namespace csharp
  {
  class parser;
}

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

      // token stream
      void set_token_stream(kdev_pg_token_stream *s)
      {
        token_stream = s;
      }

      // error recovery
      bool yy_expected_symbol(int kind, char const *name);
      bool yy_expected_token(int kind, std::size_t token, char const *name);

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
       * @return  csharp_pp::parser::result_ok if the line was processed correctly,
       *          csharp_pp::parser::result_invalid if there was a parsing error,
       *          or csharp_pp::parser::result_eof if the end of file was found (unexpectedly).
       */
      parser::pp_parse_result pp_parse_line(
        parser::token_type_enum first_token, scope* scope );

    private:

      scope* _M_scope;

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
      void add_token( parser::token_type_enum token_kind );

      token_stream_type _M_token_stream;
      memory_pool_type _M_memory_pool;


    public:
      parser()
      {
        memory_pool = 0;
        token_stream = 0;
        yytoken = Token_EOF;

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
  class visitor
    {
      typedef void (visitor::*parser_fun_t)(ast_node *);
      static parser_fun_t _S_parser_table[];

    public:
      virtual ~visitor()
      {}
      virtual void visit_node(ast_node *node)
      {
        if (node)
          (this->*_S_parser_table[node->kind - 1000])(node);
      }
      virtual void visit_pp_and_expression(pp_and_expression_ast *)
    {}
      virtual void visit_pp_declaration(pp_declaration_ast *)
      {}
      virtual void visit_pp_diagnostic(pp_diagnostic_ast *)
      {}
      virtual void visit_pp_directive(pp_directive_ast *)
      {}
      virtual void visit_pp_elif_clause(pp_elif_clause_ast *)
      {}
      virtual void visit_pp_else_clause(pp_else_clause_ast *)
      {}
      virtual void visit_pp_endif_clause(pp_endif_clause_ast *)
      {}
      virtual void visit_pp_equality_expression(pp_equality_expression_ast *)
      {}
      virtual void visit_pp_equality_expression_rest(pp_equality_expression_rest_ast *)
      {}
      virtual void visit_pp_expression(pp_expression_ast *)
      {}
      virtual void visit_pp_if_clause(pp_if_clause_ast *)
      {}
      virtual void visit_pp_line(pp_line_ast *)
      {}
      virtual void visit_pp_pragma(pp_pragma_ast *)
      {}
      virtual void visit_pp_primary_expression(pp_primary_expression_ast *)
      {}
      virtual void visit_pp_region(pp_region_ast *)
      {}
      virtual void visit_pp_unary_expression(pp_unary_expression_ast *)
      {}
    }
  ;
  class default_visitor: public visitor
    {
    public:
      virtual void visit_pp_and_expression(pp_and_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<pp_equality_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_pp_declaration(pp_declaration_ast *)
    {}

      virtual void visit_pp_diagnostic(pp_diagnostic_ast *)
      {}

      virtual void visit_pp_directive(pp_directive_ast *node)
      {
        visit_node(node->declaration);
        visit_node(node->if_clause);
        visit_node(node->elif_clause);
        visit_node(node->else_clause);
        visit_node(node->endif_clause);
        visit_node(node->diagnostic);
        visit_node(node->region);
        visit_node(node->line);
        visit_node(node->pragma);
      }

      virtual void visit_pp_elif_clause(pp_elif_clause_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_pp_else_clause(pp_else_clause_ast *)
      {}

      virtual void visit_pp_endif_clause(pp_endif_clause_ast *)
      {}

      virtual void visit_pp_equality_expression(pp_equality_expression_ast *node)
      {
        visit_node(node->expression);
        if (node->additional_expression_sequence)
          {
            const list_node<pp_equality_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_pp_equality_expression_rest(pp_equality_expression_rest_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_pp_expression(pp_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<pp_and_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_pp_if_clause(pp_if_clause_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_pp_line(pp_line_ast *)
      {}

      virtual void visit_pp_pragma(pp_pragma_ast *)
      {}

      virtual void visit_pp_primary_expression(pp_primary_expression_ast *node)
      {
        visit_node(node->parenthesis_expression);
      }

      virtual void visit_pp_region(pp_region_ast *)
      {}

      virtual void visit_pp_unary_expression(pp_unary_expression_ast *node)
      {
        visit_node(node->expression);
      }

    };

} // end of namespace csharp_pp

#endif


