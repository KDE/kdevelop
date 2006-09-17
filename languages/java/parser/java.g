-- This file is part of KDevelop.
-- Copyright (c) 2005, 2006 Jakob Petsovits <jpetso@gmx.at>
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


-----------------------------------------------------------------
-- Grammar for Java 1.3, 1.4 or 1.5
-- Modelled after the public domain ANTLR Java 1.5 grammar at
-- http://www.antlr.org/grammar/1090713067533/index.html
-- (Version 1.22.5 from January 03, 2005)
-- and the reference grammar of the Java language specification,
-- Third Edition, at http://java.sun.com/docs/books/jls/.
-----------------------------------------------------------------



-- 10 first/follow conflicts:
--  - The AT conflict in compilation_unit
--    (namely: package_declaration vs. type_declaration).
--    Needs LL(k), solved by lookahead_is_package_declaration().
--    (2 conflicts, which is actually only one)
--  - The COMMA conflict in annotation_element_array_initializer.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in enum_body: greedy is ok.
--    (done right by default, 1 conflict)
--  - The DOT conflict in class_or_interface_type_name:
--    Caused by the may-end-with-epsilon type_arguments. It doesn't apply
--    at all, only kdevelop-pg thinks it does. Code segments...
--    (done right by default, 1 conflict)
--  - The LBRACKET conflict in optional_declarator_brackets:
--    No idea where it stems from, but I think it should be ok.
--    (done right by default, 1 conflict)
--  - The COMMA conflict in type_arguments:
--    the approach for catching ">" signs works this way, and the conflict
--    is resolved by the trailing condition at the end of the rule.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in variable_array_initializer.
--    (manually resolved, 1 conflict)
--  - The LBRACKET conflict in array_creator_rest.
--    (manually resolved, 1 conflict)
--  - The AT conflict in optional_modifiers.
--    (manually resolved, 1 conflict)

-- 12 first/first conflicts:
--  - The IDENTIFIER conflict in annotation_arguments
--    (manually resolved, 1 conflict)
--  - The IDENTIFIER conflicts in *_field,
--    between the method name and variable name
--    (manually resolved, 4 conflicts)
--  - The IDENTIFIER conflict in class_field
--    (manually resolved, 1 conflict)
--  - The STATIC conflict in class_field
--    (manually resolved, 1 conflict)
--  - The SYNCHRONIZED conflict in block_statement.
--    (manually resolved, 1 conflict)
--  - The IDENTIFIER conflict (labels) in embedded_statement
--    (manually resolved, 1 conflict)
--  - The IDENTIFIER conflicts in primary_selector and super_suffix.
--    Could be written without conflict, but done on purpose to tell methods
--    (with possible type arguments) and variables (without those) apart.
--    (manually resolved, 2 identical conflicts)
--  - The LBRACKET conflict in array_creator_rest.
--    This is by design and works as expected.
--    (manually resolved, 1 conflict)

-- Total amount of conflicts: 22



------------------------------------------------------------
-- Global declarations
------------------------------------------------------------

[:
#include <string>
:]



------------------------------------------------------------
-- Parser class members
------------------------------------------------------------

%parserclass (public declaration)
[:
  /**
   * Transform the raw input into tokens.
   * When this method returns, the parser's token stream has been filled
   * and any parse_*() method can be called.
   */
  void tokenize( char *contents );

  /**
   * The compatibility_mode status variable tells which version of Java
   * should be checked against.
   */
  enum java_compatibility_mode {
      java13_compatibility = 130,
      java14_compatibility = 140,
      java15_compatibility = 150
  };

  parser::java_compatibility_mode compatibility_mode();
  void set_compatibility_mode( parser::java_compatibility_mode mode );

  enum problem_type {
      error,
      warning,
      info
  };
  void report_problem( parser::problem_type type, const char* message );
  void report_problem( parser::problem_type type, std::string message );
:]

%parserclass (private declaration)
[:
  parser::java_compatibility_mode _M_compatibility_mode;

  struct parser_state {
      // ltCounter stores the amount of currently open type arguments rules,
      // all of which are beginning with a less than ("<") character.
      // This way, also SIGNED_RSHIFT (">>") and UNSIGNED_RSHIFT (">>>") can be used
      // to close type arguments rules, in addition to GREATER_THAN (">").
      int ltCounter;
  };
  parser_state _M_state;
:]

%parserclass (constructor)
  [: _M_compatibility_mode = java15_compatibility; :]



------------------------------------------------------------
-- Enumeration types for additional AST members
------------------------------------------------------------

%namespace wildcard_type_bounds
[:
  enum extends_or_super_enum {
    extends,
    super
  };
:]

%namespace builtin_type
[:
  enum builtin_type_enum {
    type_void,
    type_boolean,
    type_byte,
    type_char,
    type_short,
    type_int,
    type_float,
    type_long,
    type_double
  };
:]

%namespace switch_label
[:
  enum branch_type_enum {
    case_branch,
    default_branch
  };
:]

%namespace expression
[:
  enum assignment_operator_enum {
    no_assignment,
    op_assign,
    op_plus_assign,
    op_minus_assign,
    op_star_assign,
    op_slash_assign,
    op_bit_and_assign,
    op_bit_or_assign,
    op_bit_xor_assign,
    op_remainder_assign,
    op_lshift_assign,
    op_signed_rshift_assign,
    op_unsigned_rshift_assign
  };
:]

%namespace equality_expression_rest
[:
  enum equality_operator_enum {
    op_equal,
    op_not_equal
  };
:]

%namespace relational_expression_rest
[:
  enum relational_operator_enum {
    op_less_than,
    op_greater_than,
    op_less_equal,
    op_greater_equal
  };
:]

%namespace shift_expression_rest
[:
  enum shift_operator_enum {
    op_lshift,
    op_signed_rshift,
    op_unsigned_rshift
  };
:]

%namespace additive_expression_rest
[:
  enum additive_operator_enum {
    op_plus,
    op_minus
  };
:]

%namespace multiplicative_expression_rest
[:
  enum multiplicative_operator_enum {
    op_star,
    op_slash,
    op_remainder
  };
:]

%namespace unary_expression
[:
  enum unary_expression_enum {
    type_incremented_expression,
    type_decremented_expression,
    type_unary_minus_expression,
    type_unary_plus_expression,
    type_unary_expression_not_plusminus
  };
:]

%namespace unary_expression_not_plusminus
[:
  enum unary_expression_not_plusminus_enum {
    type_bitwise_not_expression,
    type_logical_not_expression,
    type_cast_expression,
    type_primary_expression
  };
:]

%namespace postfix_operator
[:
  enum postfix_operator_enum {
    op_increment,
    op_decrement
  };
:]

%namespace modifiers
[:
  enum modifier_enum {
    mod_private      = 1,
    mod_public       = 1 << 1,
    mod_protected    = 1 << 2,
    mod_static       = 1 << 3,
    mod_transient    = 1 << 4,
    mod_final        = 1 << 5,
    mod_abstract     = 1 << 6,
    mod_native       = 1 << 7,
    mod_synchronized = 1 << 8,
    mod_volatile     = 1 << 9,
    mod_strictfp     = 1 << 10
  };
:]

%namespace literal
[:
  enum literal_type_enum {
    type_true,
    type_false,
    type_null,
    type_integer,
    type_floating_point,
    type_character,
    type_string
  };
:]



------------------------------------------------------------
-- List of defined tokens
------------------------------------------------------------

