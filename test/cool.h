// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef cool_h_INCLUDED
#define cool_h_INCLUDED

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-list.h"
#include "kdev-pg-token-stream.h"
#include <cassert>

struct additive_expression_ast;
struct block_expression_ast;
struct case_condition_ast;
struct case_expression_ast;
struct class_ast;
struct expression_ast;
struct feature_ast;
struct formal_ast;
struct if_expression_ast;
struct let_declaration_ast;
struct let_expression_ast;
struct multiplicative_expression_ast;
struct postfix_expression_ast;
struct primary_expression_ast;
struct program_ast;
struct relational_expression_ast;
struct unary_expression_ast;
struct while_expression_ast;

struct cool_ast_node
  {
    enum ast_node_kind_enum {
      Kind_additive_expression = 1000,
      Kind_block_expression = 1001,
      Kind_case_condition = 1002,
      Kind_case_expression = 1003,
      Kind_class = 1004,
      Kind_expression = 1005,
      Kind_feature = 1006,
      Kind_formal = 1007,
      Kind_if_expression = 1008,
      Kind_let_declaration = 1009,
      Kind_let_expression = 1010,
      Kind_multiplicative_expression = 1011,
      Kind_postfix_expression = 1012,
      Kind_primary_expression = 1013,
      Kind_program = 1014,
      Kind_relational_expression = 1015,
      Kind_unary_expression = 1016,
      Kind_while_expression = 1017,
      AST_NODE_KIND_COUNT
    };

    int kind;
    std::size_t start_token;
    std::size_t end_token;
  };

struct additive_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_additive_expression
    };

    const list_node<multiplicative_expression_ast *> *expression_sequence;
    std::size_t op;

  };

struct block_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_block_expression
    };

    const list_node<expression_ast *> *expression_sequence;

  };

struct case_condition_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_case_condition
    };

    std::size_t name;
    std::size_t type;
    expression_ast *expression;

  };

struct case_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_case_expression
    };

    expression_ast *expression;
    const list_node<case_condition_ast *> *condition_sequence;

  };

struct class_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_class
    };

    std::size_t type;
    std::size_t base_type;
    const list_node<feature_ast *> *feature_sequence;

  };

struct expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_expression
    };

    relational_expression_ast *expression;

  };

struct feature_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_feature
    };

    std::size_t name;
    const list_node<formal_ast *> *formal_sequence;
    std::size_t type;
    expression_ast *expression;

  };

struct formal_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_formal
    };

    std::size_t name;
    std::size_t type;

  };

struct if_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_if_expression
    };

    expression_ast *condition;
    expression_ast *true_expression;
    expression_ast *false_expression;

  };

struct let_declaration_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_let_declaration
    };

    std::size_t name;
    std::size_t type;
    expression_ast *expression;

  };

struct let_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_let_expression
    };

    const list_node<let_declaration_ast *> *declaration_sequence;
    expression_ast *body_expression;

  };

struct multiplicative_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_multiplicative_expression
    };

    const list_node<postfix_expression_ast *> *expression_sequence;
    std::size_t op;

  };

struct postfix_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_postfix_expression
    };

    unary_expression_ast *base_expression;
    std::size_t at_type;
    std::size_t name;
    const list_node<expression_ast *> *arguments_sequence;

  };

struct primary_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_primary_expression
    };

    std::size_t name;
    expression_ast *expression;
    const list_node<expression_ast *> *argument_sequence;
    std::size_t variable;
    std::size_t integer_literal;
    std::size_t string_literal;
    std::size_t true_literal;
    std::size_t false_literal;
    std::size_t new_type;
    if_expression_ast *if_expression;
    while_expression_ast *while_expression;
    block_expression_ast *block_expression;
    let_expression_ast *let_expression;
    case_expression_ast *case_expression;

  };

struct program_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_program
    };

    const list_node<class_ast *> *klass_sequence;

  };

