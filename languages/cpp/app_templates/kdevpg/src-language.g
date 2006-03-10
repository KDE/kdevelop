-- A kdevelop-pg grammar for %{LANGUAGE}

-- Like the lexer file %{LANGUAGE}_lexer.ll, this grammar holds some examples
-- that you may study, adapt or delete, depending on your needs.


-- At the very beginning of the grammar file, you can have a C++ code block
-- which will be inserted into the generated %{LANGUAGE}.h file.
-- That way you can for example declare parser-specific enumerations
-- and other stuff that should be declared.
-- (This comes before the the parser class is declared.)
-- Here's a quite useless enumeration to demonstrate how it's done:

[:
enum LiteralType {
  StringLiteral,
  CharacterLiteral,
  IntegerLiteral,
};
:]


-- Each token that appears in %{LANGUAGE}_lexer.ll has to be declared here.

-- Seperators:
%token LPAREN ("("), RPAREN (")"), LBRACE ("{"), RBRACE ("}"), LBRACKET ("["),
       RBRACKET ("]"), COMMA (","), SEMICOLON (";"), DOT (".") ;;

-- Operators:
%token QUESTION ("?"), COLON (":"), BANG ("!"), EQUAL ("=="), LESS_THAN ("<"),
       LESS_EQUAL ("<="), GREATER_THAN (">"), GREATER_EQUAL (">="),
       NOT_EQUAL ("!="), LOG_AND ("&&"), LOG_OR ("||"), ASSIGN ("="),
       PLUS ("+"), MINUS ("-"), STAR ("*"), SLASH ("/"), REMAINDER ("%"),
       ELLIPSIS ("...") ;;

-- Keywords:
%token ABSTRACT ("abstract"), BOOLEAN ("boolean"), BREAK ("break"),
       -- ...and so on...
       TRUE ("true"), VOID ("void"), WHILE ("while") ;;

-- Literals and identifiers:
%token IDENTIFIER ("identifier"), CHARACTER_LITERAL ("character literal"),
       STRING_LITERAL ("string literal"),
       INTEGER_LITERAL ("integer literal") ;;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;



-- The actual grammar starts here.
-- (Replace those example rules with your own ones.)


-- If you have lists of items (like "(statement)*", or "(statement)+") and want
-- to keep them, you store them in a list which is named "stmt" here. If you
-- just want to store single elements instead of lists, leave out the "#" sign.

   ( #stmt=statement )*
-> %{STARTRULE} ;;


-- This statement rule contains an easy-to-resolve first/first conflict between
-- the function_call rule and the assignment rule (to show how it can be resolved).
-- The problem is that both can start with an IDENTIFIER token, so it's not
-- clear which rule should be used. (LL(k) parsers, in opposition to LALR(1)
-- parsers like yacc, have to decide it _now_, they can't wait until enough
-- tokens have been read to easily resolve this.)
-- We know that the second token of a function call (like "doIt(bla, bla)")
-- is always an LPAREN, so let's check on that to tell function calls and
-- assignments apart.

   (  ?[: LA(2).kind == Token_LPAREN :]
      function_call=function_call
    | assignment=assignment
    | 0  -- this is also called the epsilon rule and means "empty" (no tokens)
         -- so, with "(umbrella | 0)" you have an optional umbrella.
   )
   SEMICOLON
-> statement ;;


-- The "@" (postfix) operator is quite handy for things like parameter lists.
-- "foo @ bar" directly translates to "foo (bar foo)*".

   function_name=identifier
   LPAREN
   (variable_name=identifier | literal=literal) @ COMMA
   RPAREN
-> function_call ;;


-- As a side note: remember that everything of importance is stored with an
-- annotation (variablename=rulename) while unimportant things are not
-- annotated. For example, if there is an assignment rule like this, we don't
-- need to store the ASSIGN token as we know that it has to be there and how
-- it looks like.

   variable_name=identifier ASSIGN value=literal
-> assignment ;;


-- It might be a good idea to keep the tokens containing values
-- in a seperate rule, it makes post-processing the parse tree a bit easier.

   ident=IDENTIFIER
-> identifier ;;


-- After each token, you can place C++ code that is executed when the token
-- is actually found while parsing. Let's say we want to print out a message
-- every time we find a literal in the input (given that it fits into the syntax, otherwise this rule won't
-- be processed at all).

   string_literal=STRING_LITERAL        [: literalFound(StringLiteral); :]
 | integer_literal=INTEGER_LITERAL      [: literalFound(IntegerLiteral); :]
 | character_literal=CHARACTER_LITERAL  [: literalFound(CharacterLiteral); :]
-> literal ;;



-- So much for the grammar. At the end of the grammar file, you can have
-- a code block similar to the one at the beginning, only that this one
-- will be inserted into %{LANGUAGE}.cpp instead of %{LANGUAGE}.h.
-- After all, you want to have your literalFound() function defined somewhere.

[:
#include <iostream>

void literalFound(LiteralType type)
{
    if (type == StringLiteral) {
      std::cout << "Found a string literal..." << std::endl;
    }
    else if (type == CharacterLiteral) {
      std::cout << "Found a character literal..." << std::endl;
    }
    else {
      std::cout << "Found another literal..." << std::endl;
    }
}
:]


-- Ok, enough with explaining, now it's your turn to take action - say,
-- to build the parser, inspect the lexer file, or maybe take a little break.