-- keywords:
%token ABSTRACT ("abstract"), ASSERT ("assert"), BOOLEAN ("boolean"),
       BREAK ("break"), BYTE ("byte"), CASE ("case"), CATCH ("catch"),
       CHAR ("char"), CLASS ("class"), CONST ("const"), CONTINUE ("continue"),
       DEFAULT ("default"), DO ("do"), DOUBLE ("double"), ELSE ("else"),
       ENUM ("enum"), EXTENDS ("extends"), FINAL ("final"),
       FINALLY ("finally"), FLOAT ("float"), FOR ("for"), GOTO ("goto"),
       IF ("if"), IMPLEMENTS ("implements"), IMPORT ("import"),
       INSTANCEOF ("instanceof"), INT ("int"), INTERFACE ("interface"),
       LONG ("long"), NATIVE ("native"), NEW ("new"), PACKAGE ("package"),
       PRIVATE ("private"), PROTECTED ("protected"), PUBLIC ("public"),
       RETURN ("return"), SHORT ("short"), STATIC ("static"),
       STRICTFP ("strictfp"), SUPER ("super"), SWITCH ("switch"),
       SYNCHRONIZED ("synchronized"), THIS ("this"), THROW ("throw"),
       THROWS ("throws"), TRANSIENT ("transient"), TRY ("try"), VOID ("void"),
       VOLATILE ("volatile"), WHILE ("while") ;;

-- seperators:
%token LPAREN ("("), RPAREN (")"), LBRACE ("{"), RBRACE ("}"), LBRACKET ("["),
       RBRACKET ("]"), SEMICOLON (";"), COMMA (","), DOT ("."), AT ("@") ;;

-- operators:
%token ASSIGN ("="), LESS_THAN ("<"), GREATER_THAN (">"), BANG ("!"),
       TILDE ("~"), QUESTION ("?"), COLON (":"), EQUAL ("=="),
       LESS_EQUAL ("<="), GREATER_EQUAL (">="), NOT_EQUAL ("!="),
       LOG_AND ("&&"), LOG_OR ("||"), INCREMENT ("++"), DECREMENT ("--"),
       PLUS ("+"), MINUS ("-"), STAR ("*"), SLASH ("/"), BIT_AND ("&"),
       BIT_OR ("|"), BIT_XOR ("^"), REMAINDER ("%"), LSHIFT ("<<"),
       SIGNED_RSHIFT (">>"), UNSIGNED_RSHIFT (">>>"), PLUS_ASSIGN ("+="),
       MINUS_ASSIGN ("-="), STAR_ASSIGN ("*="), SLASH_ASSIGN ("/="),
       BIT_AND_ASSIGN ("&="), BIT_OR_ASSIGN ("|="), BIT_XOR_ASSIGN ("^="),
       REMAINDER_ASSIGN ("%="), LSHIFT_ASSIGN ("<<="),
       SIGNED_RSHIFT_ASSIGN (">>="), UNSIGNED_RSHIFT_ASSIGN (">>>="),
       ELLIPSIS ("...") ;;

-- literals and identifiers:
%token TRUE ("true"), FALSE ("false"), NULL ("null"),
       INTEGER_LITERAL ("integer literal"),
       FLOATING_POINT_LITERAL ("floating point literal"),
       CHARACTER_LITERAL ("character literal"),
       STRING_LITERAL ("string literal"), IDENTIFIER ("identifier") ;;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;




