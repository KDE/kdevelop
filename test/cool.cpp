// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "cool.h"


bool cool::parse_additive_expression(additive_expression_ast **yynode)
{
  *yynode = create<additive_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      multiplicative_expression_ast *__node_0 = 0;
      if (!parse_multiplicative_expression(&__node_0))
        {
          fprintf(stderr, "** ERROR expected ``multiplicative_expression''\n");
          return false;
        }
      (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_0, memory_pool);
      while (yytoken == Token_PLUS
             || yytoken == Token_MINUS)
        {
          if (yytoken == Token_PLUS)
            {
              if (yytoken != Token_PLUS)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          else if (yytoken == Token_MINUS)
            {
              if (yytoken != Token_MINUS)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          multiplicative_expression_ast *__node_1 = 0;
          if (!parse_multiplicative_expression(&__node_1))
            {
              fprintf(stderr, "** ERROR expected ``multiplicative_expression''\n");
              return false;
            }
          (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_1, memory_pool);
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_block_expression(block_expression_ast **yynode)
{
  *yynode = create<block_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE)
    {
      if (yytoken != Token_LBRACE)
        {
          fprintf(stderr, "** ERROR expected token ``LBRACE''\n");
          return false;
        }
      yylex();
      while (yytoken == Token_LBRACE
             || yytoken == Token_ID
             || yytoken == Token_LPAREN
             || yytoken == Token_INTEGER
             || yytoken == Token_STRING
             || yytoken == Token_TRUE
             || yytoken == Token_FALSE
             || yytoken == Token_NEW
             || yytoken == Token_TILDE
             || yytoken == Token_NOT
             || yytoken == Token_ISVOID
             || yytoken == Token_IF
             || yytoken == Token_WHILE
             || yytoken == Token_LET
             || yytoken == Token_CASE)
        {
          expression_ast *__node_2 = 0;
          if (!parse_expression(&__node_2))
            {
              fprintf(stderr, "** ERROR expected ``expression''\n");
              return false;
            }
          (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_2, memory_pool);
          if (yytoken != Token_SEMICOLON)
            {
              fprintf(stderr, "** ERROR expected token ``SEMICOLON''\n");
              return false;
            }
          yylex();
        }
      if (yytoken != Token_RBRACE)
        {
          fprintf(stderr, "** ERROR expected token ``RBRACE''\n");
          return false;
        }
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_case_condition(case_condition_ast **yynode)
{
  *yynode = create<case_condition_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_ID)
    {
      if (yytoken != Token_ID)
        {
          fprintf(stderr, "** ERROR expected token ``name''\n");
          return false;
        }
      (*yynode)->name = token_stream->index() - 1;
      yylex();
      if (yytoken != Token_COLON)
        {
          fprintf(stderr, "** ERROR expected token ``COLON''\n");
          return false;
        }
      yylex();
      if (yytoken != Token_TYPE)
        {
          fprintf(stderr, "** ERROR expected token ``type''\n");
          return false;
        }
      (*yynode)->type = token_stream->index() - 1;
      yylex();
      if (yytoken != Token_RIGHT_ARROW)
        {
          fprintf(stderr, "** ERROR expected token ``RIGHT_ARROW''\n");
          return false;
        }
      yylex();
      expression_ast *__node_3 = 0;
      if (!parse_expression(&__node_3))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->expression = __node_3;
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_case_expression(case_expression_ast **yynode)
{
  *yynode = create<case_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_CASE)
    {
      if (yytoken != Token_CASE)
        {
          fprintf(stderr, "** ERROR expected token ``CASE''\n");
          return false;
        }
      yylex();
      expression_ast *__node_4 = 0;
      if (!parse_expression(&__node_4))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->expression = __node_4;
      if (yytoken != Token_OF)
        {
          fprintf(stderr, "** ERROR expected token ``OF''\n");
          return false;
        }
      yylex();
      while (yytoken == Token_ID)
        {
          case_condition_ast *__node_5 = 0;
          if (!parse_case_condition(&__node_5))
            {
              fprintf(stderr, "** ERROR expected ``case_condition''\n");
              return false;
            }
          (*yynode)->condition_sequence = snoc((*yynode)->condition_sequence, __node_5, memory_pool);
          if (yytoken != Token_SEMICOLON)
            {
              fprintf(stderr, "** ERROR expected token ``SEMICOLON''\n");
              return false;
            }
          yylex();
        }
      if (yytoken != Token_ESAC)
        {
          fprintf(stderr, "** ERROR expected token ``ESAC''\n");
          return false;
        }
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_class(class_ast **yynode)
{
  *yynode = create<class_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_CLASS)
    {
      if (yytoken != Token_CLASS)
        {
          fprintf(stderr, "** ERROR expected token ``CLASS''\n");
          return false;
        }
      yylex();
      if (yytoken != Token_TYPE)
        {
          fprintf(stderr, "** ERROR expected token ``type''\n");
          return false;
        }
      (*yynode)->type = token_stream->index() - 1;
      yylex();
      if (yytoken == Token_INHERITS)
        {
          if (yytoken != Token_INHERITS)
            {
              fprintf(stderr, "** ERROR expected token ``INHERITS''\n");
              return false;
            }
          yylex();
          if (yytoken != Token_TYPE)
            {
              fprintf(stderr, "** ERROR expected token ``base_type''\n");
              return false;
            }
          (*yynode)->base_type = token_stream->index() - 1;
          yylex();
        }
      else if (true /*epsilon*/)
      {}
      if (yytoken != Token_LBRACE)
        {
          fprintf(stderr, "** ERROR expected token ``LBRACE''\n");
          return false;
        }
      yylex();
      while (yytoken == Token_ID)
        {
          feature_ast *__node_6 = 0;
          if (!parse_feature(&__node_6))
            {
              fprintf(stderr, "** ERROR expected ``feature''\n");
              return false;
            }
          (*yynode)->feature_sequence = snoc((*yynode)->feature_sequence, __node_6, memory_pool);
          if (yytoken != Token_SEMICOLON)
            {
              fprintf(stderr, "** ERROR expected token ``SEMICOLON''\n");
              return false;
            }
          yylex();
        }
      if (yytoken != Token_RBRACE)
        {
          fprintf(stderr, "** ERROR expected token ``RBRACE''\n");
          return false;
        }
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_expression(expression_ast **yynode)
{
  *yynode = create<expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      relational_expression_ast *__node_7 = 0;
      if (!parse_relational_expression(&__node_7))
        {
          fprintf(stderr, "** ERROR expected ``relational_expression''\n");
          return false;
        }
      (*yynode)->expression = __node_7;
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_feature(feature_ast **yynode)
{
  *yynode = create<feature_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_ID)
    {
      if (( LA(2).kind == Token_LPAREN ) && (yytoken == Token_ID))
        {
          if (yytoken != Token_ID)
            {
              fprintf(stderr, "** ERROR expected token ``name''\n");
              return false;
            }
          (*yynode)->name = token_stream->index() - 1;
          yylex();
          if (yytoken != Token_LPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``LPAREN''\n");
              return false;
            }
          yylex();
          if (yytoken == Token_ID)
            {
              formal_ast *__node_8 = 0;
              if (!parse_formal(&__node_8))
                {
                  fprintf(stderr, "** ERROR expected ``formal''\n");
                  return false;
                }
              (*yynode)->formal_sequence = snoc((*yynode)->formal_sequence, __node_8, memory_pool);
              while (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    {
                      fprintf(stderr, "** ERROR expected token ``COMMA''\n");
                      return false;
                    }
                  yylex();
                  formal_ast *__node_9 = 0;
                  if (!parse_formal(&__node_9))
                    {
                      fprintf(stderr, "** ERROR expected ``formal''\n");
                      return false;
                    }
                  (*yynode)->formal_sequence = snoc((*yynode)->formal_sequence, __node_9, memory_pool);
                }
            }
          else if (true /*epsilon*/)
          {}
          if (yytoken != Token_RPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``RPAREN''\n");
              return false;
            }
          yylex();
          if (yytoken != Token_COLON)
            {
              fprintf(stderr, "** ERROR expected token ``COLON''\n");
              return false;
            }
          yylex();
          if (yytoken != Token_TYPE)
            {
              fprintf(stderr, "** ERROR expected token ``type''\n");
              return false;
            }
          (*yynode)->type = token_stream->index() - 1;
          yylex();
          if (yytoken != Token_LBRACE)
            {
              fprintf(stderr, "** ERROR expected token ``LBRACE''\n");
              return false;
            }
          yylex();
          expression_ast *__node_10 = 0;
          if (!parse_expression(&__node_10))
            {
              fprintf(stderr, "** ERROR expected ``expression''\n");
              return false;
            }
          (*yynode)->expression = __node_10;
          if (yytoken != Token_RBRACE)
            {
              fprintf(stderr, "** ERROR expected token ``RBRACE''\n");
              return false;
            }
          yylex();
        }
      else if (( LA(2).kind == Token_COLON ) && (yytoken == Token_ID))
        {
          if (yytoken != Token_ID)
            {
              fprintf(stderr, "** ERROR expected token ``name''\n");
              return false;
            }
          (*yynode)->name = token_stream->index() - 1;
          yylex();
          if (yytoken != Token_COLON)
            {
              fprintf(stderr, "** ERROR expected token ``COLON''\n");
              return false;
            }
          yylex();
          if (yytoken != Token_TYPE)
            {
              fprintf(stderr, "** ERROR expected token ``type''\n");
              return false;
            }
          (*yynode)->type = token_stream->index() - 1;
          yylex();
          if (yytoken == Token_LEFT_ARROW)
            {
              if (yytoken != Token_LEFT_ARROW)
                {
                  fprintf(stderr, "** ERROR expected token ``LEFT_ARROW''\n");
                  return false;
                }
              yylex();
              expression_ast *__node_11 = 0;
              if (!parse_expression(&__node_11))
                {
                  fprintf(stderr, "** ERROR expected ``expression''\n");
                  return false;
                }
              (*yynode)->expression = __node_11;
            }
          else if (true /*epsilon*/)
          {}
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_formal(formal_ast **yynode)
{
  *yynode = create<formal_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_ID)
    {
      if (yytoken != Token_ID)
        {
          fprintf(stderr, "** ERROR expected token ``name''\n");
          return false;
        }
      (*yynode)->name = token_stream->index() - 1;
      yylex();
      if (yytoken != Token_COLON)
        {
          fprintf(stderr, "** ERROR expected token ``COLON''\n");
          return false;
        }
      yylex();
      if (yytoken != Token_TYPE)
        {
          fprintf(stderr, "** ERROR expected token ``type''\n");
          return false;
        }
      (*yynode)->type = token_stream->index() - 1;
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_if_expression(if_expression_ast **yynode)
{
  *yynode = create<if_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_IF)
    {
      if (yytoken != Token_IF)
        {
          fprintf(stderr, "** ERROR expected token ``IF''\n");
          return false;
        }
      yylex();
      expression_ast *__node_12 = 0;
      if (!parse_expression(&__node_12))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->condition = __node_12;
      if (yytoken != Token_THEN)
        {
          fprintf(stderr, "** ERROR expected token ``THEN''\n");
          return false;
        }
      yylex();
      expression_ast *__node_13 = 0;
      if (!parse_expression(&__node_13))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->true_expression = __node_13;
      if (yytoken != Token_ELSE)
        {
          fprintf(stderr, "** ERROR expected token ``ELSE''\n");
          return false;
        }
      yylex();
      expression_ast *__node_14 = 0;
      if (!parse_expression(&__node_14))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->false_expression = __node_14;
      if (yytoken != Token_FI)
        {
          fprintf(stderr, "** ERROR expected token ``FI''\n");
          return false;
        }
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_let_declaration(let_declaration_ast **yynode)
{
  *yynode = create<let_declaration_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_ID)
    {
      if (yytoken != Token_ID)
        {
          fprintf(stderr, "** ERROR expected token ``name''\n");
          return false;
        }
      (*yynode)->name = token_stream->index() - 1;
      yylex();
      if (yytoken != Token_COLON)
        {
          fprintf(stderr, "** ERROR expected token ``COLON''\n");
          return false;
        }
      yylex();
      if (yytoken != Token_TYPE)
        {
          fprintf(stderr, "** ERROR expected token ``type''\n");
          return false;
        }
      (*yynode)->type = token_stream->index() - 1;
      yylex();
      if (yytoken == Token_LEFT_ARROW)
        {
          if (yytoken != Token_LEFT_ARROW)
            {
              fprintf(stderr, "** ERROR expected token ``LEFT_ARROW''\n");
              return false;
            }
          yylex();
          expression_ast *__node_15 = 0;
          if (!parse_expression(&__node_15))
            {
              fprintf(stderr, "** ERROR expected ``expression''\n");
              return false;
            }
          (*yynode)->expression = __node_15;
        }
      else if (true /*epsilon*/)
      {}
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_let_expression(let_expression_ast **yynode)
{
  *yynode = create<let_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LET)
    {
      if (yytoken != Token_LET)
        {
          fprintf(stderr, "** ERROR expected token ``LET''\n");
          return false;
        }
      yylex();
      let_declaration_ast *__node_16 = 0;
      if (!parse_let_declaration(&__node_16))
        {
          fprintf(stderr, "** ERROR expected ``let_declaration''\n");
          return false;
        }
      (*yynode)->declaration_sequence = snoc((*yynode)->declaration_sequence, __node_16, memory_pool);
      while (yytoken == Token_COMMA)
        {
          if (yytoken != Token_COMMA)
            {
              fprintf(stderr, "** ERROR expected token ``COMMA''\n");
              return false;
            }
          yylex();
          let_declaration_ast *__node_17 = 0;
          if (!parse_let_declaration(&__node_17))
            {
              fprintf(stderr, "** ERROR expected ``let_declaration''\n");
              return false;
            }
          (*yynode)->declaration_sequence = snoc((*yynode)->declaration_sequence, __node_17, memory_pool);
        }
      if (yytoken != Token_IN)
        {
          fprintf(stderr, "** ERROR expected token ``IN''\n");
          return false;
        }
      yylex();
      expression_ast *__node_18 = 0;
      if (!parse_expression(&__node_18))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->body_expression = __node_18;
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_multiplicative_expression(multiplicative_expression_ast **yynode)
{
  *yynode = create<multiplicative_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      postfix_expression_ast *__node_19 = 0;
      if (!parse_postfix_expression(&__node_19))
        {
          fprintf(stderr, "** ERROR expected ``postfix_expression''\n");
          return false;
        }
      (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_19, memory_pool);
      while (yytoken == Token_STAR
             || yytoken == Token_DIVIDE)
        {
          if (yytoken == Token_STAR)
            {
              if (yytoken != Token_STAR)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          else if (yytoken == Token_DIVIDE)
            {
              if (yytoken != Token_DIVIDE)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          postfix_expression_ast *__node_20 = 0;
          if (!parse_postfix_expression(&__node_20))
            {
              fprintf(stderr, "** ERROR expected ``postfix_expression''\n");
              return false;
            }
          (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_20, memory_pool);
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_postfix_expression(postfix_expression_ast **yynode)
{
  *yynode = create<postfix_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      unary_expression_ast *__node_21 = 0;
      if (!parse_unary_expression(&__node_21))
        {
          fprintf(stderr, "** ERROR expected ``unary_expression''\n");
          return false;
        }
      (*yynode)->base_expression = __node_21;
      while (yytoken == Token_AT
             || yytoken == Token_DOT)
        {
          if (yytoken == Token_AT)
            {
              if (yytoken != Token_AT)
                {
                  fprintf(stderr, "** ERROR expected token ``AT''\n");
                  return false;
                }
              yylex();
              if (yytoken != Token_TYPE)
                {
                  fprintf(stderr, "** ERROR expected token ``at_type''\n");
                  return false;
                }
              (*yynode)->at_type = token_stream->index() - 1;
              yylex();
              if (yytoken != Token_DOT)
                {
                  fprintf(stderr, "** ERROR expected token ``DOT''\n");
                  return false;
                }
              yylex();
              if (yytoken != Token_ID)
                {
                  fprintf(stderr, "** ERROR expected token ``name''\n");
                  return false;
                }
              (*yynode)->name = token_stream->index() - 1;
              yylex();
              if (yytoken != Token_LPAREN)
                {
                  fprintf(stderr, "** ERROR expected token ``LPAREN''\n");
                  return false;
                }
              yylex();
              if (yytoken == Token_LBRACE
                  || yytoken == Token_ID
                  || yytoken == Token_LPAREN
                  || yytoken == Token_INTEGER
                  || yytoken == Token_STRING
                  || yytoken == Token_TRUE
                  || yytoken == Token_FALSE
                  || yytoken == Token_NEW
                  || yytoken == Token_TILDE
                  || yytoken == Token_NOT
                  || yytoken == Token_ISVOID
                  || yytoken == Token_IF
                  || yytoken == Token_WHILE
                  || yytoken == Token_LET
                  || yytoken == Token_CASE)
                {
                  expression_ast *__node_22 = 0;
                  if (!parse_expression(&__node_22))
                    {
                      fprintf(stderr, "** ERROR expected ``expression''\n");
                      return false;
                    }
                  (*yynode)->arguments_sequence = snoc((*yynode)->arguments_sequence, __node_22, memory_pool);
                  while (yytoken == Token_COMMA)
                    {
                      if (yytoken != Token_COMMA)
                        {
                          fprintf(stderr, "** ERROR expected token ``COMMA''\n");
                          return false;
                        }
                      yylex();
                      expression_ast *__node_23 = 0;
                      if (!parse_expression(&__node_23))
                        {
                          fprintf(stderr, "** ERROR expected ``expression''\n");
                          return false;
                        }
                      (*yynode)->arguments_sequence = snoc((*yynode)->arguments_sequence, __node_23, memory_pool);
                    }
                }
              else if (true /*epsilon*/)
              {}
              if (yytoken != Token_RPAREN)
                {
                  fprintf(stderr, "** ERROR expected token ``RPAREN''\n");
                  return false;
                }
              yylex();
            }
          else if (yytoken == Token_DOT)
            {
              if (yytoken != Token_DOT)
                {
                  fprintf(stderr, "** ERROR expected token ``DOT''\n");
                  return false;
                }
              yylex();
              if (yytoken != Token_ID)
                {
                  fprintf(stderr, "** ERROR expected token ``name''\n");
                  return false;
                }
              (*yynode)->name = token_stream->index() - 1;
              yylex();
              if (yytoken != Token_LPAREN)
                {
                  fprintf(stderr, "** ERROR expected token ``LPAREN''\n");
                  return false;
                }
              yylex();
              if (yytoken == Token_LBRACE
                  || yytoken == Token_ID
                  || yytoken == Token_LPAREN
                  || yytoken == Token_INTEGER
                  || yytoken == Token_STRING
                  || yytoken == Token_TRUE
                  || yytoken == Token_FALSE
                  || yytoken == Token_NEW
                  || yytoken == Token_TILDE
                  || yytoken == Token_NOT
                  || yytoken == Token_ISVOID
                  || yytoken == Token_IF
                  || yytoken == Token_WHILE
                  || yytoken == Token_LET
                  || yytoken == Token_CASE)
                {
                  expression_ast *__node_24 = 0;
                  if (!parse_expression(&__node_24))
                    {
                      fprintf(stderr, "** ERROR expected ``expression''\n");
                      return false;
                    }
                  (*yynode)->arguments_sequence = snoc((*yynode)->arguments_sequence, __node_24, memory_pool);
                  while (yytoken == Token_COMMA)
                    {
                      if (yytoken != Token_COMMA)
                        {
                          fprintf(stderr, "** ERROR expected token ``COMMA''\n");
                          return false;
                        }
                      yylex();
                      expression_ast *__node_25 = 0;
                      if (!parse_expression(&__node_25))
                        {
                          fprintf(stderr, "** ERROR expected ``expression''\n");
                          return false;
                        }
                      (*yynode)->arguments_sequence = snoc((*yynode)->arguments_sequence, __node_25, memory_pool);
                    }
                }
              else if (true /*epsilon*/)
              {}
              if (yytoken != Token_RPAREN)
                {
                  fprintf(stderr, "** ERROR expected token ``RPAREN''\n");
                  return false;
                }
              yylex();
            }
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_primary_expression(primary_expression_ast **yynode)
{
  *yynode = create<primary_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      if (( LA(2).kind == Token_LEFT_ARROW ) && (yytoken == Token_ID))
        {
          if (yytoken != Token_ID)
            {
              fprintf(stderr, "** ERROR expected token ``name''\n");
              return false;
            }
          (*yynode)->name = token_stream->index() - 1;
          yylex();
          if (yytoken != Token_LEFT_ARROW)
            {
              fprintf(stderr, "** ERROR expected token ``LEFT_ARROW''\n");
              return false;
            }
          yylex();
          expression_ast *__node_26 = 0;
          if (!parse_expression(&__node_26))
            {
              fprintf(stderr, "** ERROR expected ``expression''\n");
              return false;
            }
          (*yynode)->expression = __node_26;
        }
      else if (( LA(2).kind == Token_LPAREN ) && (yytoken == Token_ID))
        {
          if (yytoken != Token_ID)
            {
              fprintf(stderr, "** ERROR expected token ``name''\n");
              return false;
            }
          (*yynode)->name = token_stream->index() - 1;
          yylex();
          if (yytoken != Token_LPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``LPAREN''\n");
              return false;
            }
          yylex();
          if (yytoken == Token_LBRACE
              || yytoken == Token_ID
              || yytoken == Token_LPAREN
              || yytoken == Token_INTEGER
              || yytoken == Token_STRING
              || yytoken == Token_TRUE
              || yytoken == Token_FALSE
              || yytoken == Token_NEW
              || yytoken == Token_TILDE
              || yytoken == Token_NOT
              || yytoken == Token_ISVOID
              || yytoken == Token_IF
              || yytoken == Token_WHILE
              || yytoken == Token_LET
              || yytoken == Token_CASE)
            {
              expression_ast *__node_27 = 0;
              if (!parse_expression(&__node_27))
                {
                  fprintf(stderr, "** ERROR expected ``expression''\n");
                  return false;
                }
              (*yynode)->argument_sequence = snoc((*yynode)->argument_sequence, __node_27, memory_pool);
              while (yytoken == Token_COMMA)
                {
                  if (yytoken != Token_COMMA)
                    {
                      fprintf(stderr, "** ERROR expected token ``COMMA''\n");
                      return false;
                    }
                  yylex();
                  expression_ast *__node_28 = 0;
                  if (!parse_expression(&__node_28))
                    {
                      fprintf(stderr, "** ERROR expected ``expression''\n");
                      return false;
                    }
                  (*yynode)->argument_sequence = snoc((*yynode)->argument_sequence, __node_28, memory_pool);
                }
            }
          else if (true /*epsilon*/)
          {}
          if (yytoken != Token_RPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``RPAREN''\n");
              return false;
            }
          yylex();
        }
      else if (yytoken == Token_ID)
        {
          if (yytoken != Token_ID)
            {
              fprintf(stderr, "** ERROR expected token ``variable''\n");
              return false;
            }
          (*yynode)->variable = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_INTEGER)
        {
          if (yytoken != Token_INTEGER)
            {
              fprintf(stderr, "** ERROR expected token ``integer_literal''\n");
              return false;
            }
          (*yynode)->integer_literal = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_STRING)
        {
          if (yytoken != Token_STRING)
            {
              fprintf(stderr, "** ERROR expected token ``string_literal''\n");
              return false;
            }
          (*yynode)->string_literal = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_TRUE)
        {
          if (yytoken != Token_TRUE)
            {
              fprintf(stderr, "** ERROR expected token ``true_literal''\n");
              return false;
            }
          (*yynode)->true_literal = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_FALSE)
        {
          if (yytoken != Token_FALSE)
            {
              fprintf(stderr, "** ERROR expected token ``false_literal''\n");
              return false;
            }
          (*yynode)->false_literal = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_NEW)
        {
          if (yytoken != Token_NEW)
            {
              fprintf(stderr, "** ERROR expected token ``NEW''\n");
              return false;
            }
          yylex();
          if (yytoken != Token_TYPE)
            {
              fprintf(stderr, "** ERROR expected token ``new_type''\n");
              return false;
            }
          (*yynode)->new_type = token_stream->index() - 1;
          yylex();
        }
      else if (yytoken == Token_LPAREN)
        {
          if (yytoken != Token_LPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``LPAREN''\n");
              return false;
            }
          yylex();
          expression_ast *__node_29 = 0;
          if (!parse_expression(&__node_29))
            {
              fprintf(stderr, "** ERROR expected ``expression''\n");
              return false;
            }
          (*yynode)->expression = __node_29;
          if (yytoken != Token_RPAREN)
            {
              fprintf(stderr, "** ERROR expected token ``RPAREN''\n");
              return false;
            }
          yylex();
        }
      else if (yytoken == Token_IF)
        {
          if_expression_ast *__node_30 = 0;
          if (!parse_if_expression(&__node_30))
            {
              fprintf(stderr, "** ERROR expected ``if_expression''\n");
              return false;
            }
          (*yynode)->if_expression = __node_30;
        }
      else if (yytoken == Token_WHILE)
        {
          while_expression_ast *__node_31 = 0;
          if (!parse_while_expression(&__node_31))
            {
              fprintf(stderr, "** ERROR expected ``while_expression''\n");
              return false;
            }
          (*yynode)->while_expression = __node_31;
        }
      else if (yytoken == Token_LBRACE)
        {
          block_expression_ast *__node_32 = 0;
          if (!parse_block_expression(&__node_32))
            {
              fprintf(stderr, "** ERROR expected ``block_expression''\n");
              return false;
            }
          (*yynode)->block_expression = __node_32;
        }
      else if (yytoken == Token_LET)
        {
          let_expression_ast *__node_33 = 0;
          if (!parse_let_expression(&__node_33))
            {
              fprintf(stderr, "** ERROR expected ``let_expression''\n");
              return false;
            }
          (*yynode)->let_expression = __node_33;
        }
      else if (yytoken == Token_CASE)
        {
          case_expression_ast *__node_34 = 0;
          if (!parse_case_expression(&__node_34))
            {
              fprintf(stderr, "** ERROR expected ``case_expression''\n");
              return false;
            }
          (*yynode)->case_expression = __node_34;
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_program(program_ast **yynode)
{
  *yynode = create<program_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_CLASS || yytoken == Token_EOF)
    {
      while (yytoken == Token_CLASS)
        {
          class_ast *__node_35 = 0;
          if (!parse_class(&__node_35))
            {
              fprintf(stderr, "** ERROR expected ``class''\n");
              return false;
            }
          (*yynode)->klass_sequence = snoc((*yynode)->klass_sequence, __node_35, memory_pool);
          if (yytoken != Token_SEMICOLON)
            {
              fprintf(stderr, "** ERROR expected token ``SEMICOLON''\n");
              return false;
            }
          yylex();
        }
      if (Token_EOF != yytoken)
        {
          return false;
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_relational_expression(relational_expression_ast **yynode)
{
  *yynode = create<relational_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      additive_expression_ast *__node_36 = 0;
      if (!parse_additive_expression(&__node_36))
        {
          fprintf(stderr, "** ERROR expected ``additive_expression''\n");
          return false;
        }
      (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_36, memory_pool);
      while (yytoken == Token_EQUAL
             || yytoken == Token_LESS_EQUAL
             || yytoken == Token_LESS)
        {
          if (yytoken == Token_EQUAL)
            {
              if (yytoken != Token_EQUAL)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          else if (yytoken == Token_LESS_EQUAL)
            {
              if (yytoken != Token_LESS_EQUAL)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          else if (yytoken == Token_LESS)
            {
              if (yytoken != Token_LESS)
                {
                  fprintf(stderr, "** ERROR expected token ``op''\n");
                  return false;
                }
              (*yynode)->op = token_stream->index() - 1;
              yylex();
            }
          additive_expression_ast *__node_37 = 0;
          if (!parse_additive_expression(&__node_37))
            {
              fprintf(stderr, "** ERROR expected ``additive_expression''\n");
              return false;
            }
          (*yynode)->expression_sequence = snoc((*yynode)->expression_sequence, __node_37, memory_pool);
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_unary_expression(unary_expression_ast **yynode)
{
  *yynode = create<unary_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_LBRACE
      || yytoken == Token_ID
      || yytoken == Token_LPAREN
      || yytoken == Token_INTEGER
      || yytoken == Token_STRING
      || yytoken == Token_TRUE
      || yytoken == Token_FALSE
      || yytoken == Token_NEW
      || yytoken == Token_TILDE
      || yytoken == Token_NOT
      || yytoken == Token_ISVOID
      || yytoken == Token_IF
      || yytoken == Token_WHILE
      || yytoken == Token_LET
      || yytoken == Token_CASE)
    {
      if (yytoken == Token_TILDE)
        {
          if (yytoken != Token_TILDE)
            {
              fprintf(stderr, "** ERROR expected token ``op''\n");
              return false;
            }
          (*yynode)->op = token_stream->index() - 1;
          yylex();
          primary_expression_ast *__node_38 = 0;
          if (!parse_primary_expression(&__node_38))
            {
              fprintf(stderr, "** ERROR expected ``primary_expression''\n");
              return false;
            }
          (*yynode)->expression = __node_38;
        }
      else if (yytoken == Token_NOT)
        {
          if (yytoken != Token_NOT)
            {
              fprintf(stderr, "** ERROR expected token ``op''\n");
              return false;
            }
          (*yynode)->op = token_stream->index() - 1;
          yylex();
          primary_expression_ast *__node_39 = 0;
          if (!parse_primary_expression(&__node_39))
            {
              fprintf(stderr, "** ERROR expected ``primary_expression''\n");
              return false;
            }
          (*yynode)->expression = __node_39;
        }
      else if (yytoken == Token_ISVOID)
        {
          if (yytoken != Token_ISVOID)
            {
              fprintf(stderr, "** ERROR expected token ``op''\n");
              return false;
            }
          (*yynode)->op = token_stream->index() - 1;
          yylex();
          primary_expression_ast *__node_40 = 0;
          if (!parse_primary_expression(&__node_40))
            {
              fprintf(stderr, "** ERROR expected ``primary_expression''\n");
              return false;
            }
          (*yynode)->expression = __node_40;
        }
      else if (yytoken == Token_LBRACE
               || yytoken == Token_ID
               || yytoken == Token_LPAREN
               || yytoken == Token_INTEGER
               || yytoken == Token_STRING
               || yytoken == Token_TRUE
               || yytoken == Token_FALSE
               || yytoken == Token_NEW
               || yytoken == Token_IF
               || yytoken == Token_WHILE
               || yytoken == Token_LET
               || yytoken == Token_CASE)
        {
          primary_expression_ast *__node_41 = 0;
          if (!parse_primary_expression(&__node_41))
            {
              fprintf(stderr, "** ERROR expected ``primary_expression''\n");
              return false;
            }
          (*yynode)->expression = __node_41;
        }
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

bool cool::parse_while_expression(while_expression_ast **yynode)
{
  *yynode = create<while_expression_ast>();

  (*yynode)->start_token = token_stream->index() - 1;

  if (yytoken == Token_WHILE)
    {
      if (yytoken != Token_WHILE)
        {
          fprintf(stderr, "** ERROR expected token ``WHILE''\n");
          return false;
        }
      yylex();
      expression_ast *__node_42 = 0;
      if (!parse_expression(&__node_42))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->condition = __node_42;
      if (yytoken != Token_LOOP)
        {
          fprintf(stderr, "** ERROR expected token ``LOOP''\n");
          return false;
        }
      yylex();
      expression_ast *__node_43 = 0;
      if (!parse_expression(&__node_43))
        {
          fprintf(stderr, "** ERROR expected ``expression''\n");
          return false;
        }
      (*yynode)->loop_expression = __node_43;
      if (yytoken != Token_POOL)
        {
          fprintf(stderr, "** ERROR expected token ``POOL''\n");
          return false;
        }
      yylex();
    }
  else
    {
      return false;
    }

  (*yynode)->end_token = token_stream->index() - 1;

  return true;
}

cool_visitor::parser_fun_t cool_visitor::_S_parser_table[] = {
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_additive_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_block_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_case_condition),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_case_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_class),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_feature),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_formal),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_if_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_let_declaration),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_let_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_multiplicative_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_postfix_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_primary_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_program),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_relational_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_unary_expression),
      reinterpret_cast<parser_fun_t>(&cool_visitor::visit_while_expression)
    }; // _S_parser_table[]