struct relational_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_relational_expression
    };

    const list_node<additive_expression_ast *> *expression_sequence;
    std::size_t op;

  };

struct unary_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_unary_expression
    };

    std::size_t op;
    primary_expression_ast *expression;

  };

struct while_expression_ast: public cool_ast_node
  {
    enum
    {
      KIND = Kind_while_expression
    };

    expression_ast *condition;
    expression_ast *loop_expression;

  };



class cool
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
      Token_AT = 1000,
      Token_CASE = 1001,
      Token_CLASS = 1002,
      Token_COLON = 1003,
      Token_COMMA = 1004,
      Token_DIVIDE = 1005,
      Token_DOT = 1006,
      Token_ELSE = 1007,
      Token_EOF = 1008,
      Token_EQUAL = 1009,
      Token_ESAC = 1010,
      Token_FALSE = 1011,
      Token_FI = 1012,
      Token_ID = 1013,
      Token_IF = 1014,
      Token_IN = 1015,
      Token_INHERITS = 1016,
      Token_INTEGER = 1017,
      Token_ISVOID = 1018,
      Token_LBRACE = 1019,
      Token_LEFT_ARROW = 1020,
      Token_LESS = 1021,
      Token_LESS_EQUAL = 1022,
      Token_LET = 1023,
      Token_LOOP = 1024,
      Token_LPAREN = 1025,
      Token_MINUS = 1026,
      Token_NEW = 1027,
      Token_NOT = 1028,
      Token_OF = 1029,
      Token_PLUS = 1030,
      Token_POOL = 1031,
      Token_RBRACE = 1032,
      Token_RIGHT_ARROW = 1033,
      Token_RPAREN = 1034,
      Token_SEMICOLON = 1035,
      Token_STAR = 1036,
      Token_STRING = 1037,
      Token_THEN = 1038,
      Token_TILDE = 1039,
      Token_TRUE = 1040,
      Token_TYPE = 1041,
      Token_WHILE = 1042,
      token_type_size
    }; // token_type_enum

    cool()
    {
      memory_pool = 0;
      token_stream = 0;
      yytoken = Token_EOF;
    }

    bool parse_additive_expression(additive_expression_ast **yynode);
    bool parse_block_expression(block_expression_ast **yynode);
    bool parse_case_condition(case_condition_ast **yynode);
    bool parse_case_expression(case_expression_ast **yynode);
    bool parse_class(class_ast **yynode);
    bool parse_expression(expression_ast **yynode);
    bool parse_feature(feature_ast **yynode);
    bool parse_formal(formal_ast **yynode);
    bool parse_if_expression(if_expression_ast **yynode);
    bool parse_let_declaration(let_declaration_ast **yynode);
    bool parse_let_expression(let_expression_ast **yynode);
    bool parse_multiplicative_expression(multiplicative_expression_ast **yynode);
    bool parse_postfix_expression(postfix_expression_ast **yynode);
    bool parse_primary_expression(primary_expression_ast **yynode);
    bool parse_program(program_ast **yynode);
    bool parse_relational_expression(relational_expression_ast **yynode);
    bool parse_unary_expression(unary_expression_ast **yynode);
    bool parse_while_expression(while_expression_ast **yynode);
  };