------------------------------------------------------------
-- Start of the actual grammar
------------------------------------------------------------


   0 [: _M_state.ltCounter = 0; :]
   -- There is a conflict between package_declaration and type_declaration
   -- (both can start with annotations) which requires arbitrary-length LL(k).
   -- The following construct uses backtracking with try/rollback to work
   -- around this issue.
   try/rollback(package_declaration=package_declaration) catch(0)
   try/recover(#import_declaration=import_declaration)*
   try/recover(#type_declaration=type_declaration)*
-> compilation_unit ;;



-- A PACKAGE DECLARATION: optional annotations followed by "package",
--                        then the package identifier.
-- 1 first/first conflict, because annotation as well as "| 0"
-- inside of compilation_unit may both be 0. The ANTLR grammar
-- checks on ?[:annotations "package":] to do a package_declaration.

   try/recover(#annotation=annotation)*
   PACKAGE package_name=qualified_identifier SEMICOLON
-> package_declaration ;;


-- An IMPORT DECLARATION is "import" followed by a package or type (=class) name.

   IMPORT
   (  STATIC [: (*yynode)->static_import = true;  :]
    | 0      [: (*yynode)->static_import = false; :]
   )
   identifier_name=qualified_identifier_with_optional_star SEMICOLON
-> import_declaration [
     member variable static_import: bool;
] ;;


-- A TYPE DECLARATION is either a class, interface, enum or annotation.

   (  modifiers:optional_modifiers
      (  class_declaration=class_declaration[modifiers]
       | enum_declaration=enum_declaration[modifiers]
       | interface_declaration=interface_declaration[modifiers]
       | annotation_type_declaration=annotation_type_declaration[modifiers]
      )
    | SEMICOLON
   )
-> type_declaration ;;




-- ANNOTATIONS look for example like @Info( name="Jakob", born=1983 ),
-- or @Info("Jakob"), or just @Info, and are attached to a method,
-- class, or package. @Info is equivalent to @Info().

   AT type_name=qualified_identifier
   (  LPAREN (args=annotation_arguments | 0) RPAREN
        [: (*yynode)->has_parentheses = true; :]
    | 0 [: (*yynode)->has_parentheses = false; :]
   )
-> annotation [
     member variable has_parentheses: bool;
] ;;

 ( ( ?[: LA(2).kind == Token_ASSIGN :]
     #value_pair=annotation_element_value_pair @ COMMA
   )
 | element_value=annotation_element_value  -- element_name is "value" here
 )
-> annotation_arguments ;;

   element_name=identifier ASSIGN element_value=annotation_element_value
-> annotation_element_value_pair ;;

   cond_expression=conditional_expression
 | annotation=annotation
 | element_array_initializer=annotation_element_array_initializer
-> annotation_element_value ;;

-- Same as annotation_element_value, but array_initializer is excluded.
-- That's because nested annotation array initialisers are not valid.
-- (The Java specification hides that in a short "discussion" area.)

   cond_expression=conditional_expression
 | annotation=annotation
-> annotation_element_array_value ;;

   LBRACE
   (  #element_value=annotation_element_array_value
      ( 0 [: if (LA(2).kind == Token_RBRACE) { break; } :]
        COMMA #element_value=annotation_element_array_value
      )*
    |
      0
   )
   ( COMMA | 0 )
   RBRACE
-> annotation_element_array_initializer ;;




-- Definition of a Java CLASS

   CLASS class_name=identifier
   (  ?[: compatibility_mode() >= java15_compatibility :]
      type_parameters=type_parameters
    | 0
   )  -- in Java 1.5 or higher, it might have type parameters
   (extends=class_extends_clause     | 0)  -- it might have a super class
   (implements=implements_clause     | 0)  -- it might implement some interfaces
   body=class_body
-> class_declaration [
     argument member node modifiers: optional_modifiers;
] ;;


-- Definition of a Java INTERFACE

   INTERFACE interface_name=identifier
   (  ?[: compatibility_mode() >= java15_compatibility :]
      type_parameters=type_parameters
    | 0
   )  -- in Java 1.5 or higher, it might have type parameters
   (extends=interface_extends_clause | 0)  -- it might extend other interfaces
   body=interface_body
-> interface_declaration [
     argument member node modifiers: optional_modifiers;
] ;;


-- Definition of ENUMERATIONs and ANNOTATION TYPEs

   ENUM enum_name=identifier
   (implements=implements_clause     | 0)  -- it might implement some interfaces
   body=enum_body
-> enum_declaration [
     argument member node modifiers: optional_modifiers;
] ;;

   AT INTERFACE annotation_type_name=identifier
   body=annotation_type_body
-> annotation_type_declaration [
     argument member node modifiers: optional_modifiers;
] ;;



-- BODIES of classes, interfaces, annotation types and enums.

   LBRACE try/recover(#declaration=class_field)* RBRACE
-> class_body ;;

   LBRACE try/recover(#declaration=interface_field)* RBRACE
-> interface_body ;;

   LBRACE try/recover(#annotation_type_field=annotation_type_field)* RBRACE
-> annotation_type_body ;;

-- In an enum body, you can have zero or more enum constants
-- followed by any number of fields like a regular class.

   LBRACE
   ( try/recover(#enum_constant=enum_constant)
     @ ( 0 [: if ( LA(2).kind == Token_SEMICOLON
                || LA(2).kind == Token_RBRACE )
              { break; } :] -- if the list is over, then exit the loop
         COMMA
       )
   | 0
   )
   ( COMMA | 0 )
   ( SEMICOLON try/recover(#class_field=class_field)* | 0 )
   RBRACE
-> enum_body ;;

-- An enum constant may have optional parameters and may have a class body

   try/recover(#annotation=annotation)* identifier=identifier
   ( LPAREN arguments=optional_argument_list RPAREN | 0 )
   ( body=enum_constant_body | 0 )
-> enum_constant ;;

   LBRACE try/recover(#declaration=enum_constant_field)* RBRACE
-> enum_constant_body ;;



-- Short CLAUSES used in various declarations

   EXTENDS type=class_or_interface_type_name
-> class_extends_clause ;;

   EXTENDS #type=class_or_interface_type_name @ COMMA
-> interface_extends_clause ;;

   IMPLEMENTS #type=class_or_interface_type_name @ COMMA
-> implements_clause ;;

   THROWS #identifier=qualified_identifier @ COMMA
-> throws_clause ;;




-- Now on to what happens inside the class, interface, etc. bodies:


-- An ANNOTATION TYPE FIELD. The Java specification has a bug here,
-- two different AnnotationTypeElementDeclarations in the book's body
-- and the grammar appendix. I chose the one from the body,
-- which is the same that the ANTLR grammar also uses.

 ( modifiers:optional_modifiers
   (  class_declaration=class_declaration[modifiers]
    | enum_declaration=enum_declaration[modifiers]
    | interface_declaration=interface_declaration[modifiers]
    | annotation_type_declaration=annotation_type_declaration[modifiers]
    |
      type:type
      (                      -- annotation method without arguments:
         ?[: LA(2).kind == Token_LPAREN :] -- resolves the identifier conflict
                                      -- between method name and variable name
         method_declaration=annotation_method_declaration[ modifiers, type ]
       |                     -- or a ConstantDeclaration:
         #variable_declarator:variable_declarator @ COMMA
         SEMICOLON
         constant_declaration=variable_declaration_data[
           modifiers, type, variable_declarator_sequence
         ]
      )
   )
 | SEMICOLON
 )
-> annotation_type_field ;;


-- A CLASS FIELD, representing the various things
-- that can be defined inside a class.

 ( ?[: !(yytoken == Token_STATIC && LA(2).kind == Token_LBRACE) :]
    -- resolves the 'static' conflict, see further down
   modifiers:optional_modifiers
   (  class_declaration=class_declaration[modifiers]
    | enum_declaration=enum_declaration[modifiers]
    | interface_declaration=interface_declaration[modifiers]
    | annotation_type_declaration=annotation_type_declaration[modifiers]
    |
      -- A generic method/ctor has the type_parameters before the return type.
      -- This is not allowed for variable definitions, which is checked later.
      (  ?[: compatibility_mode() >= java15_compatibility :]
         type_parameters:type_parameters
       | 0
      )
      (  -- constructor declaration (without prepended type specification)
         ?[: LA(2).kind == Token_LPAREN :]
         -- resolves the identifier conflict with type
         constructor_declaration=constructor_declaration[
            modifiers, type_parameters
         ]
       |
         -- method or variable declaration
         type:type
         (
            ?[: LA(2).kind == Token_LPAREN :] -- resolves the identifier
                          -- conflict between method name and variable name
            method_declaration=method_declaration[
              modifiers, type_parameters, type
            ]
          |
            ?[: type_parameters == 0 :]
            #variable_declarator:variable_declarator @ COMMA
            SEMICOLON
            variable_declaration=variable_declaration_data[
              modifiers, type, variable_declarator_sequence
            ]
          |
            0 [: report_problem( error,
                   "Expected method declaration after type parameter list" );
               :]
            SEMICOLON -- not really needed, but avoids conflict warnings
         )
      )
   )
 |
   instance_initializer_block=block  -- "{...}" instance initializer
 |
   -- The static class initializer block ("static {...}") has a conflict with
   -- the modifiers from above, which can also be static. A block must always
   -- start with "{", so when encountering static, this can be used to resolve
   -- this conflict. (Lookahead done at the top of the rule.)
   STATIC static_initializer_block=block
 |
   SEMICOLON
 )
-> class_field ;;


-- An ENUM CONSTANT FIELD is just like a class field but without
-- the possibility of a constructor definition or a static initializer.

 ( modifiers:optional_modifiers
   (  class_declaration=class_declaration[modifiers]
    | enum_declaration=enum_declaration[modifiers]
    | interface_declaration=interface_declaration[modifiers]
    | annotation_type_declaration=annotation_type_declaration[modifiers]
    |
      -- A generic method has the type_parameters before the return type.
      -- This is not allowed for variable definitions, which is checked later.
      (  ?[: compatibility_mode() >= java15_compatibility :]
         type_parameters:type_parameters
       | 0
      )
      type:type
      (
         ?[: LA(2).kind == Token_LPAREN :] -- resolves the identifier conflict
                                      -- between method name and variable name
         method_declaration=method_declaration[
           modifiers, type_parameters, type
         ]
       |
         ?[: type_parameters == 0 :]
         #variable_declarator:variable_declarator @ COMMA
         SEMICOLON
         variable_declaration=variable_declaration_data[
           modifiers, type, variable_declarator_sequence
         ]
       |
         0 [: report_problem( error,
                "Expected method declaration after type parameter list" );
            :]
         SEMICOLON -- not really needed, but avoids conflict warnings
      )
   )
 | instance_initializer_block=block  -- "{...}" instance initializer
 | SEMICOLON
 )
-> enum_constant_field ;;


-- An INTERFACE FIELD is the same as an enum constant field but without
-- the possibility of any initializers or method blocks.

 ( modifiers:optional_modifiers
   (  class_declaration=class_declaration[modifiers]
    | enum_declaration=enum_declaration[modifiers]
    | interface_declaration=interface_declaration[modifiers]
    | annotation_type_declaration=annotation_type_declaration[modifiers]
    |
      -- A generic method has the type_parameters before the return type.
      -- This is not allowed for variable definitions, which is checked later.
      0 [: bool has_type_parameters = false; :]
      (  ?[: compatibility_mode() >= java15_compatibility :]
         type_parameters:type_parameters [: has_type_parameters = true; :]
       | 0
      )
      type:type
      (
         ?[: LA(2).kind == Token_LPAREN :] -- resolves the identifier conflict
                                      -- between method name and variable name
         interface_method_declaration=interface_method_declaration[
           modifiers, type_parameters, type
         ]
       |
         ?[: type_parameters == 0 :]
         #variable_declarator:variable_declarator @ COMMA
         SEMICOLON
         variable_declaration=variable_declaration_data[
           modifiers, type, variable_declarator_sequence
         ]
       |
         0 [: report_problem( error,
                "Expected method declaration after type parameter list" );
            :]
         SEMICOLON -- not really needed, but avoids conflict warnings
      )
   )
 | SEMICOLON
 )
-> interface_field ;;


   annotation_name=identifier
   LPAREN RPAREN
   -- declarator_brackets=optional_declarator_brackets -- ANTLR grammar's bug:
   -- It's not in the Java Spec, and obviously has been copied
   -- from classField even if it doesn't belong here.
   (DEFAULT annotation_element_value=annotation_element_value | 0)
   SEMICOLON
-> annotation_method_declaration [
     argument member node modifiers:           optional_modifiers;
     argument member node return_type:         type;
] ;;

   class_name=identifier
   LPAREN parameters=optional_parameter_declaration_list RPAREN
   (throws_clause=throws_clause | 0)
   body=block
   -- leaving out explicit this(...) and super(...) invocations,
   -- these are just normal statements for the grammar
-> constructor_declaration [
     argument member node modifiers:           optional_modifiers;
     argument member node type_parameters:     type_parameters;
] ;;

   method_name=identifier
   LPAREN parameters=optional_parameter_declaration_list RPAREN
   declarator_brackets=optional_declarator_brackets
   (throws_clause=throws_clause | 0)
   (body:block | SEMICOLON)
-> method_declaration [
     argument member node modifiers:           optional_modifiers;
     argument member node type_parameters:     type_parameters;
     argument member node return_type:         type;
] ;;

   method_name=identifier
   LPAREN parameters=optional_parameter_declaration_list RPAREN
   declarator_brackets=optional_declarator_brackets
   (throws_clause=throws_clause | 0)
   SEMICOLON
-> interface_method_declaration [
     argument member node modifiers:           optional_modifiers;
     argument member node type_parameters:     type_parameters;
     argument member node return_type:         type;
] ;;





-- So much for the rough structure, now we get into the details


-- A PARAMETER DECLARATION LIST is part of a method header and can contain
-- zero or more parameters, optionally ending with a variable-length parameter.
-- It's not as hackish as it used to be, nevertheless it could still be nicer.

   0 [: bool ellipsis_occurred = false; :]
   (
      #parameter_declaration=parameter_declaration_ellipsis[&ellipsis_occurred]
      @ ( 0 [: if( ellipsis_occurred == true ) { break; } :]
            -- Don't proceed after the ellipsis. If there's a cleaner way
            -- to exit the loop when ellipsis_occurred == true,
            -- please use that instead of this construct (see below).
          COMMA
        )
    |
      0
   )
-> optional_parameter_declaration_list ;;

-- How it _should_ look:
--
--    0 [: bool ellipsis_occurred = false; :]
--    (
--       #parameter_declaration=parameter_declaration_ellipsis[&ellipsis_occurred]
--       @ ( ?[: ellipsis_occurred == false :] COMMA )
--           -- kdev-pg dismisses this condition!
--     |
--       0
--    )
-- -> optional_parameter_declaration_list ;;

   parameter_modifiers=optional_parameter_modifiers
   type=type
   (  ELLIPSIS [: (*yynode)->has_ellipsis = true; *ellipsis_occurred = true; :]
    | 0        [: (*yynode)->has_ellipsis = false; :]
   )
   variable_name=identifier
   declarator_brackets=optional_declarator_brackets
-> parameter_declaration_ellipsis [
     member variable has_ellipsis: bool;
     argument temporary variable ellipsis_occurred: bool*;
] ;;

   0 [: (*yynode)->has_mod_final = false; :]
   (  FINAL [: (*yynode)->has_mod_final = true; :]
    | try/recover(#mod_annotation=annotation)
   )*
-> optional_parameter_modifiers [
     member variable has_mod_final: bool;
] ;;



-- An OPTIONAL ARGUMENT LIST is used when calling methods
-- (not for declaring them, that's what parameter declaration lists are for).

   try/recover(#expression=expression @ COMMA) | 0
-> optional_argument_list ;;



-- The body of a CONSTRUCTOR METHOD is the same as a normal block, except that
-- it's possible to have a constructor call like this(...) or super(...)
-- at the beginning of the block. This causes a conflict which is difficult
-- to resolve, so class_field uses block instead of constructor_body.
--
--    LBRACE
--    ( try/recover(explicit_constructor_invocation=explicit_constructor_invocation)
--    | 0
--    )
--    try/recover(#statement=embedded_statement)*
--    RBRACE
-- -> constructor_body ;;
--
-- -- Catches obvious constructor calls, but not the expr.super(...) calls:
--
--    (  ?[: compatibility_mode() >= java15_compatibility :]
--       type_arguments=type_arguments
--     | 0
--    )
--    (invocated_constructor=THIS | invocated_constructor=SUPER)
--    LPAREN arguments=argument_list RPAREN SEMICOLON
-- -> explicit_constructor_invocation ;;





-- Type parameters and type arguments, the two rules responsible for the
-- greater-than special casing. (This is the generic aspect in Java >= 1.5.)

-- TYPE PARAMETERS are used in class, interface etc. declarations to
-- determine the generic types allowed as type argument.

   LESS_THAN [: int currentLtLevel = _M_state.ltCounter; _M_state.ltCounter++; :]
   #type_parameter=type_parameter @ COMMA
   (
      type_arguments_or_parameters_end
    | 0  -- they can also be changed by type_parameter or type_argument
   )
   -- make sure we have gobbled up enough '>' characters
   -- if we are at the "top level" of nested type_parameters productions
   [: if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel ) {
        if (!yy_block_errors) {
          report_problem(error, "The amount of closing ``>'' characters is incorrect");
        }
        return false;
      }
   :]
-> type_parameters ;;

   identifier=identifier
   (EXTENDS (#extends_type=class_or_interface_type_name @ BIT_AND) | 0)
-> type_parameter ;;


-- TYPE ARGUMENTS are used in initializers, invocations, etc. to
-- specify the exact types for this generic class/method instance.

   LESS_THAN [: int currentLtLevel = _M_state.ltCounter; _M_state.ltCounter++; :]
   #type_argument=type_argument
   ( -- only proceed when we are at the right nesting level:
     0 [: if( _M_state.ltCounter != currentLtLevel + 1 ) { break; } :]
     COMMA #type_argument=type_argument
   )*
   (
      type_arguments_or_parameters_end
    | 0  -- they can also be changed by type_parameter or type_argument
   )
   -- make sure we have gobbled up enough '>' characters
   -- if we are at the "top level" of nested type_arguments productions
   [: if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel ) {
        if (!yy_block_errors) {
          report_problem(error, "The amount of closing ``>'' characters is incorrect");
        }
        return false;
      }
   :]
-> type_arguments ;;

   LESS_THAN [: int currentLtLevel = _M_state.ltCounter; _M_state.ltCounter++; :]
   #type_argument_type=type_argument_type
   ( -- only proceed when we are at the right nesting level:
     0 [: if( _M_state.ltCounter != currentLtLevel + 1 ) { break; } :]
     COMMA #type_argument_type=type_argument_type
   )*
   (
      type_arguments_or_parameters_end
    | 0  -- they can also be changed by type_parameter or type_argument
   )
   -- make sure we have gobbled up enough '>' characters
   -- if we are at the "top level" of nested type_arguments productions
   [: if (currentLtLevel == 0 && _M_state.ltCounter != currentLtLevel ) {
        if (!yy_block_errors) {
          report_problem(error, "The amount of closing ``>'' characters is incorrect");
        }
        return false;
      }
   :]
-> non_wildcard_type_arguments ;;

   type_argument_type=type_argument_type
 | wildcard_type=wildcard_type
-> type_argument ;;

-- Any type specification except primitive types is allowed as type argument.
-- Arrays of primitive types are allowed nonetheless.

   class_type=class_type
 | mandatory_array_builtin_type=mandatory_array_builtin_type
-> type_argument_type ;;

   QUESTION (bounds=wildcard_type_bounds | 0)
-> wildcard_type ;;

   (  EXTENDS [: (*yynode)->extends_or_super = wildcard_type_bounds::extends; :]
    | SUPER   [: (*yynode)->extends_or_super = wildcard_type_bounds::super;   :]
   )
   type=class_type
-> wildcard_type_bounds [
     member variable extends_or_super: wildcard_type_bounds::extends_or_super_enum;
] ;;


   GREATER_THAN    [: _M_state.ltCounter -= 1; :]  -- ">"
 | SIGNED_RSHIFT   [: _M_state.ltCounter -= 2; :]  -- ">>"
 | UNSIGNED_RSHIFT [: _M_state.ltCounter -= 3; :]  -- ">>>"
-> type_arguments_or_parameters_end ;;





-- And now for the good stuff: statements, expressions and the likes. Yay!

-- This is a BLOCK, a list of statements. It is used in many contexts:
--  - Inside a class definition prefixed with "static" as class initializer
--  - Inside a class definition without "static" as instance initializer
--  - As the body of a method
--  - As a completely independent braced block of code inside a method,
--    starting a new scope for variable definitions

   LBRACE try/recover(#statement=block_statement)* RBRACE
-> block ;;

-- A BLOCK STATEMENT is either an embedded statement, a variable declaration
-- or a type declaration (you know, nested classes and the likes...).

   -- Variable declarations, as well as expression statements, can start with
   -- class1<xxx>.bla or similar. This is only solvable with LL(k), so we need
   -- backtracking in form of the try/rollback() construct.
   try/rollback(variable_declaration_statement=variable_declaration_statement)
   catch(
      -- resolves the SYNCHRONIZED conflict between
      -- synchronized_statement and modifier:
      ?[: (yytoken != Token_SYNCHRONIZED) ||
          (yytoken == Token_SYNCHRONIZED && LA(2).kind == Token_LPAREN)
        :]
      statement=embedded_statement
    |
      -- Inside a block, our four "complex types" can be declared
      -- (enums, nested classes and the likes...):
      modifiers:optional_modifiers
      (  class_declaration=class_declaration[modifiers]
        | enum_declaration=enum_declaration[modifiers]
        | interface_declaration=interface_declaration[modifiers]
        | annotation_type_declaration=annotation_type_declaration[modifiers]
      )
   )
-> block_statement ;;



-- VARIABLE DECLARATIONS, initializers, etc.
-- TODO: the modifiers need to be checked (after parsing) if they contain
--       only the allowed modifiers, which is FINAL and annotations.

   variable_declaration=variable_declaration SEMICOLON
-> variable_declaration_statement ;;

   modifiers:optional_modifiers type:type
   #variable_declarator:variable_declarator @ COMMA
   data=variable_declaration_data[
     modifiers, type, variable_declarator_sequence
   ]
-> variable_declaration ;;

   0
-> variable_declaration_data [
     argument member node modifiers:   optional_modifiers;
     argument member node type:        type;
     argument member node #declarator: variable_declarator;
] ;;

   ( ASSIGN first_initializer=variable_initializer | 0 )
   ( COMMA #variable_declarator=variable_declarator )*
-> variable_declaration_rest ;;

-- A VARIABLE DECLARATOR, as used in a variable_declaration or *_field

   variable_name=identifier
   declarator_brackets=optional_declarator_brackets
   ( ASSIGN initializer=variable_initializer | 0 )
-> variable_declarator ;;

   ( expression=expression | array_initializer=variable_array_initializer )
-> variable_initializer ;;

   LBRACE
   (  ( #variable_initializer=variable_initializer
        ( 0 [: if (LA(2).kind == Token_RBRACE) { break; } :]
          COMMA #variable_initializer=variable_initializer
        )*
        ( COMMA | 0 )
      )
    | 0
   )
   RBRACE
-> variable_array_initializer ;;


-- This PARAMETER DECLARATION rule is not used in optional_parameter_declaration_list,
-- and lacks the ellipsis possibility & handling. It's used in catch_clause
-- and in for_control.

   parameter_modifiers=optional_modifiers
   type=type
   variable_name=identifier
   declarator_brackets=optional_declarator_brackets
-> parameter_declaration ;;


-- This rule, admittedly, runs deep into implementation details.
-- But anyways, it helps transforming the ugly representation of variable
-- declarations in for_control and catch_clause to the standard format. Unity!

   0 [:
     variable_declarator_ast* declarator = create<variable_declarator_ast>();
     declarator->variable_name       = parameter_declaration->variable_name;
     declarator->declarator_brackets = parameter_declaration->declarator_brackets;

     if (rest)
       declarator->initializer         = rest->first_initializer;
     else
       declarator->initializer         = 0;

     declarator_sequence = snoc(declarator_sequence, declarator, memory_pool);

     if (rest && rest->variable_declarator_sequence)
       {
         const list_node<variable_declarator_ast*> *__it
           = rest->variable_declarator_sequence->to_front(), *__end = __it;

         do {
             declarator_sequence = snoc(declarator_sequence, __it->element, memory_pool);
             __it = __it->next;
         } while (__it != __end);
       }
   :]
   data=variable_declaration_data[
     parameter_declaration->parameter_modifiers, parameter_declaration->type,
     declarator_sequence
   ]
-> variable_declaration_split_data [
     argument temporary node parameter_declaration: parameter_declaration;
     argument temporary node rest:                  variable_declaration_rest;
     temporary node #declarator:                    variable_declarator;
] ;;




-- The (embedded) STATEMENT is a central point of the grammar,
-- even if delegating most of the work to its children.

 (
   block=block  -- more block_statements within {} braces
 | assert_statement=assert_statement
 | if_statement=if_statement
 | for_statement=for_statement
 | while_statement=while_statement
 | do_while_statement=do_while_statement
 | try_statement=try_statement
 | switch_statement=switch_statement
 | synchronized_statement=synchronized_statement
 | return_statement=return_statement
 | throw_statement=throw_statement
 | break_statement=break_statement
 | continue_statement=continue_statement
 | SEMICOLON
 |
   ?[: LA(2).kind == Token_COLON :]
   labeled_statement=labeled_statement
 |
   -- method call, assignment, etc.:
   expression_statement=statement_expression SEMICOLON
 )
-> embedded_statement ;;


-- Simple one-rule statements:

   ASSERT condition=expression
   (COLON message=expression | 0) SEMICOLON
-> assert_statement ;;

   IF LPAREN condition=expression RPAREN if_body=embedded_statement
   (ELSE else_body=embedded_statement | 0)
     -- the traditional "dangling-else" conflict:
     -- kdevelop-pg generates proper code here, matching as soon as possible.
-> if_statement ;;

   WHILE LPAREN condition=expression RPAREN body=embedded_statement
-> while_statement ;;

   DO body=embedded_statement
   WHILE LPAREN condition=expression RPAREN SEMICOLON
-> do_while_statement ;;

   SYNCHRONIZED LPAREN locked_type=expression RPAREN
   synchronized_body=block
-> synchronized_statement ;;

   RETURN (return_expression=expression | 0) SEMICOLON
-> return_statement ;;

   THROW exception=expression SEMICOLON
-> throw_statement ;;

   BREAK (label=identifier | 0) SEMICOLON
-> break_statement ;;

   CONTINUE (label=identifier | 0) SEMICOLON
-> continue_statement ;;

   label=identifier COLON statement=embedded_statement
-> labeled_statement ;;


-- The TRY STATEMENT, also known as try/catch/finally block.

   TRY try_body=block
   (  (#catch_clause=catch_clause)+ (FINALLY finally_body=block | 0)
    | FINALLY finally_body=block
   )
-> try_statement ;;

   CATCH LPAREN exception_parameter:parameter_declaration RPAREN
   exception_declaration=variable_declaration_split_data[exception_parameter, 0]
   body=block
-> catch_clause ;;


-- The SWITCH STATEMENT, consisting of a header and multiple
-- "case x:" or "default:" switch statement groups.

   SWITCH LPAREN switch_expression=expression RPAREN
   LBRACE try/recover(#switch_section=switch_section)* RBRACE
-> switch_statement ;;

   (#label=switch_label)+
   try/recover(#statement=block_statement)*
-> switch_section ;;

   (  CASE case_expression=expression
      [: (*yynode)->branch_type = switch_label::case_branch;    :]
    | DEFAULT
      [: (*yynode)->branch_type = switch_label::default_branch; :]
   ) COLON
-> switch_label [
     member variable branch_type: switch_label::branch_type_enum;
] ;;



-- The FOR STATEMENT, including its problematic child for_control.

   FOR LPAREN for_control=for_control RPAREN for_body=embedded_statement
-> for_statement ;;

-- The FOR CONTROL is the three statements inside the for(...) parentheses,
-- or the alternative foreach specifier. It has the same problematic conflict
-- between parameter_declaration and expression that block_statement also has
-- and which is only solvable with arbitrary-length LL(k) and therefore needs
-- backtracking with try/rollback.

 (
   try/rollback(
     vardecl_start_or_foreach_parameter:parameter_declaration  -- "int i"
     (
         -- foreach: int i : intList.values()
         ?[: compatibility_mode() >= java15_compatibility :]
         COLON iterable_expression:expression
         foreach_declaration=foreach_declaration_data[
           vardecl_start_or_foreach_parameter, iterable_expression
         ]
       |
         -- traditional: int i = 0; i < size; i++
         variable_declaration_rest:variable_declaration_rest  -- "= 0"
         variable_declaration=variable_declaration_split_data[
           vardecl_start_or_foreach_parameter, variable_declaration_rest
         ]
         traditional_for_rest=for_clause_traditional_rest  -- "; i < size; i++"
     )
   )
   catch(
     #statement_expression=statement_expression @ COMMA
     traditional_for_rest=for_clause_traditional_rest
   )
 |
   traditional_for_rest=for_clause_traditional_rest  -- only starting with ";"
 )
-> for_control ;;

   SEMICOLON
   (for_condition=expression | 0) SEMICOLON                   -- "i < size;"
   (#for_update_expression=statement_expression @ COMMA | 0)  -- "i++"
-> for_clause_traditional_rest ;;

   0
-> foreach_declaration_data [
     argument member node foreach_parameter:   parameter_declaration;
     argument member node iterable_expression: expression;
] ;;




-- EXPRESSIONS
-- Note that most of these expressions follow the pattern
--   thisLevelExpression :
--     nextHigherPrecedenceExpression @ OPERATOR
--
-- The operators in java have the following precedences:
--  lowest  (13)  Assignment                   = *= /= %= += -= <<= >>= >>>= &= ^= |=
--          (12)  Conditional                  ?:
--          (11)  Conditional OR               ||
--          (10)  Conditional AND              &&
--          ( 9)  Logical OR                   |
--          ( 8)  Logical XOR                  ^
--          ( 7)  Logical AND                  &
--          ( 6)  Equality                     == !=
--          ( 5)  Relational and type-testing  < <= > >=
--          ( 4)  Shift                        << >>
--          ( 3)  Additive                     +(binary) -(binary)
--          ( 2)  Multiplicative               * / %
--          ( 1)  Unary                        ++ -- +(unary) -(unary) ~ ! (type)x
--                Primary                      f(x) x.y a[x]
--  highest       new ()  (explicit parenthesis)
--
-- the last two are not usually on a precedence chart; they are put in
-- to point out that "new" has a higher precedence than ".", so you
-- can validly use
--   new Frame().show()


-- A STATEMENT EXPRESSION may not contain certain subsets of expression,
-- but it's just not feasible for LL(k) parsers to filter them out.

   expression=expression
-> statement_expression ;;


-- So this is the actual EXPRESSION, also known as assignment expression.

   conditional_expression=conditional_expression
   (
      (  ASSIGN
           [: (*yynode)->assignment_operator = expression::op_assign;                :]
       | PLUS_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_plus_assign;           :]
       | MINUS_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_minus_assign;          :]
       | STAR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_star_assign;           :]
       | SLASH_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_slash_assign;          :]
       | BIT_AND_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_and_assign;        :]
       | BIT_OR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_or_assign;         :]
       | BIT_XOR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_xor_assign;        :]
       | REMAINDER_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_remainder_assign;      :]
       | LSHIFT_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_lshift_assign;         :]
       | SIGNED_RSHIFT_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_signed_rshift_assign;   :]
       | UNSIGNED_RSHIFT_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_unsigned_rshift_assign; :]
      )
      assignment_expression=expression
    |
      0 [: (*yynode)->assignment_operator = expression::no_assignment; :]
   )
-> expression [
     member variable assignment_operator: expression::assignment_operator_enum;
] ;;


   logical_or_expression=logical_or_expression
   (  QUESTION if_expression=expression
      COLON    else_expression=conditional_expression
    | 0
   )
-> conditional_expression ;;

   #expression=logical_and_expression @ LOG_OR
-> logical_or_expression ;;

   #expression=bit_or_expression @ LOG_AND
-> logical_and_expression ;;

   #expression=bit_xor_expression @ BIT_OR
-> bit_or_expression ;;

   #expression=bit_and_expression @ BIT_XOR
-> bit_xor_expression ;;

   #expression=equality_expression @ BIT_AND
-> bit_and_expression ;;

   expression=relational_expression
   (#additional_expression=equality_expression_rest)*
-> equality_expression ;;

   (  EQUAL     [: (*yynode)->equality_operator = equality_expression_rest::op_equal;     :]
    | NOT_EQUAL [: (*yynode)->equality_operator = equality_expression_rest::op_not_equal; :]
   )
   expression=relational_expression
-> equality_expression_rest [
     member variable equality_operator: equality_expression_rest::equality_operator_enum;
] ;;

   expression=shift_expression
   (  (#additional_expression=relational_expression_rest)+
    | INSTANCEOF instanceof_type=type
    | 0
   )
-> relational_expression ;;

   (  LESS_THAN     [: (*yynode)->relational_operator = relational_expression_rest::op_less_than;     :]
    | GREATER_THAN  [: (*yynode)->relational_operator = relational_expression_rest::op_greater_than;  :]
    | LESS_EQUAL    [: (*yynode)->relational_operator = relational_expression_rest::op_less_equal;    :]
    | GREATER_EQUAL [: (*yynode)->relational_operator = relational_expression_rest::op_greater_equal; :]
   )
   expression=shift_expression
-> relational_expression_rest [
     member variable relational_operator: relational_expression_rest::relational_operator_enum;
] ;;

   expression=additive_expression
   (#additional_expression=shift_expression_rest)*
-> shift_expression ;;

   (  LSHIFT          [: (*yynode)->shift_operator = shift_expression_rest::op_lshift;          :]
    | SIGNED_RSHIFT   [: (*yynode)->shift_operator = shift_expression_rest::op_signed_rshift;   :]
    | UNSIGNED_RSHIFT [: (*yynode)->shift_operator = shift_expression_rest::op_unsigned_rshift; :]
   )
   expression=additive_expression
-> shift_expression_rest [
     member variable shift_operator: shift_expression_rest::shift_operator_enum;
] ;;

   expression=multiplicative_expression
   (#additional_expression=additive_expression_rest)*
-> additive_expression ;;

   (  PLUS  [: (*yynode)->additive_operator = additive_expression_rest::op_plus;  :]
    | MINUS [: (*yynode)->additive_operator = additive_expression_rest::op_minus; :]
   )
   expression=multiplicative_expression
-> additive_expression_rest [
     member variable additive_operator: additive_expression_rest::additive_operator_enum;
] ;;

   expression=unary_expression
   (#additional_expression=multiplicative_expression_rest)*
-> multiplicative_expression ;;

   (  STAR      [: (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_star;      :]
    | SLASH     [: (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_slash;     :]
    | REMAINDER [: (*yynode)->multiplicative_operator = multiplicative_expression_rest::op_remainder; :]
   )
   expression=unary_expression
-> multiplicative_expression_rest [
     member variable multiplicative_operator: multiplicative_expression_rest::multiplicative_operator_enum;
] ;;


-- The UNARY EXPRESSION and the its not-plusminus part are one rule in the
-- specification, but split apart for better cast_expression lookahead results.

 (
   INCREMENT unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_incremented_expression; :]
 | DECREMENT unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_decremented_expression; :]
 | MINUS unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_unary_minus_expression; :]
 | PLUS  unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_unary_plus_expression;  :]
 | unary_expression_not_plusminus=unary_expression_not_plusminus
     [: (*yynode)->rule_type = unary_expression::type_unary_expression_not_plusminus; :]
 )
-> unary_expression [
     member variable rule_type: unary_expression::unary_expression_enum;
] ;;


-- So, up till now this was the easy stuff. Here comes another sincere
-- conflict in the grammar that can only be solved with LL(k).
-- The conflict in this rule is the ambiguity between type casts (which
-- can be arbitrary class names within parentheses) and primary_expressions,
-- which can also look that way from an LL(1) perspective.
-- Using backtracking with try/rollback solves the problem.

 (
   TILDE bitwise_not_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression_not_plusminus::type_bitwise_not_expression; :]
 | BANG  logical_not_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression_not_plusminus::type_logical_not_expression; :]
 |
   try/rollback(
     cast_expression=cast_expression
       [: (*yynode)->rule_type = unary_expression_not_plusminus::type_cast_expression;      :]
   )
   catch (
     primary_expression=primary_expression (#postfix_operator=postfix_operator)*
       [: (*yynode)->rule_type = unary_expression_not_plusminus::type_primary_expression;   :]
   )
 )
-> unary_expression_not_plusminus [
     member variable rule_type: unary_expression_not_plusminus::unary_expression_not_plusminus_enum;
] ;;

   LPAREN
   (  builtin_type=optional_array_builtin_type RPAREN
      builtin_casted_expression=unary_expression
    |
      class_type=class_type RPAREN
      class_casted_expression=unary_expression_not_plusminus
   )
-> cast_expression ;;

   INCREMENT [: (*yynode)->postfix_operator = postfix_operator::op_increment; :]
 | DECREMENT [: (*yynode)->postfix_operator = postfix_operator::op_decrement; :]
-> postfix_operator [
     member variable postfix_operator: postfix_operator::postfix_operator_enum;
] ;;


-- PRIMARY EXPRESSIONs: qualified names, array expressions,
--                      method invocation, post increment/decrement

   primary_atom=primary_atom (#selector=primary_selector)*
-> primary_expression ;;

-- SELECTORs appended to a primary atom can provide access to ".this" or
-- ".super", create classes with ".new ClassName(...)", call methods with
-- ".methodName(...)", access a member variable with ".variableName",
-- the expression's class info with ".class" and access arrays with "[...]".

 (
   DOT
   (  CLASS class_access=class_access_data -- empty rules, for having
    | THIS this_access=this_access_data    -- a default visitor auto-generated.
    | new_expression=new_expression
    |
      ?[: LA(2).kind != Token_LPAREN :]  -- member variable access
      identifier:identifier           -- (no method call)
      simple_name_access=simple_name_access_data[ identifier ]
    |
      -- method calls (including the "super" ones) may have type arguments
      (  ?[: compatibility_mode() >= java15_compatibility :]
         type_arguments:non_wildcard_type_arguments
       | 0
      )
      (  SUPER super_suffix:super_suffix
         super_access=super_access_data[ type_arguments, super_suffix ]
       |
         identifier:identifier
         LPAREN arguments:optional_argument_list RPAREN
         method_call=method_call_data[ type_arguments, identifier, arguments ]
      )
   )
 |
   array_access=array_access
 )
-> primary_selector ;;

   LBRACKET array_index_expression=expression RBRACKET
-> array_access ;;



-- SUPER SUFFIX: a call to either a constructor, a method or
--               a member variable of the super class.

 (
   LPAREN constructor_arguments=optional_argument_list RPAREN  -- constructor call
 |
   DOT  -- member access
   (  ?[: LA(2).kind != Token_LPAREN :]  -- member variable access (no method call)
      identifier:identifier
      simple_name_access=simple_name_access_data[ identifier ]
    |
      -- method access (looks like super.methodName(...) in the end)
      (  ?[: compatibility_mode() >= java15_compatibility :]
         type_arguments:non_wildcard_type_arguments
       | 0
      )
      identifier:identifier
      LPAREN arguments:optional_argument_list RPAREN
      method_call=method_call_data[ type_arguments, identifier, arguments ]
   )
 )
-> super_suffix ;;



-- PRIMARY ATOM: the basic element of a primary expression,
--               and expressions in general

 (
   literal=literal
 | new_expression=new_expression
 | LPAREN parenthesis_expression=expression RPAREN
 |
   -- stuff like int.class or int[].class
   builtin_type_dot_class=builtin_type_dot_class
 |
   THIS
   (  LPAREN arguments:optional_argument_list RPAREN
      this_call=this_call_data[ 0 /* no type arguments */, arguments ]
    |
      this_access=this_access_data -- empty rule, for having a default visitor
   )
 |
   SUPER super_suffix:super_suffix
   super_access=super_access_data[ 0 /* no type arguments */, super_suffix ]
 |
   ?[: compatibility_mode() >= java15_compatibility :]
   -- generic method invocation with type arguments:
   type_arguments:non_wildcard_type_arguments
   (
      SUPER super_suffix:super_suffix
      super_access=super_access_data[ type_arguments, super_suffix ]
    |
      THIS LPAREN arguments:optional_argument_list RPAREN
      this_call=this_call_data[ type_arguments, arguments ]
    |
      identifier:identifier
      LPAREN arguments:optional_argument_list RPAREN
      method_call=method_call_data[ type_arguments, identifier, arguments ]
   )
 |
   try/rollback(
     -- stuff like narf.zoht[][].class
     array_type_dot_class=array_type_dot_class
   )
   catch (
     -- type names (normal) - either pure or as method
     identifier:identifier
     (
         LPAREN arguments:optional_argument_list RPAREN
         method_call=method_call_data[
           0 /* no type arguments */, identifier, arguments
         ]
       |
         simple_name_access=simple_name_access_data[ identifier ]
     )
   )
 )
-> primary_atom ;;


   builtin_type=optional_array_builtin_type DOT CLASS
-> builtin_type_dot_class ;;

   qualified_identifier=qualified_identifier
   declarator_brackets=mandatory_declarator_brackets DOT CLASS
-> array_type_dot_class ;;


   0
-> method_call_data [
     argument member node type_arguments: non_wildcard_type_arguments;
     argument member node method_name:    identifier;
     argument member node arguments:      optional_argument_list;
] ;;

   0
-> this_call_data [
     argument member node type_arguments: non_wildcard_type_arguments;
     argument member node arguments:      optional_argument_list;
] ;;

   0
-> this_access_data ;; -- probably the emptiest rule in the whole grammar ;)
   -- but kdev-pg creates a default visitor method, and that's why it's there

   0
-> class_access_data ;; -- hm maybe this rule is equally empty...

   0
-> super_access_data [
     argument member node type_arguments: non_wildcard_type_arguments;
     argument member node super_suffix:   super_suffix;
] ;;

   0
-> simple_name_access_data [
     argument member node name:           identifier;
] ;;




-- NEW EXPRESSIONs are allocations of new types or arrays.

   NEW
   (  ?[: compatibility_mode() >= java15_compatibility :]
      type_arguments=non_wildcard_type_arguments
    | 0
   )
   type=non_array_type
   (  LPAREN class_constructor_arguments=optional_argument_list RPAREN
      (class_body=class_body | 0)
    |
      array_creator_rest=array_creator_rest
   )
-> new_expression ;;

-- This array creator rest can be either
-- a.) empty brackets with an optional initializer (e.g. "[][]{exp,exp}") or
-- b.) at least one filled bracket, afterwards any amount of empty ones

 (
   ?[: LA(2).kind == Token_RBRACKET :]
   mandatory_declarator_brackets=mandatory_declarator_brackets
   array_initializer=variable_array_initializer
 |
   LBRACKET #index_expression=expression RBRACKET
   ( 0 [: if (LA(2).kind == Token_RBRACKET) { break; } :]
        -- exit the loop when noticing declarator brackets
     LBRACKET #index_expression=expression RBRACKET
   )*
   optional_declarator_brackets=optional_declarator_brackets
 )
-> array_creator_rest ;;





-- All kinds of rules for types here.

-- A TYPE is a type name with optionally appended brackets
-- (which would make it an array type).

   class_type=class_type
 | builtin_type=optional_array_builtin_type
-> type ;;

   type=class_or_interface_type_name
   declarator_brackets=optional_declarator_brackets
-> class_type ;;

   type=builtin_type declarator_brackets=optional_declarator_brackets
-> optional_array_builtin_type ;;

   type=builtin_type declarator_brackets=mandatory_declarator_brackets
-> mandatory_array_builtin_type ;;


-- A NON-ARRAY TYPE is just a type name, without appended brackets

   class_or_interface_type=class_or_interface_type_name
 | builtin_type=builtin_type
-> non_array_type ;;

-- The primitive types. The Java specification doesn't include void here,
-- but the ANTLR grammar works that way, and so does this one.

   VOID    [: (*yynode)->type = builtin_type::type_void;    :]
 | BOOLEAN [: (*yynode)->type = builtin_type::type_boolean; :]
 | BYTE    [: (*yynode)->type = builtin_type::type_byte;    :]
 | CHAR    [: (*yynode)->type = builtin_type::type_char;    :]
 | SHORT   [: (*yynode)->type = builtin_type::type_short;   :]
 | INT     [: (*yynode)->type = builtin_type::type_int;     :]
 | FLOAT   [: (*yynode)->type = builtin_type::type_float;   :]
 | LONG    [: (*yynode)->type = builtin_type::type_long;    :]
 | DOUBLE  [: (*yynode)->type = builtin_type::type_double;  :]
-> builtin_type [
     member variable type: builtin_type::builtin_type_enum;
] ;;

   #part=class_or_interface_type_name_part @ DOT
-> class_or_interface_type_name ;;

   identifier=identifier
   (  ?[: compatibility_mode() >= java15_compatibility :]
      type_arguments=type_arguments
    | 0
   )
-> class_or_interface_type_name_part ;;



-- QUALIFIED identifiers are either qualified ones or raw identifiers.

   #name=identifier @ DOT
-> qualified_identifier ;;

   #name=identifier [: (*yynode)->has_star = false; :]
   ( DOT (  #name=identifier
          | STAR    [: (*yynode)->has_star = true; break; :]
                       -- break -> no more identifiers after the star
         )
   )*
-> qualified_identifier_with_optional_star [
     member variable has_star: bool;
] ;;

-- Declarator brackets are part of a type specification, like String[][].
-- They are always empty, only have to be counted.

   ( LBRACKET RBRACKET [: (*yynode)->bracket_count++; :] )*
-> optional_declarator_brackets [
     member variable bracket_count: int;
] ;;

   ( LBRACKET RBRACKET [: (*yynode)->bracket_count++; :] )+
-> mandatory_declarator_brackets [
     member variable bracket_count: int;
] ;;




-- MODIFIERs for Java classes, interfaces, class/instance vars and methods.
-- Sometimes not all of them are valid, but that has to be checked manually
-- after running the parser. The ANTLR grammar also does it this way.
-- All the occurring modifiers are stored together in the "modifiers"
-- AST node member as flags, except for the annotations who get their own list.

 (
   PRIVATE      [: (*yynode)->modifiers |= modifiers::mod_private;      :]
 | PUBLIC       [: (*yynode)->modifiers |= modifiers::mod_public;       :]
 | PROTECTED    [: (*yynode)->modifiers |= modifiers::mod_protected;    :]
 | STATIC       [: (*yynode)->modifiers |= modifiers::mod_static;       :]
 | TRANSIENT    [: (*yynode)->modifiers |= modifiers::mod_transient;    :]
 | FINAL        [: (*yynode)->modifiers |= modifiers::mod_final;        :]
 | ABSTRACT     [: (*yynode)->modifiers |= modifiers::mod_abstract;     :]
 | NATIVE       [: (*yynode)->modifiers |= modifiers::mod_native;       :]
 -- Neither in the Java spec nor in the JavaCC grammar, just in the ANTLR one:
 -- | mod_threadsafe=THREADSAFE
 | SYNCHRONIZED [: (*yynode)->modifiers |= modifiers::mod_synchronized; :]
 | VOLATILE     [: (*yynode)->modifiers |= modifiers::mod_volatile;     :]
 | STRICTFP     [: (*yynode)->modifiers |= modifiers::mod_strictfp;     :]
 |
 -- A modifier may be any annotation (e.g. @bla), but not @interface.
 -- This condition resolves the conflict between modifiers
 -- and annotation type declarations:
   0 [: if (yytoken == Token_AT && LA(2).kind == Token_INTERFACE) { break; } :]
   try/recover(#mod_annotation=annotation)
 )*
-> optional_modifiers [
     member variable modifiers: unsigned int;
] ;;



   ident=IDENTIFIER
-> identifier ;;

 (
   TRUE   [: (*yynode)->literal_type = literal::type_true;  :]
 | FALSE  [: (*yynode)->literal_type = literal::type_false; :]
 | NULL   [: (*yynode)->literal_type = literal::type_null;  :]
 |
   integer_literal=INTEGER_LITERAL
   [: (*yynode)->literal_type = literal::type_integer;  :]
 |
   floating_point_literal=FLOATING_POINT_LITERAL
   [: (*yynode)->literal_type = literal::type_floating_point;  :]
 |
   character_literal=CHARACTER_LITERAL
   [: (*yynode)->literal_type = literal::type_character;  :]
 |
   string_literal=STRING_LITERAL
   [: (*yynode)->literal_type = literal::type_string;  :]
 )
-> literal [
     member variable literal_type: literal::literal_type_enum;
] ;;







-----------------------------------------------------------------
-- Code segments copied to the implementation (.cpp) file.
-- If existent, kdevelop-pg's current syntax requires this block
-- to occur at the end of the file.
-----------------------------------------------------------------

[:
#include "java_lexer.h"


namespace java
{

void parser::tokenize( char *contents )
{
    Lexer lexer( this, contents );

    int kind = parser::Token_EOF;
    do
    {
        kind = lexer.yylex();
        //std::cerr << lexer.YYText() << std::endl; //" "; // debug output

        if ( !kind ) // when the lexer returns 0, the end of file is reached
            kind = parser::Token_EOF;

        parser::token_type &t = this->token_stream->next();
        t.kind = kind;
        t.begin = lexer.tokenBegin();
        t.end = lexer.tokenEnd();
        t.text = contents;
    }
    while ( kind != parser::Token_EOF );

    this->yylex(); // produce the look ahead token
}

parser::java_compatibility_mode parser::compatibility_mode()
{
    return _M_compatibility_mode;
}
void parser::set_compatibility_mode( parser::java_compatibility_mode mode )
{
    _M_compatibility_mode = mode;
}


parser::parser_state *parser::copy_current_state()
{
    parser_state *state = new parser_state();
    state->ltCounter = _M_state.ltCounter;
    return state;
}

void parser::restore_state( parser::parser_state *state )
{
    _M_state.ltCounter = state->ltCounter;
}

} // end of namespace java

:]
