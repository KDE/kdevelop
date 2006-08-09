-----------------------------------------------------------------------------
-- Copyright (c) 2006 Jakob Petsovits <jpetso@gmx.at>
--
-- This grammar is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Library General Public
-- License as published by the Free Software Foundation; either
-- version 2 of the License, or (at your option) any later version.
--
-- This grammar is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Library General Public License
-- along with this library; see the file COPYING.LIB.  If not, write to
-- the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
-- Boston, MA 02110-1301, USA.
-----------------------------------------------------------------------------


-----------------------------------------------------------------------------
-- Grammar for the pre-processor part of C# 2.0
-- Modelled after the reference grammar of the C# 2.0 language specification
-- (ECMA-334, Third Edition from June 2005, available at
-- http://www.ecma-international.org/publications/standards/Ecma-334.htm).
-----------------------------------------------------------------------------


------------------------------------------------------------
-- Global declarations
------------------------------------------------------------

[:
#include "csharp_pp_scope.h"

namespace csharp
{
  class parser;
  class Lexer;
}
:]



------------------------------------------------------------
-- Parser class members
------------------------------------------------------------

%parserclass (public declaration)
[:
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
:]

%parserclass (private declaration)
[:
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
:]

%parserclass (constructor)
[:
  set_token_stream(&_M_token_stream);
  set_memory_pool(&_M_memory_pool);
:]



------------------------------------------------------------
-- Enumeration types for additional AST members
------------------------------------------------------------

%namespace pp_equality_expression_rest
[:
  enum pp_equality_operator_enum {
    op_equal,
    op_not_equal
  };
:]

%namespace pp_primary_expression
[:
  enum pp_primary_expression_type_enum {
    type_true,
    type_false,
    type_conditional_symbol,
    type_parenthesis_expression
  };
:]

%namespace pp_declaration
[:
  enum pp_declaration_type_enum {
    type_define,
    type_undef
  };
:]

%namespace pp_diagnostic
[:
  enum pp_diagnostic_type_enum {
    type_error,
    type_warning
  };
:]

%namespace pp_region
[:
  enum pp_region_type_enum {
    type_region,
    type_endregion
  };
:]




------------------------------------------------------------
-- List of defined tokens
------------------------------------------------------------

-- start tokens:
%token PP_DEFINE ("#define"), PP_UNDEF ("#undef"), PP_IF ("#if"),
       PP_ELIF ("#elif"), PP_ELSE ("#else"), PP_ENDIF ("#endif"),
       PP_LINE ("#line"), PP_ERROR ("#error"), PP_WARNING ("#warning"),
       PP_REGION ("#region"), PP_ENDREGION ("#endregion"),
       PP_PRAGMA ("#pragma") ;;

-- value tokens and the line break:
%token PP_CONDITIONAL_SYMBOL ("pre-processor symbol"),
       PP_NEW_LINE ("line break"), PP_LINE_NUMBER ("line number"),
       PP_FILE_NAME ("file name (in double quotes)"),
       PP_IDENTIFIER_OR_KEYWORD ("identifier or keyword"),
       PP_MESSAGE ("single-line text"), PP_PRAGMA_TEXT ("pragma text") ;;

-- operators and keywords:
%token PP_EQUAL ("=="), PP_NOT_EQUAL ("!="), PP_LOG_AND ("&&"),
       PP_LOG_OR ("||"), PP_BANG ("!"), PP_LPAREN ("("), PP_RPAREN (")"),
       PP_TRUE ("true"), PP_FALSE ("false"), PP_DEFAULT ("default") ;;

-- token that makes the parser fail in any case:
%token PP_INVALID ("invalid token") ;;




------------------------------------------------------------
-- Start of the actual grammar
------------------------------------------------------------

--
-- Preprocessor rules. Those are handled before the grammar is
-- actually processed, as part of the lexing (called from within the lexer).
--

   (  declaration=pp_declaration
    | if_clause=pp_if_clause
    | elif_clause=pp_elif_clause
    | else_clause=pp_else_clause
    | endif_clause=pp_endif_clause
    | diagnostic=pp_diagnostic
    | region=pp_region
    | line=pp_line
    | pragma=pp_pragma
   )
   PP_NEW_LINE
-> pp_directive ;;

   (  PP_DEFINE [: (*yynode)->type = pp_declaration::type_define; :]
    | PP_UNDEF  [: (*yynode)->type = pp_declaration::type_undef;  :]
   )
   conditional_symbol=PP_CONDITIONAL_SYMBOL
-> pp_declaration [
     member variable type: pp_declaration::pp_declaration_type_enum;
] ;;

   PP_IF expression=pp_expression
-> pp_if_clause ;;

   PP_ELIF expression=pp_expression
-> pp_elif_clause ;;

   PP_ELSE
-> pp_else_clause ;;

   PP_ENDIF
-> pp_endif_clause ;;

   (  PP_ERROR    [: (*yynode)->type = pp_diagnostic::type_error;   :]
    | PP_WARNING  [: (*yynode)->type = pp_diagnostic::type_warning; :]
   )
   (message=PP_MESSAGE | 0)
-> pp_diagnostic [
     member variable type: pp_diagnostic::pp_diagnostic_type_enum;
] ;;

   (  PP_REGION     [: (*yynode)->type = pp_region::type_region;    :]
    | PP_ENDREGION  [: (*yynode)->type = pp_region::type_endregion; :]
   )
   (label=PP_MESSAGE | 0)
-> pp_region [
     member variable type: pp_region::pp_region_type_enum;
] ;;

   PP_LINE
   (  line_number=PP_LINE_NUMBER (file_name=PP_FILE_NAME | 0)
    | token_default=PP_DEFAULT
    | identifier_or_keyword=PP_IDENTIFIER_OR_KEYWORD
   )
-> pp_line ;;

   PP_PRAGMA (pragma_text=PP_PRAGMA_TEXT | 0)
-> pp_pragma ;;



-- Expressions used in #if and #elif

   #expression=pp_and_expression @ PP_LOG_OR
-> pp_expression ;;

   #expression=pp_equality_expression @ PP_LOG_AND
-> pp_and_expression ;;

   expression=pp_unary_expression
   (#additional_expression=pp_equality_expression_rest)*
-> pp_equality_expression ;;

   (  PP_EQUAL     [: (*yynode)->equality_operator = pp_equality_expression_rest::op_equal;     :]
    | PP_NOT_EQUAL [: (*yynode)->equality_operator = pp_equality_expression_rest::op_not_equal; :]
   )
   expression=pp_unary_expression
-> pp_equality_expression_rest [
     member variable equality_operator: pp_equality_expression_rest::pp_equality_operator_enum;
] ;;

 ( PP_BANG expression=pp_primary_expression [: (*yynode)->negated = true;  :]
 | expression=pp_primary_expression         [: (*yynode)->negated = false; :]
 )
-> pp_unary_expression [
     member variable negated: bool;
] ;;

 (
   PP_TRUE   [: (*yynode)->type = pp_primary_expression::type_true;           :]
 | PP_FALSE  [: (*yynode)->type = pp_primary_expression::type_false;          :]
 | conditional_symbol=PP_CONDITIONAL_SYMBOL
     [: (*yynode)->type = pp_primary_expression::type_conditional_symbol;     :]
 | PP_LPAREN parenthesis_expression=pp_expression PP_RPAREN
     [: (*yynode)->type = pp_primary_expression::type_parenthesis_expression; :]
 )
-> pp_primary_expression [
     member variable type: pp_primary_expression::pp_primary_expression_type_enum;
] ;;





-----------------------------------------------------------------
-- Code segments copied to the implementation (.cpp) file.
-- If existent, kdevelop-pg's current syntax requires this block
-- to occur at the end of the file.
-----------------------------------------------------------------

[:
#include "csharp.h"
#include "csharp_lexer.h"
#include "csharp_pp_handler_visitor.h"

#include <string>


namespace csharp_pp
{

void parser::tokenize(bool &encountered_eof)
{
  encountered_eof = false;
  int kind = parser::Token_EOF;
  do
    {
      kind = _M_lexer->yylex();
      //std::cerr << "pp: " << lexer.YYText() << std::endl; //" "; // debug output

      parser::token_type &t = this->token_stream->next();
      t.kind = kind;
      t.begin = _M_lexer->token_begin();
      t.end = _M_lexer->token_end();
      t.text = _M_lexer->contents();

      if (kind == parser::Token_EOF)
        {
          t.kind = parser::Token_PP_NEW_LINE;
          encountered_eof = true;
          break;
        }
    }
  while (kind != parser::Token_PP_NEW_LINE);

  parser::token_type &t = this->token_stream->next();
  t.kind = parser::Token_EOF;
  t.begin = _M_lexer->token_begin();
  t.end = _M_lexer->token_end();
  t.text = _M_lexer->contents();

  this->yylex(); // produce the look ahead token
}

void parser::add_token(parser::token_type_enum token_kind)
{
  //std::cerr << "pp: " << lexer.YYText() << std::endl; //" "; // debug output
  parser::token_type &t = this->token_stream->next();
  t.kind = token_kind;
  t.begin = _M_lexer->token_begin();
  t.end = _M_lexer->token_end();
  t.text = _M_lexer->contents();
}


parser::pp_parse_result parser::pp_parse_line(
  parser::token_type_enum first_token, scope* scope, csharp::Lexer *lexer )
{
  // 0) setup
  if (scope == 0)
    return parser::result_invalid;

  _M_scope = scope;
  bool encountered_eof;

  // 1) tokenize
  _M_lexer = lexer;
  add_token(first_token);
  tokenize(encountered_eof);

  // 2) parse
  pp_directive_ast* pp_directive_node = 0;
  bool matched = parse_pp_directive(&pp_directive_node);

  if (matched)
    {
      handler_visitor v(this);
      v.set_scope(_M_scope);
      v.visit_node(pp_directive_node);
    }
  else
    {
      yy_expected_symbol(ast_node::Kind_pp_directive, "pp_directive"); // ### remove me
      return parser::result_invalid;
    }

  if (encountered_eof)
    return parser::result_eof;
  else
    return parser::result_ok;
}


// custom error recovery
void parser::yy_expected_token(int /*expected*/, std::size_t /*where*/, char const *name)
{
  //print_token_environment(this);
  if (_M_scope->csharp_parser() != 0)
    {
      _M_scope->csharp_parser()->report_problem(
        csharp::parser::error,
        std::string("Invalid pre-processor directive: Expected token ``") + name
          //+ "'' instead of ``" + current_token_text
          + "''"
      );
    }
}

void parser::yy_expected_symbol(int /*expected_symbol*/, char const *name)
{
  //print_token_environment(this);
  if (_M_scope->csharp_parser() != 0)
    {
      _M_scope->csharp_parser()->report_problem(
        csharp::parser::error,
        std::string("Invalid pre-processor directive: Expected symbol ``") + name
          //+ "'' instead of ``" + current_token_text
          + "''"
      );
    }
}

} // end of namespace csharp_pp

:]