class cool_visitor
  {
    typedef void (cool_visitor::*parser_fun_t)(cool_ast_node *);
    static parser_fun_t _S_parser_table[];

  public:
    virtual ~cool_visitor()
    {}
    virtual void visit_node(cool_ast_node *node)
    {
      if (node)
        (this->*_S_parser_table[node->kind - 1000])(node);
    }
    virtual void visit_additive_expression(additive_expression_ast *)
  {}
    virtual void visit_block_expression(block_expression_ast *)
    {}
    virtual void visit_case_condition(case_condition_ast *)
    {}
    virtual void visit_case_expression(case_expression_ast *)
    {}
    virtual void visit_class(class_ast *)
    {}
    virtual void visit_expression(expression_ast *)
    {}
    virtual void visit_feature(feature_ast *)
    {}
    virtual void visit_formal(formal_ast *)
    {}
    virtual void visit_if_expression(if_expression_ast *)
    {}
    virtual void visit_let_declaration(let_declaration_ast *)
    {}
    virtual void visit_let_expression(let_expression_ast *)
    {}
    virtual void visit_multiplicative_expression(multiplicative_expression_ast *)
    {}
    virtual void visit_postfix_expression(postfix_expression_ast *)
    {}
    virtual void visit_primary_expression(primary_expression_ast *)
    {}
    virtual void visit_program(program_ast *)
    {}
    virtual void visit_relational_expression(relational_expression_ast *)
    {}
    virtual void visit_unary_expression(unary_expression_ast *)
    {}
    virtual void visit_while_expression(while_expression_ast *)
    {}
  }
;
class cool_default_visitor: public cool_visitor
  {
  public:
    virtual void visit_additive_expression(additive_expression_ast *node)
    {
      if (node->expression_sequence)
        {
          const list_node<multiplicative_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_block_expression(block_expression_ast *node)
    {
      if (node->expression_sequence)
        {
          const list_node<expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_case_condition(case_condition_ast *node)
    {
      visit_node(node->expression);
    }

    virtual void visit_case_expression(case_expression_ast *node)
    {
      visit_node(node->expression);
      if (node->condition_sequence)
        {
          const list_node<case_condition_ast*> *__it = node->condition_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_class(class_ast *node)
    {
      if (node->feature_sequence)
        {
          const list_node<feature_ast*> *__it = node->feature_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_expression(expression_ast *node)
    {
      visit_node(node->expression);
    }

    virtual void visit_feature(feature_ast *node)
    {
      if (node->formal_sequence)
        {
          const list_node<formal_ast*> *__it = node->formal_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
      visit_node(node->expression);
    }

    virtual void visit_formal(formal_ast *node)
  {}

    virtual void visit_if_expression(if_expression_ast *node)
    {
      visit_node(node->condition);
      visit_node(node->true_expression);
      visit_node(node->false_expression);
    }

    virtual void visit_let_declaration(let_declaration_ast *node)
    {
      visit_node(node->expression);
    }

    virtual void visit_let_expression(let_expression_ast *node)
    {
      if (node->declaration_sequence)
        {
          const list_node<let_declaration_ast*> *__it = node->declaration_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
      visit_node(node->body_expression);
    }

    virtual void visit_multiplicative_expression(multiplicative_expression_ast *node)
    {
      if (node->expression_sequence)
        {
          const list_node<postfix_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_postfix_expression(postfix_expression_ast *node)
    {
      visit_node(node->base_expression);
      if (node->arguments_sequence)
        {
          const list_node<expression_ast*> *__it = node->arguments_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_primary_expression(primary_expression_ast *node)
    {
      visit_node(node->expression);
      if (node->argument_sequence)
        {
          const list_node<expression_ast*> *__it = node->argument_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
      visit_node(node->if_expression);
      visit_node(node->while_expression);
      visit_node(node->block_expression);
      visit_node(node->let_expression);
      visit_node(node->case_expression);
    }

    virtual void visit_program(program_ast *node)
    {
      if (node->klass_sequence)
        {
          const list_node<class_ast*> *__it = node->klass_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_relational_expression(relational_expression_ast *node)
    {
      if (node->expression_sequence)
        {
          const list_node<additive_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
          do
            {
              visit_node(__it->element);
              __it = __it->next;
            }
          while (__it != __end);
        }
    }

    virtual void visit_unary_expression(unary_expression_ast *node)
    {
      visit_node(node->expression);
    }

    virtual void visit_while_expression(while_expression_ast *node)
    {
      visit_node(node->condition);
      visit_node(node->loop_expression);
    }

  };
#endif


