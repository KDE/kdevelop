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
-- Grammar for C# 2.0
-- Modelled after the reference grammar of the C# 2.0 language specification
-- (ECMA-334, Third Edition from June 2005, available at
-- http://www.ecma-international.org/publications/standards/Ecma-334.htm),
-- and the BSD-licensed ANTLR C# 1.0 grammar "kcsparse" at
-- http://antlr.org/grammar/list (version from December 01, 2005)
-----------------------------------------------------------------------------


-- 18 first/follow conflicts:
--  - The EXTERN conflicts in compilation_unit. They would be gone if
--    type_declaration used optional_type_modifiers instead of
--    broader optional_modifiers, but we stick with the latter one in order
--    to improve the AST. As the extern_alias_directive comes first, it
--    rightfully gets selected, and so the conflict is harmless.
--    (done right by default, 2 conflicts)
--  - The LBRACKET conflict in compilation_unit,
--    and the following EXTERN, LBRACKET conflict there.
--    LBRACKET is resolved, EXTERN is just the above harmless conflict again.
--    (manually resolved, 2 conflicts)
--  - The COMMA conflict in global_attribute_section. Easy.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in attribute_section, exactly the same one as above.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in attribute_arguments: greedy is ok.
--    (done right by default, 1 conflict)
--  - The COMMA conflict in enum_body, also similar to the above.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in type_arguments:
--    the approach for catching ">" signs works this way, and the conflict
--    is resolved by the trailing condition at the end of the rule.
--    (manually resolved, 1 conflict)
--  - The COMMA conflict in array_initializer, another one of those.
--    (manually resolved, 1 conflict)
--  - The BIT_AND conflict in bit_and_expression,
--    the PLUS, MINUS conflict in additive_expression,
--    the STAR conflict in multiplicative_expression, and
--    the LPAREN, INCREMENT, DECREMENT conflict in primary_expression.
--    They originate in the rather hackish solution to make expressions like
--    "a is sometype ? if_exp : else_exp" work (see conditional_expression)
--    and can be ignored, because the condition does the necessary check.
--    (manually resolved, 4 conflicts)
--  - The LBRACKET conflict in array_creation_expression_rest, for which
--    new_expression with its array_creation_expression part is to blame.
--    Caused by the fact that array_creation_expression can't be seperated
--    from primary_atom and put into primary_expression instead.
--    (manually resolved, 1 conflict)
--  - The LBRACKET, STAR conflict in unmanaged_type, similar to the one
--    in array_creation_expression, only that it's triggered by the
--    rank specifiers instead. The LBRACKET conflict is caused by the fact
--    that array_creation_expression can't be seperated.
--    As a consequence, all rank specifiers are checked for safety, always.
--    The star conflict is caused by the may-end-with-epsilon type_arguments.
--    It doesn't apply at all, only kdevelop-pg thinks it does. Code segments...
--    (manually resolved, 1 conflict)
--  - The DOT conflict in namespace_or_type_name:
--    Caused by the may-end-with-epsilon type_arguments. It doesn't apply
--    at all, only kdevelop-pg thinks it does. Code segments...
--    (done right by default, 1 conflict)
--  - The DOT conflict in namespace_or_type_name_safe,
--    which actually stems from indexer_declaration.
--    (manually resolved, 1 conflict)

-- 14 first/first conflicts:
--  - The ADD, ALIAS, etc. (identifier) conflict in using_directive.
--    (manually resolved, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflicts in attribute_arguments,
--    two similar ones.
--    (manually resolved, 2 conflicts)
--  - The PARTIAL conflict in class_or_struct_member_declaration,
--    between type_declaration_rest and identifier.
--    (manually resolved, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflict
--    in class_or_struct_member_declaration, between constructor_declaration
--    and the (type ...) part of the rule.
--    (manually resolved, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflict
--    in class_or_struct_member_declaration (another one), between
--    the field declaration and the (type_name_safe ...) part of the subrule.
--    (manually resolved, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflict in event_declaration,
--    between variable_declarator and type_name.
--    (manually resolved, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflict
--    in type_parameter_constraints, caused by the similarity of
--    primary_or_secondary_constraint (with class_type) and
--    secondary_constraints (with type_name). Not resolved, instead,
--    primary_or_secondary_constraint may be both, as indicated by the name.
--    (done right by default, 1 conflict)
--  - The ADD, ALIAS, etc. (identifier) conflict in block_statement
--    between all three statement types. labeled_statement vs. the other two
--    is resolved easily, whereas local_variable_declaration_statement
--    vs. embedded_statement needs arbitrary-length LL(k), and is solved
--    with a try/rollback() block.
--    (manually resolved, 1 conflict)
--  - The CHECKED, UNCHECKED, YIELD conflict in embedded_statement.
--    For "checked" and "unchecked", this is because there are both blocks
--    and expression statements starting with "(un)checked". For "yield",
--    this is because "yield" is not only the start of yield_statement,
--    but also a non-keyword identifier, and as such needs special treatment.
--    Seperate LA(2) comparisons for all three of them.
--    (manually resolved, 1 conflict)
--  - The CATCH conflict in catch_clauses.
--    (manually resolved, 1 conflict)
--  - The VOID conflict in typeof_expression.
--    (manually resolved, 1 conflict)
--  - The BOOL, BYTE, CHAR, etc. conflict in typeof_expression.
--    That conflict is only naturally, because there's the same "type" rule
--    in two branches of an alternative item. One for C# 2.0 or higher,
--    and another one for C# 1.0. So, this is an artificial conflict.
--    (manually resolved, 1 conflict)
--  - The VOID conflict in return_type.
--    (manually resolved, 1 conflict)

-- Total amount of conflicts: 32



------------------------------------------------------------
-- Global declarations
------------------------------------------------------------

[:
#include <string>
#include <set>

namespace csharp_pp
{
  class handler_visitor;
}
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
   * The compatibility_mode status variable tells which version of C#
   * should be checked against.
   */
  enum csharp_compatibility_mode {
      csharp10_compatibility = 100,
      csharp20_compatibility = 200
  };
  parser::csharp_compatibility_mode compatibility_mode();
  void set_compatibility_mode( parser::csharp_compatibility_mode mode );

  void pp_define_symbol( std::string symbol_name );

  enum problem_type {
      error,
      warning,
      info
  };
  void report_problem( parser::problem_type type, const char* message );
  void report_problem( parser::problem_type type, std::string message );
:]

%parserclass (protected declaration)
[:
  friend class ::csharp_pp::handler_visitor; // calls the pp_*() methods

  /** Called when an #error or #warning directive has been found.
   *  @param type   Either parser::error or parser::warning.
   *  @param label  The error/warning text.
   */
  virtual void pp_diagnostic( parser::problem_type /*type*/, std::string /*message*/ ) {}
  virtual void pp_diagnostic( parser::problem_type /*type*/ ) {}
:]

%parserclass (private declaration)
[:
  void pp_undefine_symbol( std::string symbol_name );
  bool pp_is_symbol_defined( std::string symbol_name );

  parser::csharp_compatibility_mode _M_compatibility_mode;
  std::set<std::string> _M_pp_defined_symbols;

  struct parser_state {
      // ltCounter stores the amount of currently open type arguments rules,
      // all of which are beginning with a less than ("<") character.
      // This way, also RSHIFT (">>") can be used to close type arguments rules,
      // in addition to GREATER_THAN (">").
      int ltCounter;
  };
  parser_state _M_state;

  // Rather hackish solution for recognizing expressions like
  // "a is sometype ? if_exp : else_exp", see conditional_expression.
  bool is_nullable_type( type_ast *type );
  void unset_nullable_type( type_ast *type );
  type_ast *last_relational_expression_rest_type(
      null_coalescing_expression_ast *null_coalescing_expression );
:]

%parserclass (constructor)
[:
  _M_compatibility_mode = csharp20_compatibility;
:]



------------------------------------------------------------
-- Enumeration types for additional AST members
------------------------------------------------------------

%namespace access_policy
[:
  enum access_policy_enum {
    access_private = 0, // default value: memory pool initializes everything with zeros
    access_protected,
    access_protected_internal,
    access_internal,
    access_public
  };
:]

%namespace modifiers
[:
  enum modifier_enum {
    mod_new          = 1,
    mod_abstract     = 1 << 1,
    mod_sealed       = 1 << 2,
    mod_static       = 1 << 3,
    mod_readonly     = 1 << 4,
    mod_volatile     = 1 << 5,
    mod_virtual      = 1 << 6,
    mod_override     = 1 << 7,
    mod_extern       = 1 << 8,
    mod_unsafe       = 1 << 9,
    mod_fixed        = 1 << 10
  };
:]

%namespace conversion_operator_declaration
[:
  enum conversion_type_enum {
    conversion_implicit,
    conversion_explicit
  };
:]

%namespace constructor_initializer
[:
  enum constructor_initializer_type_enum {
    type_base,
    type_this
  };
:]

%namespace overloadable_operator
[:
  enum unary_or_binary_enum {
    type_unary,
    type_binary
  };

  enum overloadable_operator_enum
  {
    // overloadable unary operators:
    op_bang,
    op_tilde,
    op_increment,
    op_decrement,
    op_true,
    op_false,
    // overloadable unary or binary operators:
    op_plus,
    op_minus,
    // overloadable binary operators:
    op_star,
    op_slash,
    op_remainder,
    op_bit_and,
    op_bit_or,
    op_bit_xor,
    op_lshift,
    op_rshift,
    op_equal,
    op_not_equal,
    op_greater_than,
    op_less_than,
    op_greater_equal,
    op_less_equal
  };
:]

%namespace accessor_declarations
[:
  enum accessor_type_enum {
    type_get,
    type_set,
    type_none // only possible for the second, optional accessor
  };
:]

%namespace primary_or_secondary_constraint
[:
  enum primary_or_secondary_constraint_enum {
    type_type,
    type_class,
    type_struct
  };
:]

%namespace argument
[:
  enum argument_type_enum {
    type_value_parameter,
    type_reference_parameter,
    type_output_parameter
  };
:]

%namespace goto_statement
[:
  enum goto_statement_enum {
    type_labeled_statement,
    type_switch_case,
    type_switch_default
  };
:]

%namespace yield_statement
[:
  enum yield_statement_enum {
    type_yield_return,
    type_yield_break
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
    op_remainder_assign,
    op_bit_and_assign,
    op_bit_or_assign,
    op_bit_xor_assign,
    op_lshift_assign,
    op_rshift_assign
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
    op_greater_equal,
    op_is,
    op_as
  };
:]

%namespace shift_expression_rest
[:
  enum shift_operator_enum {
    op_lshift,
    op_rshift
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
    type_bitwise_not_expression,
    type_logical_not_expression,
    type_cast_expression,
    type_primary_expression,
    type_pointer_indirection_expression,
    type_addressof_expression
  };
:]

%namespace primary_suffix
[:
  enum primary_suffix_enum {
    type_member_access,
    type_pointer_member_access,
    type_invocation,
    type_element_access,
    type_increment,
    type_decrement
  };
:]

%namespace primary_atom
[:
  enum primary_atom_enum {
    type_literal,
    type_parenthesized_expression,
    type_member_access,
    type_this_access,
    type_base_access,
    type_new_expression,
    type_typeof_expression,
    type_checked_expression,
    type_unchecked_expression,
    type_default_value_expression,
    type_anonymous_method_expression,
    type_sizeof_expression
  };
:]

%namespace predefined_type
[:
  enum predefined_type_enum {
    type_bool,
    type_byte,
    type_char,
    type_decimal,
    type_double,
    type_float,
    type_int,
    type_long,
    type_object,
    type_sbyte,
    type_short,
    type_string,
    type_uint,
    type_ulong,
    type_ushort
  };
:]

%namespace base_access
[:
  enum base_access_enum {
    type_base_member_access,
    type_base_indexer_access
  };
:]

%namespace typeof_expression
[:
  enum typeof_expression_enum {
    type_void,
    type_unbound_type_name,
    type_type
  };
:]

%namespace return_type
[:
  enum return_type_enum {
    type_regular,
    type_void
  };
:]

%namespace pointer_type
[:
  enum pointer_type_enum {
    type_regular,
    type_void_star
  };
:]

%namespace unmanaged_type_suffix
[:
  enum suffix_type {
    type_star,
    type_rank_specifier
  };
:]

%namespace builtin_class_type
[:
  enum builtin_class_type_enum {
    type_object,
    type_string
  };
:]

%namespace simple_type
[:
  enum simple_type_enum {
    type_numeric,
    type_bool
  };
:]

%namespace numeric_type
[:
  enum numeric_type_enum {
    type_integral,
    type_floating_point,
    type_decimal
  };
:]

%namespace integral_type
[:
  enum integral_type_enum {
    type_sbyte,
    type_byte,
    type_short,
    type_ushort,
    type_int,
    type_uint,
    type_long,
    type_ulong,
    type_char
  };
:]

%namespace floating_point_type
[:
  enum floating_point_type_enum {
    type_float,
    type_double
  };
:]

%namespace parameter
[:
  enum parameter_type_enum {
    value_parameter,
    reference_parameter,
    output_parameter
  };
:]

%namespace literal
[:
  enum literal_type_enum {
    type_true,
    type_false,
    type_null,
    type_integer,
    type_real,
    type_character,
    type_string
  };
:]



------------------------------------------------------------
-- List of defined tokens
------------------------------------------------------------

-- keywords:
%token ABSTRACT ("abstract"), AS ("as"), BASE ("base"), BOOL ("bool"),
       BREAK ("break"), BYTE ("byte"), CASE ("case"), CATCH ("catch"),
       CHAR ("char"), CHECKED ("checked"), CLASS ("class"), CONST ("const"),
       CONTINUE ("continue"), DECIMAL ("decimal"), DEFAULT ("default"),
       DELEGATE ("delegate"), DO ("do"), DOUBLE ("double"), ELSE ("else"),
       ENUM ("enum"), EVENT ("event"), EXPLICIT ("explicit"),
       EXTERN ("extern"), FINALLY ("finally"), FIXED ("fixed"),
       FLOAT ("float"), FOREACH ("foreach"), FOR ("for"), GOTO ("goto"),
       IF ("if"), IMPLICIT ("implicit"), IN ("in"), INT ("int"),
       INTERFACE ("interface"), INTERNAL ("internal"), IS ("is"),
       LOCK ("lock"), LONG ("long"), NAMESPACE ("namespace"), NEW ("new"),
       OBJECT ("object"), OPERATOR ("operator"), OUT ("out"),
       OVERRIDE ("override"), PARAMS ("params"), PRIVATE ("private"),
       PROTECTED ("protected"), PUBLIC ("public"), READONLY ("readonly"),
       REF ("ref"), RETURN ("return"), SBYTE ("sbyte"), SEALED ("sealed"),
       SHORT ("short"), SIZEOF ("sizeof"), STACKALLOC ("stackalloc"),
       STATIC ("static"), STRING ("string"), STRUCT ("struct"),
       SWITCH ("switch"), THIS ("this"), THROW ("throw"), TRY ("try"),
       TYPEOF ("typeof"), UINT ("uint"), ULONG ("ulong"),
       UNCHECKED ("unchecked"), UNSAFE ("unsafe"), USHORT ("ushort"),
       USING ("using"), VIRTUAL ("virtual"), VOID ("void"),
       VOLATILE ("volatile"), WHILE ("while") ;;

-- non-keyword identifiers with special meaning in the grammar:
%token ADD ("add"), ALIAS("alias"), GET ("get"), GLOBAL ("global"),
       PARTIAL ("partial"), REMOVE ("remove"), SET ("set"), VALUE ("value"),
       WHERE ("where"), YIELD ("yield"), ASSEMBLY ("assembly") ;;

-- seperators:
%token LPAREN ("("), RPAREN (")"), LBRACE ("{"), RBRACE ("}"), LBRACKET ("["),
       RBRACKET ("]"), COMMA (","), SEMICOLON (";"), DOT (".") ;;

-- operators:
%token COLON (":"), SCOPE ("::"), QUESTION ("?"), QUESTIONQUESTION ("??"),
       BANG ("!"), TILDE ("~"), EQUAL ("=="), LESS_THAN ("<"),
       LESS_EQUAL ("<="), GREATER_THAN (">"), GREATER_EQUAL (">="),
       NOT_EQUAL ("!="), LOG_AND ("&&"), LOG_OR ("||"), ARROW_RIGHT ("->"),
       INCREMENT ("++"), DECREMENT ("--"), ASSIGN ("="), PLUS ("+"),
       PLUS_ASSIGN ("+="), MINUS ("-"), MINUS_ASSIGN ("-="), STAR ("*"),
       STAR_ASSIGN ("*="), SLASH ("/"), SLASH_ASSIGN ("/="), BIT_AND ("&"),
       BIT_AND_ASSIGN ("&="), BIT_OR ("|"), BIT_OR_ASSIGN ("|="),
       BIT_XOR ("^"), BIT_XOR_ASSIGN ("^="), REMAINDER ("%"),
       REMAINDER_ASSIGN ("%="), LSHIFT ("<<"), LSHIFT_ASSIGN ("<<="),
       RSHIFT (">>"), RSHIFT_ASSIGN (">>=") ;;

-- literals and identifiers:
%token TRUE ("true"), FALSE ("false"), NULL ("null"),
       INTEGER_LITERAL ("integer literal"), REAL_LITERAL ("real literal"),
       CHARACTER_LITERAL ("character literal"),
       STRING_LITERAL ("string literal"), IDENTIFIER ("identifier") ;;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;




------------------------------------------------------------
-- Start of the actual grammar
------------------------------------------------------------


   0 [: _M_state.ltCounter = 0; :]
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#extern_alias=extern_alias_directive)+
    | 0
   )
   try/recover(#using=using_directive)*
   try/recover(
     0 [: if (LA(2).kind != Token_ASSEMBLY) break; :] -- exit the "star loop"
     #global_attribute=global_attribute_section
   )*
   try/recover(#namespace=namespace_member_declaration)*
-> compilation_unit ;;


   EXTERN ALIAS identifier=identifier SEMICOLON
-> extern_alias_directive ;;

   USING
   (  ?[: LA(2).kind == Token_ASSIGN :]  -- "using alias" directive
      alias:identifier ASSIGN namespace_or_type_name:namespace_or_type_name
      using_alias_directive=using_alias_directive_data[alias, namespace_or_type_name]
    |
      namespace_name:namespace_name      -- "using namespace" directive
      using_namespace_directive=using_namespace_directive_data[namespace_name]
   )
   SEMICOLON
-> using_directive ;;

   0
-> using_alias_directive_data [
     argument member node alias:                  identifier;
     argument member node namespace_or_type_name: namespace_or_type_name;
] ;;

   0
-> using_namespace_directive_data [
     argument member node namespace_name: namespace_name;
] ;;




-- ATTRIBUTE sections, global and standard ones. They have a slight similarity
-- with Java's annotations in that they are used as advanced type modifiers.

-- Strictly seen, the ASSEMBLY here should just be attribute_target.
-- But for the sake of avoiding an LL(k)-ambiguous conflict
-- (in compilation_unit), we just allow "assembly".

   LBRACKET ASSEMBLY COLON
   #attribute=attribute
   ( 0 [: if (LA(2).kind == Token_RBRACKET) { break; } :]
     COMMA #attribute=attribute
   )*
   ( COMMA | 0 )
   RBRACKET
-> global_attribute_section ;;

   try/recover(#attribute=attribute_section)*
-> optional_attribute_sections ;;

   LBRACKET
   (  ?[: LA(2).kind == Token_COLON :] target=attribute_target COLON
    | 0
   )
   #attribute=attribute
   ( 0 [: if (LA(2).kind == Token_RBRACKET) { break; } :]
     COMMA #attribute=attribute
   )*
   ( COMMA | 0 )
   RBRACKET
-> attribute_section ;;

   identifier=identifier | keyword=keyword
-> attribute_target ;;

   name=type_name (arguments=attribute_arguments | 0)
-> attribute ;;

   LPAREN
   (
      -- empty argument list:
      RPAREN
    |
      -- argument list only containing named arguments:
      ?[: LA(2).kind == Token_ASSIGN :]
      #named_argument=named_argument @ COMMA
      RPAREN
    |
      -- argument list with positional arguments and
      -- optionally appended named arguments:
      #positional_argument=positional_argument
      ( COMMA
        (  ?[: LA(2).kind == Token_ASSIGN :]
           (#named_argument=named_argument @ COMMA)
           [: break; :] -- go directly to the closing parenthesis
         |
           #positional_argument=positional_argument
        )
      )*
      RPAREN
   )
-> attribute_arguments ;;

   attribute_argument_expression=expression
-> positional_argument ;;

   argument_name=identifier ASSIGN attribute_argument_expression=expression
-> named_argument ;;




-- NAMESPACES can be nested arbitrarily and contain stuff
-- like classes, interfaces, or other declarations.

   namespace_declaration=namespace_declaration
 | type_declaration=type_declaration
-> namespace_member_declaration ;;

   NAMESPACE name=qualified_identifier body=namespace_body (SEMICOLON | 0)
-> namespace_declaration ;;

   LBRACE
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#extern_alias=extern_alias_directive)+
    | 0
   )
   try/recover(#using=using_directive)*
   try/recover(#namespace=namespace_member_declaration)*
   RBRACE
-> namespace_body ;;

   attributes:optional_attribute_sections
   modifiers:optional_modifiers
   rest=type_declaration_rest[ attributes, modifiers ]
-> type_declaration ;;

-- C# 2.0 or higher allows partial classes, structs and interfaces.
-- We don't need to introduce an additional check here, because the lexer only
-- returns the PARTIAL token for C# versions >= 2.0, and IDENTIFIER otherwise.
-- TODO: after parsing, check if the modifiers are allowed for the specific
--       kind of type declaration.

 (
   PARTIAL
   (  class_declaration=class_declaration[ attributes, modifiers, true ]
    | struct_declaration=struct_declaration[ attributes, modifiers, true ]
    | interface_declaration=interface_declaration[ attributes, modifiers, true ]
   )
 |
   (  class_declaration=class_declaration[ attributes, modifiers, false ]
    | struct_declaration=struct_declaration[ attributes, modifiers, false ]
    | interface_declaration=interface_declaration[ attributes, modifiers, false ]
    | enum_declaration=enum_declaration[ attributes, modifiers ]
    | delegate_declaration=delegate_declaration[ attributes, modifiers ]
   )
 )
-> type_declaration_rest [
     argument temporary node attributes: optional_attribute_sections;
     argument temporary node modifiers:  optional_modifiers;
] ;;



-- Definition of a C# CLASS

   CLASS class_name=identifier
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   (class_base=class_base | 0)
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   body=class_body
   (SEMICOLON | 0)
-> class_declaration [
     argument member node attributes:  optional_attribute_sections;
     argument member node modifiers:   optional_modifiers;
     argument member variable partial: bool;
] ;;


-- Definition of a C# STRUCT

   STRUCT struct_name=identifier
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   (struct_interfaces=interface_base | 0)
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   body=struct_body
   (SEMICOLON | 0)
-> struct_declaration [
     argument member node attributes:  optional_attribute_sections;
     argument member node modifiers:   optional_modifiers;
     argument member variable partial: bool;
] ;;


-- Definition of a C# INTERFACE

   INTERFACE interface_name=identifier
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   (interface_base=interface_base | 0)
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   body=interface_body
   (SEMICOLON | 0)
-> interface_declaration [
     argument member node attributes:  optional_attribute_sections;
     argument member node modifiers:   optional_modifiers;
     argument member variable partial: bool;
] ;;


-- Definition of a C# ENUM

   ENUM enum_name=identifier
   (enum_base=enum_base | 0)
   body=enum_body
   (SEMICOLON | 0)
-> enum_declaration [
     argument member node attributes:  optional_attribute_sections;
     argument member node modifiers:   optional_modifiers;
] ;;


-- Definition of a C# DELEGATE

   DELEGATE return_type=return_type delegate_name=identifier
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   LPAREN (formal_parameters=formal_parameter_list | 0) RPAREN
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   SEMICOLON
-> delegate_declaration [
     argument member node attributes:  optional_attribute_sections;
     argument member node modifiers:   optional_modifiers;
] ;;


-- BASE CLASSES and INTERFACES
-- which can be implemented by the types above

-- For class_base, the first type in the list of base types _may_ be a class.
-- (But it can also be an interface.) In order to avoid ambiguities, only the
-- first part of the rule has the exact seperation. If the second part of the
-- rule is chosen, then the first element of base_type_sequence can be
-- the base class.

   COLON
   (  builtin_class_type=builtin_class_type
      (COMMA (#interface_type=type_name @ COMMA) | 0)
    |
      #base_type=type_name @ COMMA
   )
-> class_base ;;

   COLON #interface_type=type_name @ COMMA
-> interface_base ;;

   COLON integral_type=integral_type
-> enum_base ;;



-- BODIES of classes, interfaces, and the likes.

   LBRACE try/recover(#member_declaration=class_member_declaration)* RBRACE
-> class_body ;;

   LBRACE try/recover(#member_declaration=struct_member_declaration)* RBRACE
-> struct_body ;;

   LBRACE try/recover(#member_declaration=interface_member_declaration)* RBRACE
-> interface_body ;;

   LBRACE
   try/recover(
      #member_declaration=enum_member_declaration
      ( 0 [: if (LA(2).kind == Token_RBRACE) break; :] -- exit the "star loop"
        COMMA #member_declaration=enum_member_declaration
      )*
      (COMMA | 0)
    |
      0
   )
   RBRACE
-> enum_body ;;

   attributes=optional_attribute_sections
   member_name=identifier
   (ASSIGN value=constant_expression | 0)
-> enum_member_declaration ;;




-- Now on to what happens inside the class, interface, etc. bodies:


-- The CLASS MEMBER DECLARATION is one of the most complex rules in here,
-- and covers everything that can occur inside a class body.

   attributes:optional_attribute_sections
   modifiers:optional_modifiers
   (  finalizer_declaration=finalizer_declaration[ attributes, modifiers ]
    | other_declaration=class_or_struct_member_declaration[ attributes, modifiers ]
   )
-> class_member_declaration ;;

   attributes:optional_attribute_sections
   modifiers:optional_modifiers
   declaration=class_or_struct_member_declaration[ attributes, modifiers ]
-> struct_member_declaration ;;

-- The first few declarations start with a specific token and don't need
-- to be refactored. The other declarations must be split to avoid conflicts.

 (
   constant_declaration=constant_declaration[ attributes, modifiers ]
 | event_declaration=event_declaration[ attributes, modifiers ]
 |
   -- The OPERATOR DECLARATION, part one: conversion operator overloading.
   conversion_operator_declaration=conversion_operator_declaration[ attributes, modifiers ]
 |
   -- A normal or static CONSTRUCTOR DECLARATION.
   ?[: LA(2).kind == Token_LPAREN :]
   constructor_declaration=constructor_declaration[ attributes, modifiers ]
 |
   -- The TYPE DECLARATION, buried under lookahead conditions ;)
   ?[: (yytoken != Token_PARTIAL) || (LA(2).kind == Token_CLASS
        || LA(2).kind == Token_INTERFACE || LA(2).kind == Token_ENUM
        || LA(2).kind == Token_STRUCT || LA(2).kind == Token_DELEGATE) :]
   type_declaration_rest=type_declaration_rest[ attributes, modifiers ]
 |
   -- Many of the declarations start with a type, and method declarations
   -- start with a return type which is therefore the least common denominator.
   member_type:return_type
   (
      -- The OPERATOR DECLARATION rest, part two: overloading of real operators
      ?[: member_type->type == return_type::type_regular :]
      unary_or_binary_operator_declaration=unary_or_binary_operator_declaration[
        attributes, modifiers, member_type->regular_type
      ]
    |
      -- The INDEXER DECLARATION rest, part one.
      ?[: member_type->type == return_type::type_regular :]
      indexer_declaration=indexer_declaration[
        attributes, modifiers, member_type->regular_type, 0 /* no interface type */
      ]
    |
      -- The FIELD DECLARATION rest. Declares member variables.
      ?[: ( LA(2).kind == Token_SEMICOLON || LA(2).kind == Token_ASSIGN
            || LA(2).kind == Token_COMMA || LA(2).kind == Token_LBRACKET
          ) && (member_type->type == return_type::type_regular)  :]
      (#variable_declarator:variable_declarator[
         ((modifiers->modifiers & modifiers::mod_fixed) != 0) /* is a fixed size buffer? */
       ] @ COMMA)
      SEMICOLON
      field_declaration=variable_declaration_data[
        attributes, modifiers,
        member_type->regular_type, variable_declarator_sequence
      ]
    |
      -- and this is for rules that are still not split up sufficiently:
      member_name_or_interface_type:type_name_safe
        -- (interface_type for the indexer declaration, member_name otherwise)
      (
         -- The INDEXER DECLARATION rest, part two.
         ?[: member_type->type == return_type::type_regular :]
         DOT
         indexer_declaration=indexer_declaration[
           attributes, modifiers,
           member_type->regular_type, member_name_or_interface_type
         ]
       |
         -- The PROPERTY DECLARATION rest.
         ?[: member_type->type == return_type::type_regular :]
         property_declaration=property_declaration[
           attributes, modifiers, member_type->regular_type, member_name_or_interface_type
         ]
       |
         -- The METHOD DECLARATION rest.
         method_declaration=method_declaration[
           attributes, modifiers, member_type, member_name_or_interface_type
         ]
      )
   )
 )
-> class_or_struct_member_declaration [
     argument temporary node attributes: optional_attribute_sections;
     argument temporary node modifiers:  optional_modifiers;
] ;;



-- Having summarized the possible class/struct members, we now specify
-- what they look like.


-- The FINALIZER is what other languages know as deconstructor.
-- Only allowed inside classes.

   TILDE class_name=identifier LPAREN RPAREN
   (finalizer_body=block | SEMICOLON)
-> finalizer_declaration [
     argument member node attributes: optional_attribute_sections;
     argument member node modifiers:  optional_modifiers;
] ;;



-- The CONSTRUCTOR DECLARATION. Naturally quite similar to the method one.
-- Will also be folded into the member_declaration.

   class_name=identifier
   LPAREN (formal_parameters=formal_parameter_list | 0) RPAREN
   (constructor_initializer=constructor_initializer | 0)
   (body=block | SEMICOLON)
-> constructor_declaration [
     argument member node attributes: optional_attribute_sections;
     argument member node modifiers:  optional_modifiers;
] ;;

   COLON
   (  BASE [: (*yynode)->initializer_type = constructor_initializer::type_base; :]
    | THIS [: (*yynode)->initializer_type = constructor_initializer::type_this; :]
   )
   LPAREN arguments=optional_argument_list RPAREN
-> constructor_initializer [
     member variable initializer_type: constructor_initializer::constructor_initializer_type_enum;
] ;;

-- There is also a STATIC CONSTRUCTOR DECLARATION which is only used if
-- the modifiers contain "static" (and optionally "extern", and nothing else).
-- Apart from the "static" modifier, the static constructor declaration
-- is a subset of the generic one. In order to keep the parser simple,
-- we don't check on the modifiers here and thus only use
-- constructor_declaration. On the whole, modifier checking should be done
-- in the visitor, later on... but we could make an exception here.

--    class_name=identifier LPAREN RPAREN
--    (body=block | SEMICOLON)
-- -> static_constructor_declaration ;;



-- The EVENT DECLARATION.

   EVENT type=type
   (
      -- this condition avoids the LL(k) conflict here:
      ?[: (LA(2).kind == Token_COMMA) || (LA(2).kind == Token_ASSIGN)
          || (LA(2).kind == Token_SEMICOLON)
       :]
      (#variable_declarator=variable_declarator[false] @ COMMA) SEMICOLON
    |
      event_name=type_name
      LBRACE event_accessor_declarations=event_accessor_declarations RBRACE
   )
-> event_declaration [
     argument member node attributes: optional_attribute_sections;
     argument member node modifiers:  optional_modifiers;
] ;;

-- EVENT ACCESSOR DECLARATIONS appear inside an event declaration.

   accessor1_attributes:optional_attribute_sections
   (
      ADD accessor1_body:block
      add_accessor_declaration=event_accessor_declaration[
        accessor1_attributes, accessor1_body
      ]
      accessor2_attributes:optional_attribute_sections
      REMOVE accessor2_body:block
      remove_accessor_declaration=event_accessor_declaration[
        accessor2_attributes, accessor2_body
      ]
    |
      REMOVE accessor1_body:block
      remove_accessor_declaration=event_accessor_declaration[
        accessor1_attributes, accessor1_body
      ]
      accessor2_attributes:optional_attribute_sections
      ADD accessor2_body:block
      add_accessor_declaration=event_accessor_declaration[
        accessor2_attributes, accessor2_body
      ]
   )
-> event_accessor_declarations ;;

   0
-> event_accessor_declaration [
     argument member node attributes: optional_attribute_sections;
     argument member node body:       block;
] ;;



-- The different forms of the OPERATOR DECLARATION which overloads operators:
-- conversion, unary and binary operator declarations.

   (  IMPLICIT [: (*yynode)->conversion = conversion_operator_declaration::conversion_implicit; :]
    | EXPLICIT [: (*yynode)->conversion = conversion_operator_declaration::conversion_explicit; :]
   )
   OPERATOR target_type=type
   LPAREN source_type=type source_name=identifier RPAREN
   (body=block | SEMICOLON)
-> conversion_operator_declaration [
     argument member node attributes: optional_attribute_sections;
     argument member node modifiers:  optional_modifiers;
     member variable conversion:      conversion_operator_declaration::conversion_type_enum;
] ;;

   OPERATOR
   (   unary_op:overloadable_unary_only_operator[&(*yynode)->overloadable_operator_type]
       LPAREN source1_type=type source1_name=identifier RPAREN
         [: (*yynode)->unary_or_binary = overloadable_operator::type_unary;     :]
     |
       binary_op:overloadable_binary_only_operator[&(*yynode)->overloadable_operator_type]
       LPAREN source1_type=type source1_name=identifier
       COMMA  source2_type=type source2_name=identifier RPAREN
         [: (*yynode)->unary_or_binary = overloadable_operator::type_binary;    :]
     |
       unary_or_binary_op:overloadable_unary_or_binary_operator[&(*yynode)->overloadable_operator_type]
       LPAREN source1_type=type source1_name=identifier
       (
          COMMA source2_type=type source2_name=identifier
            [: (*yynode)->unary_or_binary = overloadable_operator::type_binary; :]
        |
          0 [: (*yynode)->unary_or_binary = overloadable_operator::type_unary;  :]
       )
       RPAREN
   )
   (body=block | SEMICOLON)
-> unary_or_binary_operator_declaration [
     argument member node attributes:            optional_attribute_sections;
     argument member node modifiers:             optional_modifiers;
     argument member node return_type:           type;
     member variable overloadable_operator_type: overloadable_operator::overloadable_operator_enum;
     member variable unary_or_binary:            overloadable_operator::unary_or_binary_enum;
] ;;


-- OVERLOADABLE OPERATORS for operator declarations.

 (
   BANG          [: *op = overloadable_operator::op_bang;          :]
 | TILDE         [: *op = overloadable_operator::op_tilde;         :]
 | INCREMENT     [: *op = overloadable_operator::op_increment;     :]
 | DECREMENT     [: *op = overloadable_operator::op_decrement;     :]
 | TRUE          [: *op = overloadable_operator::op_true;          :]
 | FALSE         [: *op = overloadable_operator::op_false;         :]
 )
-> overloadable_unary_only_operator [
     argument temporary variable op: overloadable_operator::overloadable_operator_enum*;
] ;;

 (
   STAR          [: *op = overloadable_operator::op_star;          :]
 | SLASH         [: *op = overloadable_operator::op_slash;         :]
 | REMAINDER     [: *op = overloadable_operator::op_remainder;     :]
 | BIT_AND       [: *op = overloadable_operator::op_bit_and;       :]
 | BIT_OR        [: *op = overloadable_operator::op_bit_or;        :]
 | BIT_XOR       [: *op = overloadable_operator::op_bit_xor;       :]
 | LSHIFT        [: *op = overloadable_operator::op_lshift;        :]
 | RSHIFT        [: *op = overloadable_operator::op_rshift;        :]
 | EQUAL         [: *op = overloadable_operator::op_equal;         :]
 | NOT_EQUAL     [: *op = overloadable_operator::op_not_equal;     :]
 | GREATER_THAN  [: *op = overloadable_operator::op_greater_than;  :]
 | LESS_THAN     [: *op = overloadable_operator::op_less_than;     :]
 | GREATER_EQUAL [: *op = overloadable_operator::op_greater_equal; :]
 | LESS_EQUAL    [: *op = overloadable_operator::op_less_equal;    :]
 )
-> overloadable_binary_only_operator [
     argument temporary variable op: overloadable_operator::overloadable_operator_enum*;
] ;;

 (
   PLUS          [: *op = overloadable_operator::op_plus;          :]
 | MINUS         [: *op = overloadable_operator::op_minus;         :]
 )
-> overloadable_unary_or_binary_operator [
     argument temporary variable op: overloadable_operator::overloadable_operator_enum*;
] ;;



-- The INDEXER DECLARATION rest.

   THIS LBRACKET formal_parameters=formal_parameter_list RBRACKET
   LBRACE accessor_declarations=accessor_declarations RBRACE
-> indexer_declaration [
     argument member node attributes:     optional_attribute_sections;
     argument member node modifiers:      optional_modifiers;
     argument member node type:           type;
     argument member node interface_type: type_name_safe;
] ;;



-- The PROPERTY DECLARATION rest.

   LBRACE accessor_declarations=accessor_declarations RBRACE
-> property_declaration [
     argument member node attributes:     optional_attribute_sections;
     argument member node modifiers:      optional_modifiers;
     argument member node type:           type;
     argument member node property_name:  type_name_safe;
] ;;

-- ACCESSOR DECLARATIONS appear inside a property declaration.

   accessor1_attributes=optional_attribute_sections
   (accessor1_modifier=accessor_modifier | 0)
   (
      GET (accessor1_body=block | SEMICOLON)
        [: (*yynode)->accessor1_type = accessor_declarations::type_get;     :]
      (
         accessor2_attributes=optional_attribute_sections
         (accessor2_modifier=accessor_modifier | 0)
         SET (accessor2_body=block | SEMICOLON)
           [: (*yynode)->accessor2_type = accessor_declarations::type_set;  :]
       | 0 [: (*yynode)->accessor2_type = accessor_declarations::type_none; :]
      )
    |
      SET (accessor1_body=block | SEMICOLON)
        [: (*yynode)->accessor1_type = accessor_declarations::type_set;     :]
      (
         accessor2_attributes=optional_attribute_sections
         (accessor2_modifier=accessor_modifier | 0)
         GET (accessor2_body=block | SEMICOLON)
           [: (*yynode)->accessor2_type = accessor_declarations::type_get;  :]
       | 0 [: (*yynode)->accessor2_type = accessor_declarations::type_none; :]
      )
   )
-> accessor_declarations [
     member variable accessor1_type: accessor_declarations::accessor_type_enum;
     member variable accessor2_type: accessor_declarations::accessor_type_enum;
] ;;

 (
   PROTECTED     [: (*yynode)->access_policy = access_policy::access_protected;          :]
   (  INTERNAL   [: (*yynode)->access_policy = access_policy::access_protected_internal; :]
    | 0
   )
 | INTERNAL      [: (*yynode)->access_policy = access_policy::access_internal;           :]
   (  PROTECTED  [: (*yynode)->access_policy = access_policy::access_protected_internal; :]
    | 0
   )
 | PRIVATE       [: (*yynode)->access_policy = access_policy::access_private;            :]
 )
-> accessor_modifier [
     member variable access_policy: access_policy::access_policy_enum;
] ;;



-- The METHOD DECLARATION rest.

   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   LPAREN (formal_parameters=formal_parameter_list | 0) RPAREN
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   (method_body=block | SEMICOLON)
-> method_declaration [
     argument member node attributes:     optional_attribute_sections;
     argument member node modifiers:      optional_modifiers;
     argument member node return_type:    return_type;
     argument member node method_name:    type_name_safe;
] ;;




-- Interfaces have their own specific INTERFACE MEMBER DECLARATIONS.
-- Resembling the class_or_struct_member_declaration, but less complex.

   attributes:optional_attribute_sections
   (  NEW  [: decl_new = true;  :]
    | 0    [: decl_new = false; :]
   )
   (
      event_declaration=interface_event_declaration[
        attributes, decl_new
      ]
    |
      -- Many of the declarations start with a type, and method declarations
      -- start with a return type which is therefore the least common denominator.
      member_type:return_type
      (
         -- The INDEXER DECLARATION rest.
         ?[: member_type->type == return_type::type_regular :]
         indexer_declaration=interface_indexer_declaration[
           attributes, decl_new, member_type->regular_type
         ]
       |
         -- The method and property declarations need to be split further.
         member_name:identifier
         (
            -- The INTERFACE PROPERTY DECLARATION rest.
            ?[: member_type->type == return_type::type_regular :]
            interface_property_declaration=interface_property_declaration[
              attributes, decl_new,
              member_type->regular_type, member_name
            ]
          |
            -- The INTERFACE METHOD DECLARATION rest.
            interface_method_declaration=interface_method_declaration[
              attributes, decl_new, member_type, member_name
            ]
         )
      )
   )
-> interface_member_declaration [
     temporary variable decl_new: bool; -- specifies if the "new" keyword prepends the declaration
] ;;


-- Here are the detailed interface member declaration rules. Basically,
-- all of them are just simplified versions of the class/struct ones.

-- The INTERFACE EVENT DECLARATION.

   EVENT event_type=type event_name=identifier SEMICOLON
-> interface_event_declaration [
     argument member node attributes:    optional_attribute_sections;
     argument member variable decl_new:  bool;
] ;;

-- The INTERFACE INDEXER DECLARATION.

   THIS LBRACKET formal_parameters=formal_parameter_list RBRACKET
   LBRACE interface_accessors=interface_accessors RBRACE
-> interface_indexer_declaration [
     argument member node attributes:    optional_attribute_sections;
     argument member variable decl_new:  bool;
     argument member node type:          type;
] ;;

-- The INTERFACE PROPERTY DECLARATION.

   LBRACE interface_accessors=interface_accessors RBRACE
-> interface_property_declaration [
     argument member node attributes:    optional_attribute_sections;
     argument member variable decl_new:  bool;
     argument member node type:          type;
     argument member node property_name: identifier;
] ;;

-- And last but not least, the INTERFACE METHOD DECLARATION.

   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(type_parameters=type_parameters)
    | 0
   )
   LPAREN (formal_parameters=formal_parameter_list | 0) RPAREN
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      try/recover(#type_parameter_constraints=type_parameter_constraints_clause)+
    | 0
   )
   SEMICOLON
-> interface_method_declaration [
     argument member node attributes:    optional_attribute_sections;
     argument member variable decl_new:  bool;
     argument member node return_type:   return_type;
     argument member node method_name:   identifier;
] ;;



-- An INTERFACE ACCESSOR looks like "[attributes] get;" or "[attributes] set;"
-- and is used by interface_indexer_declaration and interface_property_declaration.

   accessor1_attributes=optional_attribute_sections
   (
      GET SEMICOLON
        [: (*yynode)->accessor1_type = accessor_declarations::type_get; :]
      (
         accessor2_attributes=optional_attribute_sections SET SEMICOLON
           [: (*yynode)->accessor2_type = accessor_declarations::type_set;  :]
       | 0 [: (*yynode)->accessor2_type = accessor_declarations::type_none; :]
      )
    |
      SET SEMICOLON
        [: (*yynode)->accessor1_type = accessor_declarations::type_set; :]
      (
         accessor2_attributes=optional_attribute_sections GET SEMICOLON
           [: (*yynode)->accessor2_type = accessor_declarations::type_get;  :]
       | 0 [: (*yynode)->accessor2_type = accessor_declarations::type_none; :]
      )
   )
-> interface_accessors [
     member variable accessor1_type: accessor_declarations::accessor_type_enum;
     member variable accessor2_type: accessor_declarations::accessor_type_enum;
] ;;




-- A FORMAL PARAMETER LIST is part of a method header and contains one or more
-- parameters, optionally ending with a variable-length "parameter array".
-- It's not as hackish as it used to be, nevertheless it could still be nicer.

   0 [: bool parameter_array_occurred = false; :]
   (
      try/recover(
        #formal_parameter=formal_parameter[&parameter_array_occurred]
        ( 0 [: if( parameter_array_occurred == true ) { break; } :]
            -- Don't proceed after the parameter array. If there's a cleaner way
            -- to exit the loop when _M_parameter_array_occurred == true,
            -- please use that instead of this construct.
          COMMA #formal_parameter=formal_parameter[&parameter_array_occurred]
        )*
      )
    | 0
   )
-> formal_parameter_list ;;

-- How it _should_ look:
--
--    0 [: bool parameter_array_occurred = false; :]
--    (
--       try/recover(
--         #formal_parameter=formal_parameter[&parameter_array_occurred]
--         ( ?[: parameter_array_occurred == false :] -- kdev-pg dismisses this condition!
--           COMMA #formal_parameter=formal_parameter[&parameter_array_occurred]
--         )*
--       )
--     | 0
--    )
-- -> formal_parameter_list ;;

   attributes=optional_attribute_sections
   (
      PARAMS params_type=array_type variable_name=identifier
        [: *parameter_array_occurred = true; :]
    |
      modifier=optional_parameter_modifier type=type variable_name=identifier
   )
-> formal_parameter [
     argument temporary variable parameter_array_occurred: bool*;
] ;;


-- An OPTIONAL ARGUMENT LIST is used when calling methods
-- (not for declaring them, that's what formal parameter lists are for).

   try/recover(#argument=argument @ COMMA) | 0
-> optional_argument_list ;;

 (
   expression=expression
     [: (*yynode)->argument_type = argument::type_value_parameter;     :]
 | REF expression=expression
     [: (*yynode)->argument_type = argument::type_reference_parameter; :]
 | OUT expression=expression
     [: (*yynode)->argument_type = argument::type_output_parameter;    :]
 )
-> argument [
     member variable argument_type: argument::argument_type_enum;
] ;;




-- Type parameters, type arguments, and constraints clauses form C#'s support
-- for generics and are responsible for the greater-than special casing.

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

   attributes=optional_attribute_sections parameter_name=identifier
-> type_parameter ;;


-- TYPE ARGUMENTS are used in initializers, invocations, etc. to
-- specify the exact types for this generic class/method instance.

   LESS_THAN [: int currentLtLevel = _M_state.ltCounter; _M_state.ltCounter++; :]
   #type_argument=type @ COMMA
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


   GREATER_THAN  [: _M_state.ltCounter -= 1; :]  -- ">"
 | RSHIFT        [: _M_state.ltCounter -= 2; :]  -- ">>"
-> type_arguments_or_parameters_end ;;


-- Type parameter CONSTRAINTS CLAUSES also belong to C#'s generics,
-- and can narrow down the allowed types given as type arguments.

   WHERE type_parameter=identifier COLON constraints=type_parameter_constraints
-> type_parameter_constraints_clause ;;

 (
   primary_or_secondary_constraint=primary_or_secondary_constraint
   (  COMMA
      (  #secondary_constraint=secondary_constraint
         (COMMA (  #secondary_constraint=secondary_constraint
                 | constructor_constraint=constructor_constraint
                     [: break; :] -- it's the last item: exit the "star loop"
                )
         )*
       |
         constructor_constraint=constructor_constraint
      )
    | 0
   )
 |
   #secondary_constraint=secondary_constraint
   (COMMA (  #secondary_constraint=secondary_constraint
           | constructor_constraint=constructor_constraint
               [: break; :] -- it's the last item: exit the "star loop"
          )
   )*
 |
   constructor_constraint=constructor_constraint
 )
-> type_parameter_constraints ;;

 (
   class_type_or_secondary_constraint=class_type
           [: (*yynode)->constraint_type = primary_or_secondary_constraint::type_type;   :]
 | CLASS   [: (*yynode)->constraint_type = primary_or_secondary_constraint::type_class;  :]
 | STRUCT  [: (*yynode)->constraint_type = primary_or_secondary_constraint::type_struct; :]
 )
-> primary_or_secondary_constraint [
     member variable constraint_type: primary_or_secondary_constraint::primary_or_secondary_constraint_enum;
] ;;

   #interface_type_or_type_parameter=type_name
-> secondary_constraint ;;

   NEW LPAREN RPAREN
-> constructor_constraint ;;





-- And now for the good stuff: statements, expressions and the likes. Yay!

-- This is a BLOCK, a list of statements. It is used in many contexts:
--  - As the body of a method, constructor, overloaded operator, ...
--  - As the body of an accessor
--  - As a completely independent braced block of code inside a method,
--    starting a new scope for variable definitions

   LBRACE try/recover(#statement=block_statement)* RBRACE
-> block ;;

-- A BLOCK STATEMENT is either an embedded statement or a variable declaration.

 (
   ?[: LA(2).kind == Token_COLON :]
   labeled_statement=labeled_statement
 |
   local_constant_declaration_statement=local_constant_declaration SEMICOLON
 |
   -- Local variable declarations, as well as expression statements, can start
   -- with class1<xxx>.bla or similar. This is only solvable with LL(k), so
   -- what's needed here is the following hack lookahead function, until
   -- backtracking or real LL(k) is implemented.
   try/rollback (
     local_variable_declaration_statement=local_variable_declaration_statement
   ) catch (
     statement=embedded_statement
   )
 )
-> block_statement ;;

   label=identifier COLON block_statement
-> labeled_statement ;;



-- VARIABLE DECLARATIONS, initializers, etc.

-- The LOCAL VARIABLE DECLARATION does not allow attributes or modifiers,
-- this is only allowed in field declarations. Both store their data with
-- the variable_declaration_data rule, using rule arguments.

   declaration=local_variable_declaration SEMICOLON
-> local_variable_declaration_statement ;;

   type:type (#declarator:variable_declarator[false] @ COMMA)
   data=variable_declaration_data[
     0 /* no attributes */, 0 /* no modifiers */, type, declarator_sequence
   ]
-> local_variable_declaration ;;

   0
-> variable_declaration_data [
     argument member node attributes:  optional_attribute_sections; -- not used in local
     argument member node modifiers:   optional_modifiers;          -- variable declarations
     argument member node type:                 type;
     argument member node #variable_declarator: variable_declarator;
] ;;

-- The VARIABLE DECLARATOR is the part after the type specification for a
-- variable declaration. There can be more declarators, seperated by commas.

   variable_name=identifier
   (
      -- Fixed size buffers are a C# 2.0 feature. Not in the
      -- ECMA specification, but used by both Microsoft and Mono compilers.
      ?[: fixed_size_buffer :]
      LBRACKET array_size=expression RBRACKET
    |
      ?[: !fixed_size_buffer :]
      ASSIGN variable_initializer=variable_initializer
    |
      ?[: !fixed_size_buffer :]
      0
   )
-> variable_declarator [
     argument temporary variable fixed_size_buffer: bool;
] ;;


-- The CONSTANT DECLARATION. Declares "const" values.
-- Analog to variable declarations, attributes and modifiers are not allowed
-- in local constant declarations, only in class-wide ones.

   CONST type:type (#declarator:constant_declarator @ COMMA)
   data=constant_declaration_data[
     0 /* no attributes */, 0 /* no modifiers */, type, declarator_sequence
   ]
-> local_constant_declaration ;;

   CONST type:type (#declarator:constant_declarator @ COMMA) SEMICOLON
   data=constant_declaration_data[
     attributes, modifiers, type, declarator_sequence
   ]
-> constant_declaration [
     argument temporary node attributes:  optional_attribute_sections;
     argument temporary node modifiers:   optional_modifiers;
] ;;

   0
-> constant_declaration_data [
     argument member node attributes:  optional_attribute_sections; -- not used in local
     argument member node modifiers:   optional_modifiers;          -- constant declarations
     argument member node type:                 type;
     argument member node #constant_declarator: constant_declarator;
] ;;

   constant_name=identifier ASSIGN expression=constant_expression
-> constant_declarator ;;


-- The INITIALIZERS provide the actual values for the variable declarators.

   expression=expression
 | array_initializer=array_initializer
 -- unsafe grammar extension: stackalloc initializer
 | stackalloc_initializer=stackalloc_initializer
-> variable_initializer ;;

   LBRACE
   try/recover(
      #variable_initializer=variable_initializer
      ( 0 [: if (LA(2).kind == Token_RBRACE) { break; } :]
        COMMA #variable_initializer=variable_initializer
      )*
      ( COMMA | 0 )
    |
      0
   )
   RBRACE
-> array_initializer ;;

-- unsafe grammar extension: stackalloc initializer
   STACKALLOC unmanaged_type LBRACKET expression=expression RBRACKET
-> stackalloc_initializer ;;




-- The (embedded) STATEMENT is a central point of the grammar,
-- even if delegating most of the work to its children.

 (
   block=block  -- more block_statements within {} braces
 -- selection statements:
 | if_statement=if_statement
 | switch_statement=switch_statement
 -- iteration statements:
 | while_statement=while_statement
 | do_while_statement=do_while_statement
 | for_statement=for_statement
 | foreach_statement=foreach_statement
 -- jump statements:
 | break_statement=break_statement
 | continue_statement=continue_statement
 | goto_statement=goto_statement
 | return_statement=return_statement
 | throw_statement=throw_statement
 -- other statements:
 | try_statement=try_statement
 | lock_statement=lock_statement
 | using_statement=using_statement
 | SEMICOLON    -- the specification calls it empty_statement
 |
   -- CHECKED and UNCHECKED can also be the start of an expression.
   -- So, manual checking if the second token is the start of a block.
   ?[: LA(2).kind == Token_LBRACE :]
   checked_statement=checked_statement
 |
   ?[: LA(2).kind == Token_LBRACE :]
   unchecked_statement=unchecked_statement
 |
   -- Iterators with yield have been introduced by C# 2.0, and for C# 1.0
   -- the lexer returns the IDENTIFIER token instead of YIELD, so we don't
   -- need to do a specific version check here.
   -- YIELD is a non-keyword identifier, so it clashes with expressions:
   ?[: LA(2).kind == Token_RETURN || LA(2).kind == Token_BREAK :]
   yield_statement=yield_statement
 |
   -- method call, assignment, etc.:
   expression_statement=statement_expression SEMICOLON
 |
   -- unsafe grammar extension: "unsafe" statement
   unsafe_statement=unsafe_statement
 |
   -- unsafe grammar extension: "fixed" statement
   fixed_statement=fixed_statement
 )
-> embedded_statement ;;


-- Simple one-rule statements:

   IF LPAREN condition=boolean_expression RPAREN if_body=embedded_statement
   (ELSE else_body=embedded_statement | 0)
     -- the traditional "dangling-else" conflict:
     -- kdevelop-pg generates proper code here, matching as soon as possible.
-> if_statement ;;

   WHILE LPAREN condition=boolean_expression RPAREN body=embedded_statement
-> while_statement ;;

   DO body=embedded_statement
   WHILE LPAREN condition=boolean_expression RPAREN SEMICOLON
-> do_while_statement ;;

   FOREACH LPAREN
   variable_type=type variable_name=identifier IN collection=expression
   RPAREN
   body=embedded_statement
-> foreach_statement ;;

   BREAK SEMICOLON
-> break_statement ;;

   CONTINUE SEMICOLON
-> continue_statement ;;

   GOTO
   (  label=identifier
        [: (*yynode)->goto_type = goto_statement::type_labeled_statement; :]
    | CASE constant_expression=constant_expression
        [: (*yynode)->goto_type = goto_statement::type_switch_case;       :]
    | DEFAULT
        [: (*yynode)->goto_type = goto_statement::type_switch_default;    :]
   )
   SEMICOLON
-> goto_statement [
     member variable goto_type: goto_statement::goto_statement_enum;
] ;;

   RETURN (return_expression=expression | 0) SEMICOLON
-> return_statement ;;

   THROW (exception=expression | 0) SEMICOLON
-> throw_statement ;;

   CHECKED body=block
-> checked_statement ;;

   UNCHECKED body=block
-> unchecked_statement ;;

   LOCK LPAREN lock_expression=expression RPAREN body=embedded_statement
-> lock_statement ;;

   YIELD
   (  RETURN return_expression=expression
        [: (*yynode)->yield_type = yield_statement::type_yield_return; :]
    | BREAK
        [: (*yynode)->yield_type = yield_statement::type_yield_break; :]
   )
   SEMICOLON
-> yield_statement [
     member variable yield_type: yield_statement::yield_statement_enum;
] ;;


-- unsafe grammar extension: "unsafe" and "fixed" statements

   UNSAFE body=block
-> unsafe_statement ;;

   FIXED LPAREN
   pointer_type=pointer_type
   (fixed_pointer_declarator=fixed_pointer_declarator @ COMMA)
   RPAREN
   body=embedded_statement
-> fixed_statement ;;

   pointer_name=identifier ASSIGN initializer=expression
-> fixed_pointer_declarator ;;


-- The SWITCH STATEMENT, consisting of a header and multiple
-- "case x:" or "default:" switch statement groups.

   SWITCH LPAREN switch_expression=expression RPAREN
   LBRACE try/recover(#switch_section=switch_section)* RBRACE
-> switch_statement ;;

   (#label=switch_label)+
   ( 0 [: if (yytoken == Token_DEFAULT && LA(2).kind != Token_LPAREN)
            { break; }  // don't give in to default_value_expression
        :]
     #statement=block_statement
   )+
-> switch_section ;;

   (  CASE case_expression=constant_expression
      [: (*yynode)->branch_type = switch_label::case_branch;    :]
    | DEFAULT
      [: (*yynode)->branch_type = switch_label::default_branch; :]
   ) COLON
-> switch_label [
     member variable branch_type: switch_label::branch_type_enum;
] ;;


-- The TRY STATEMENT, also known as try/catch/finally block.

   TRY try_body=block
   (  catch_clauses=catch_clauses (FINALLY finally_body=block | 0)
    | FINALLY finally_body=block
   )
-> try_statement ;;

   (
      ?[: LA(2).kind != Token_LPAREN :]
      general_catch_clause=general_catch_clause
    |
      ( -- also let general catch clauses get through:
        0 [: if (LA(2).kind != Token_LPAREN) { break; } :]
        #specific_catch_clause=specific_catch_clause
      )+
      ( general_catch_clause=general_catch_clause | 0 )
   )
-> catch_clauses ;;

   CATCH LPAREN
   exception_type=class_type (exception_name=identifier | 0)
   RPAREN
   body=block
-> specific_catch_clause ;;

   CATCH body=block
-> general_catch_clause ;;


-- The USING STATEMENT, acquiring and afterwards disposing a System.Disposable.

   USING LPAREN resource_acquisition=resource_acquisition RPAREN
   body=embedded_statement
-> using_statement ;;

-- Hm, we know that LL(k) conflict from somewhere, don't we?
-- Right, it's the same one as in block_statement and the upcoming for_control.

   try/rollback( local_variable_declaration=local_variable_declaration )
   catch( expression=expression )
-> resource_acquisition ;;


-- The FOR STATEMENT, including its problematic child for_control.

   FOR LPAREN for_control=for_control RPAREN for_body=embedded_statement
-> for_statement ;;

-- The FOR CONTROL is the three statements inside the for(...) parentheses,
-- or the alternative foreach specifier. It has the same problematic conflict
-- between local_variable_declaration and expression that block_statement also
-- has and which is only solvable with LL(k). Until backtracking or real LL(k)
-- is implemented, we have to workaround with a lookahead hack function.

   (
      try/rollback (
        local_variable_declaration=local_variable_declaration  -- "int i = 0"
      ) catch (
        #statement_expression=statement_expression @ COMMA
      )
    |
      0
   )
   SEMICOLON
   (for_condition=boolean_expression           | 0) SEMICOLON  -- "i < size;"
   (#for_iterator=statement_expression @ COMMA | 0)            -- "i++"
-> for_control ;;




-- EXPRESSIONS
-- Note that most of these expressions follow the pattern
--   thisLevelExpression :
--     nextHigherPrecedenceExpression @ OPERATOR
--
-- The operators in C# have the following precedences:
--  lowest  (14)  Assignment                   = *= /= %= += -= <<= >>= &= ^= |=
--          (13)  Conditional                  ?:
--          (12)  Conditional OR               ||
--          (11)  Conditional AND              &&
--          (10)  Logical OR                   |
--          ( 9)  Logical XOR                  ^
--          ( 8)  Logical AND                  &
--          ( 7)  Equality                     == !=
--          ( 6)  Relational and type-testing  < > <= >= is as
--          ( 5)  Shift                        << >>
--          ( 4)  Additive                     +(binary) -(binary)
--          ( 3)  Multiplicative               * / %
--          ( 2)  Unary                        +(unary) -(unary) ! ~ ++x --x (type)x
--  highest ( 1)  Primary                      x.y f(x) a[x] x++ x-- new


-- Both BOOLEAN and CONSTANT EXPRESSIONS ought to return a certain kind of
-- value, but it's not possible for any parser to check those restrictions.

   expression=expression
-> constant_expression ;;

   expression=expression
-> boolean_expression ;;

-- A STATEMENT EXPRESSION may not contain certain subsets of expression,
-- but it's just not feasible for LL(k) parsers to filter them out.

   expression=expression
-> statement_expression ;;


-- So this is the actual EXPRESSION, also known as assignment expression.

   conditional_expression=conditional_expression
   (
      (  ASSIGN
           [: (*yynode)->assignment_operator = expression::op_assign;           :]
       | PLUS_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_plus_assign;      :]
       | MINUS_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_minus_assign;     :]
       | STAR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_star_assign;      :]
       | SLASH_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_slash_assign;     :]
       | REMAINDER_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_remainder_assign; :]
       | BIT_AND_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_and_assign;   :]
       | BIT_OR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_or_assign;    :]
       | BIT_XOR_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_bit_xor_assign;   :]
       | LSHIFT_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_lshift_assign;    :]
       | RSHIFT_ASSIGN
           [: (*yynode)->assignment_operator = expression::op_rshift_assign;    :]
      )
      assignment_expression=expression
    |
      0 [: (*yynode)->assignment_operator = expression::no_assignment; :]
   )
-> expression [
     member variable assignment_operator: expression::assignment_operator_enum;
] ;;


   null_coalescing_expression=null_coalescing_expression
   (
      QUESTION if_expression=expression COLON else_expression=expression
    |
      -- this rather hackish solution prevents false errors for expressions
      -- like "a is sometype ? if_exp : else_exp", where nullable_type steals
      -- the question mark from conditional_expression.
      ?[: (compatibility_mode() >= csharp20_compatibility)
          && is_nullable_type(last_relational_expression_rest_type((*yynode)->null_coalescing_expression)) :]
      0 [: unset_nullable_type(last_relational_expression_rest_type((*yynode)->null_coalescing_expression)); :]
      if_expression=expression COLON else_expression=expression
    |
      0
   )
-> conditional_expression ;;

-- The NULL COALESCING EXPRESSION is new in C# 2.0 and provides fallback values
-- for nullable variables. If a is non-null, (a ?? b) returns a,
-- but if a is null, (a ?? b) returns b.
-- Version checking is already done by the lexer and not needed here.

   #expression=logical_or_expression @ QUESTIONQUESTION
-> null_coalescing_expression ;;

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
   (#additional_expression=relational_expression_rest)*
-> relational_expression ;;

 (
   (  LESS_THAN     [: (*yynode)->relational_operator = relational_expression_rest::op_less_than;     :]
    | GREATER_THAN  [: (*yynode)->relational_operator = relational_expression_rest::op_greater_than;  :]
    | LESS_EQUAL    [: (*yynode)->relational_operator = relational_expression_rest::op_less_equal;    :]
    | GREATER_EQUAL [: (*yynode)->relational_operator = relational_expression_rest::op_greater_equal; :]
   )
   expression=shift_expression
 |
   (  IS [: (*yynode)->relational_operator = relational_expression_rest::op_is; :]
    | AS [: (*yynode)->relational_operator = relational_expression_rest::op_as; :]
   )
   type=type
 )
-> relational_expression_rest [
     member variable relational_operator: relational_expression_rest::relational_operator_enum;
] ;;

   expression=additive_expression
   (#additional_expression=shift_expression_rest)*
-> shift_expression ;;

   (  LSHIFT   [: (*yynode)->shift_operator = shift_expression_rest::op_lshift; :]
    | RSHIFT   [: (*yynode)->shift_operator = shift_expression_rest::op_rshift; :]
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



-- So, up till now this was the easy stuff. Here comes another sincere
-- conflict in the grammar that can only be solved with LL(k).
-- The conflict in this rule is the ambiguity between type casts (which
-- can be arbitrary class names within parentheses) and primary_expressions,
-- which can also look that way from an LL(1) perspective.
-- Until real LL(k) or backtracking is implemented in kdev-pg, this problem
-- is solved with another lookahead hack function.

 (
   INCREMENT unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_incremented_expression; :]
 | DECREMENT unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_decremented_expression; :]
 | MINUS unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_unary_minus_expression; :]
 | PLUS  unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_unary_plus_expression;  :]
 | TILDE unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_bitwise_not_expression; :]
 | BANG  unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_logical_not_expression; :]
 |
   try/rollback (
     cast_expression=cast_expression
       [: (*yynode)->rule_type = unary_expression::type_cast_expression;      :]
   )
   catch (
     primary_expression=primary_expression
       [: (*yynode)->rule_type = unary_expression::type_primary_expression;   :]
   )
 |
   -- unsafe grammar extension: pointer indirection expression
   STAR unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_pointer_indirection_expression; :]
 |
   -- unsafe grammar extension: addressof expression
   BIT_AND unary_expression=unary_expression
     [: (*yynode)->rule_type = unary_expression::type_addressof_expression;   :]
 )
-> unary_expression [
     member variable rule_type: unary_expression::unary_expression_enum;
] ;;

   LPAREN type=type RPAREN casted_expression=unary_expression
-> cast_expression ;;


-- PRIMARY EXPRESSIONs: qualified names, array expressions,
--                      method invocation, post increment/decrement, etc.

   primary_atom=primary_atom (#primary_suffix=primary_suffix)*
-> primary_expression ;;

 (
   -- this is the part of member_access that's not in primary_atom
   DOT member_name=identifier
   try/rollback(
      ?[: compatibility_mode() >= csharp20_compatibility :]
      type_arguments=type_arguments
    | 0
   ) catch(0)
     [: (*yynode)->suffix_type = primary_suffix::type_member_access;   :]
 |
   -- the suffix part of invocation_expression
   LPAREN arguments=optional_argument_list RPAREN
     [: (*yynode)->suffix_type = primary_suffix::type_invocation;      :]
 |
   -- element_access (also known as array access)
   LBRACKET (#expression=expression @ COMMA) RBRACKET
     [: (*yynode)->suffix_type = primary_suffix::type_element_access;  :]
 |
   INCREMENT
     [: (*yynode)->suffix_type = primary_suffix::type_increment;       :]
 |
   DECREMENT
     [: (*yynode)->suffix_type = primary_suffix::type_decrement;       :]
 |
   -- unsafe grammar extension: pointer access
   ARROW_RIGHT member_name=identifier
   try/rollback(
      ?[: compatibility_mode() >= csharp20_compatibility :]
      type_arguments=type_arguments
    | 0
   ) catch(0)
     [: (*yynode)->suffix_type = primary_suffix::type_pointer_member_access; :]
 )
-> primary_suffix [
     member variable suffix_type: primary_suffix::primary_suffix_enum;
] ;;


-- PRIMARY ATOM: the basic element of a primary expression,
--               and expressions in general

 (
   literal=literal
     [: (*yynode)->rule_type = primary_atom::type_literal;               :]
 |
   LPAREN expression=expression RPAREN
     [: (*yynode)->rule_type = primary_atom::type_parenthesized_expression;    :]
 |
   simple_name_or_member_access=simple_name_or_member_access
     [: (*yynode)->rule_type = primary_atom::type_member_access;         :]
 |
   THIS
     [: (*yynode)->rule_type = primary_atom::type_this_access;           :]
 |
   base_access=base_access
     [: (*yynode)->rule_type = primary_atom::type_base_access;           :]
 |
   new_expression=new_expression
     [: (*yynode)->rule_type = primary_atom::type_new_expression;        :]
 |
   typeof_expression=typeof_expression
     [: (*yynode)->rule_type = primary_atom::type_typeof_expression;     :]
 |
   CHECKED LPAREN expression=expression RPAREN
     [: (*yynode)->rule_type = primary_atom::type_checked_expression;    :]
 |
   UNCHECKED LPAREN expression=expression RPAREN
     [: (*yynode)->rule_type = primary_atom::type_unchecked_expression;  :]
 |
   DEFAULT LPAREN type=type RPAREN
     [: (*yynode)->rule_type = primary_atom::type_default_value_expression;    :]
 |
   ?[: compatibility_mode() >= csharp20_compatibility :]
   anonymous_method_expression=anonymous_method_expression
     [: (*yynode)->rule_type = primary_atom::type_anonymous_method_expression; :]
 |
   -- unsafe grammar extension: sizeof(type)
   SIZEOF LPAREN unmanaged_type=unmanaged_type RPAREN
     [: (*yynode)->rule_type = primary_atom::type_sizeof_expression;    :]
 )
-> primary_atom [
     member variable rule_type: primary_atom::primary_atom_enum;
] ;;


-- Here come the more complex parts of primary_atom that have been split out.

-- This rule covers two rules from the specification, the SIMPLE NAME and
-- most of MEMBER ACCESS.

 (
   (  ?[: LA(2).kind == Token_SCOPE :] qualified_alias_label=identifier SCOPE
    | 0
   )
   member_name=identifier
   try/rollback(
      ?[: compatibility_mode() >= csharp20_compatibility :]
      type_arguments=type_arguments
    | 0
   ) catch(0)
 |
   predefined_type=predefined_type DOT member_name=identifier
   try/rollback(
      ?[: compatibility_mode() >= csharp20_compatibility :]
      type_arguments=type_arguments
    | 0
   ) catch(0)
 )
-> simple_name_or_member_access ;;

 (
   BOOL     [: (*yynode)->type = predefined_type::type_bool;    :]
 | BYTE     [: (*yynode)->type = predefined_type::type_byte;    :]
 | CHAR     [: (*yynode)->type = predefined_type::type_char;    :]
 | DECIMAL  [: (*yynode)->type = predefined_type::type_decimal; :]
 | DOUBLE   [: (*yynode)->type = predefined_type::type_double;  :]
 | FLOAT    [: (*yynode)->type = predefined_type::type_float;   :]
 | INT      [: (*yynode)->type = predefined_type::type_int;     :]
 | LONG     [: (*yynode)->type = predefined_type::type_long;    :]
 | OBJECT   [: (*yynode)->type = predefined_type::type_object;  :]
 | SBYTE    [: (*yynode)->type = predefined_type::type_sbyte;   :]
 | SHORT    [: (*yynode)->type = predefined_type::type_short;   :]
 | STRING   [: (*yynode)->type = predefined_type::type_string;  :]
 | UINT     [: (*yynode)->type = predefined_type::type_uint;    :]
 | ULONG    [: (*yynode)->type = predefined_type::type_ulong;   :]
 | USHORT   [: (*yynode)->type = predefined_type::type_ushort;  :]
 )
-> predefined_type [
     member variable type: predefined_type::predefined_type_enum;
] ;;


   BASE
   (  DOT identifier=identifier
      try/rollback(
         ?[: compatibility_mode() >= csharp20_compatibility :]
         type_arguments=type_arguments
       | 0
      ) catch(0)
        [: (*yynode)->access_type = base_access::type_base_member_access;  :]
    |
      LBRACKET (#expression=expression @ COMMA) RBRACKET
        [: (*yynode)->access_type = base_access::type_base_indexer_access; :]
   )
-> base_access [
     member variable access_type: base_access::base_access_enum;
] ;;


   DELEGATE (anonymous_method_signature=anonymous_method_signature | 0)
   body=block
-> anonymous_method_expression ;;

   LPAREN
   ( (#anonymous_method_parameter=anonymous_method_parameter @ COMMA) | 0 )
   RPAREN
-> anonymous_method_signature ;;

   modifier=optional_parameter_modifier type=type variable_name=identifier
-> anonymous_method_parameter ;;


-- NEW EXPRESSION is actually three rules in one: array_creation_expression,
-- object_creation_expression and delegate_creation_expression.
-- But as they all contain a "type" rule in the same place, it's not
-- a good idea to try to tell them apart. Also, object creation and
-- delegate creation can derive the exact same token sequence.

   NEW type:type
   (  array_creation_expression=array_creation_expression_rest[type]
    | object_or_delegate_creation_expression=object_or_delegate_creation_expression_rest[type]
   )
-> new_expression ;;

-- The rest of object/delegate or array creation expressions.

   LPAREN argument_list_or_expression=optional_argument_list RPAREN
-> object_or_delegate_creation_expression_rest [
     argument member node type: type;
] ;;

 (
   array_initializer=array_initializer
 |
   LBRACKET (#expression=expression @ COMMA) RBRACKET
   ( 0 [: if (LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
            { break; }
        :] -- avoids swallowing the LBRACKETs in
           -- primary_suffix's element access part.
     #rank_specifier=rank_specifier
   )*
   (array_initializer=array_initializer | 0)
 )
-> array_creation_expression_rest [
     argument member node type: type;
] ;;


-- The TYPEOF EXPRESSION is nasty, because it either needs LL(k) lookahead
-- or a very ugly duplication of the type system. And when I say _very_ ugly,
-- I mean it. I tried it, and decided to go with the cleaner lookahead hack.

   TYPEOF LPAREN
   (
      ?[: LA(2).kind == Token_RPAREN :]
      VOID
        [: (*yynode)->typeof_type = typeof_expression::type_void;   :]
    |
      ?[: compatibility_mode() >= csharp20_compatibility :]
      try/rollback(
        unbound_type_name=unbound_type_name
          [: (*yynode)->typeof_type = typeof_expression::type_unbound_type_name; :]
      )
      catch(
        other_type=type
          [: (*yynode)->typeof_type = typeof_expression::type_type; :]
      )
    |
      other_type=type
        [: (*yynode)->typeof_type = typeof_expression::type_type;   :]
   )
   RPAREN
-> typeof_expression [
     member variable typeof_type: typeof_expression::typeof_expression_enum;
] ;;

   (  ?[: LA(2).kind == Token_SCOPE :] qualified_alias_label=identifier SCOPE
    | 0
   )
   #name_part=unbound_type_name_part @ DOT
-> unbound_type_name ;;

   identifier=identifier
   generic_dimension_specifier=generic_dimension_specifier
-> unbound_type_name_part ;;

   LESS_THAN [: (*yynode)->comma_count = 0; :]
   ( COMMA   [: (*yynode)->comma_count++;   :] )*
   GREATER_THAN
-> generic_dimension_specifier [
     member variable comma_count: int;
] ;;





--
-- All kinds of rules for types here.
--

-- The RETURN TYPE can only be used as return value, not in a declaration.

 (
   ?[: LA(2).kind != Token_STAR :] -- "void*" is a regular type in unsafe code
   VOID              [: (*yynode)->type = return_type::type_void;    :]
 |
   regular_type=type [: (*yynode)->type = return_type::type_regular; :]
 )
-> return_type [
     member variable type: return_type::return_type_enum;
] ;;

-- The regular TYPE recognizes the same set of tokens as the one in the C#
-- specification, but had to be refactored quite a bit. Looks different here.

   unmanaged_type=unmanaged_type   -- it's too cumbersome to track "unsafe",
   -- | managed_type=managed_type  -- so have it on by default
-> type ;;

   -- unsafe grammar extension: unmanaged type (includes all of the managed one)
   (  regular_type=non_array_type
        [: (*yynode)->type = pointer_type::type_regular;   :]
    | VOID STAR
        [: (*yynode)->type = pointer_type::type_void_star; :]
   )
   ( 0 [: if (yytoken == Token_LBRACKET &&
              LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
            { break; }
        :] -- prevents rank_specifier from swallowing the LBRACKETs in
           -- new_expression/array_creation_expression.
     #unmanaged_type_suffix=unmanaged_type_suffix
   )*
-> unmanaged_type [
     member variable type: pointer_type::pointer_type_enum;
] ;;

   -- unsafe grammar extension: pointer type
   (  regular_type=non_array_type
      #unmanaged_type_suffix=unmanaged_type_suffix
        [: (*yynode)->type = pointer_type::type_regular;   :]
    | VOID STAR
        [: (*yynode)->type = pointer_type::type_void_star; :]
   )
   ( 0 [: if (yytoken == Token_LBRACKET &&
              LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
            { break; }
        :] -- prevents rank_specifier from swallowing the LBRACKETs in
           -- new_expression/array_creation_expression.
     #unmanaged_type_suffix=unmanaged_type_suffix
   )*
-> pointer_type [
     member variable type: pointer_type::pointer_type_enum;
     member variable star_count: int;
] ;;

   STAR                            [: (*yynode)->type = unmanaged_type_suffix::type_star;           :]
 | rank_specifier=rank_specifier   [: (*yynode)->type = unmanaged_type_suffix::type_rank_specifier; :]
-> unmanaged_type_suffix [
     member variable type: unmanaged_type_suffix::suffix_type;
] ;;

   non_array_type=non_array_type
   ( 0 [: if (LA(2).kind != Token_COMMA && LA(2).kind != Token_RBRACKET)
            { break; }
        :] -- avoids swallowing the LBRACKETs in
           -- new_expression/array_creation_expression.
     #rank_specifier=rank_specifier
   )*
-> managed_type ;;

   non_array_type=non_array_type (#rank_specifier=rank_specifier)+
-> array_type ;;

   LBRACKET [: (*yynode)->dimension_seperator_count = 0; :]
   ( COMMA  [: (*yynode)->dimension_seperator_count++;   :] )*
   RBRACKET
-> rank_specifier [
     member variable dimension_seperator_count: int;
] ;;

   builtin_class_type=builtin_class_type
 | optionally_nullable_type=optionally_nullable_type
-> non_array_type ;;

   type_name=type_name
 | builtin_class_type=builtin_class_type
-> class_type ;;

   OBJECT [: (*yynode)->type = builtin_class_type::type_object; :]
 | STRING [: (*yynode)->type = builtin_class_type::type_string; :]
-> builtin_class_type [
     member variable type: builtin_class_type::builtin_class_type_enum;
] ;;

-- NULLABLE TYPES are new in C# 2.0 and need to be expressed a little bit
-- differently than in LALR grammars like in the C# specification.

   non_nullable_type=non_nullable_type
   (  ?[: compatibility_mode() >= csharp20_compatibility :]
      QUESTION [: (*yynode)->nullable = true;  :]
    |
      0        [: (*yynode)->nullable = false; :]
   )
-> optionally_nullable_type [
     member variable nullable: bool;
] ;;

   type_name=type_name
 | simple_type=simple_type
-> non_nullable_type ;;


-- Now for SIMPLE TYPES, this is easier ;)

 (
   numeric_type=numeric_type
     [: (*yynode)->type = simple_type::type_numeric; :]
 | BOOL
     [: (*yynode)->type = simple_type::type_bool; :]
 )
-> simple_type [
     member variable type: simple_type::simple_type_enum;
] ;;


-- NUMERIC TYPES include INTEGRAL TYPES, FLOATING POINT TYPES, and DECIMAL.

 (
   int_type=integral_type
     [: (*yynode)->type = numeric_type::type_integral; :]
 | float_type=floating_point_type
     [: (*yynode)->type = numeric_type::type_floating_point; :]
 | DECIMAL
     [: (*yynode)->type = numeric_type::type_decimal; :]
 )
-> numeric_type [
     member variable type: numeric_type::numeric_type_enum;
] ;;

 (
   SBYTE   [: (*yynode)->type = integral_type::type_sbyte;  :]
 | BYTE    [: (*yynode)->type = integral_type::type_byte;   :]
 | SHORT   [: (*yynode)->type = integral_type::type_short;  :]
 | USHORT  [: (*yynode)->type = integral_type::type_ushort; :]
 | INT     [: (*yynode)->type = integral_type::type_int;    :]
 | UINT    [: (*yynode)->type = integral_type::type_uint;   :]
 | LONG    [: (*yynode)->type = integral_type::type_long;   :]
 | ULONG   [: (*yynode)->type = integral_type::type_ulong;  :]
 | CHAR    [: (*yynode)->type = integral_type::type_char;   :]
 )
-> integral_type [
     member variable type: integral_type::integral_type_enum;
] ;;

 (
   FLOAT   [: (*yynode)->type = floating_point_type::type_float;  :]
 | DOUBLE  [: (*yynode)->type = floating_point_type::type_double; :]
 )
-> floating_point_type [
     member variable type: floating_point_type::floating_point_type_enum;
] ;;


-- TYPE NAMES and NAMESPACE NAMES are the same thing,
-- essentially qualified identifiers with optional type arguments.

   namespace_name=namespace_or_type_name
-> namespace_name ;;

   type_name=namespace_or_type_name
-> type_name ;;

   (  ?[: LA(2).kind == Token_SCOPE :] qualified_alias_label=identifier SCOPE
    | 0
   )
   #name_part=namespace_or_type_name_part @ DOT
-> namespace_or_type_name ;;

   identifier=identifier
   try/rollback(
      ?[: compatibility_mode() >= csharp20_compatibility :]
      type_arguments=type_arguments
    | 0
   ) catch(0)
-> namespace_or_type_name_part ;;

   type_name=namespace_or_type_name_safe
-> type_name_safe ;;

   (  ?[: LA(2).kind == Token_SCOPE :] qualified_alias_label=identifier SCOPE
    | 0
   )
   #name_part=namespace_or_type_name_part
   ( 0 [: if (LA(2).kind != Token_IDENTIFIER) break; :] -- exit the "star loop"
     DOT #name_part=namespace_or_type_name_part
   )*
-> namespace_or_type_name_safe ;;


-- QUALIFIED IDENTIFIERs are either qualified ones or raw identifiers.
-- In the C# grammar, they're only used as namespace names.

   #name=identifier @ DOT
-> qualified_identifier ;;




--
-- MODIFIERS, KEYWORDS, LITERALS, and the IDENTIFIER wrapper
--

 (
   REF       [: (*yynode)->parameter_type = parameter::reference_parameter; :]
 | OUT       [: (*yynode)->parameter_type = parameter::output_parameter;    :]
 | 0         [: (*yynode)->parameter_type = parameter::value_parameter;     :]
 )
-> optional_parameter_modifier [
     member variable parameter_type: parameter::parameter_type_enum;
] ;;

-- These are all the modifiers that can occur in front of type and type member
-- declarations. They are not valid in every combination, this has to be
-- checked seperately after parsing this rule.

 (
   PUBLIC     [: (*yynode)->access_policy = access_policy::access_public;  :]
 | PRIVATE    [: (*yynode)->access_policy = access_policy::access_private; :]
 | PROTECTED
     [: if ((*yynode)->access_policy == access_policy::access_internal)
          (*yynode)->access_policy = access_policy::access_protected_internal;
        else
          (*yynode)->access_policy = access_policy::access_protected;
     :]
 | INTERNAL
     [: if ((*yynode)->access_policy == access_policy::access_protected)
          (*yynode)->access_policy = access_policy::access_protected_internal;
        else
          (*yynode)->access_policy = access_policy::access_internal;
     :]
 | NEW        [: (*yynode)->modifiers |= modifiers::mod_new;        :]
 | ABSTRACT   [: (*yynode)->modifiers |= modifiers::mod_abstract;   :]
 | SEALED     [: (*yynode)->modifiers |= modifiers::mod_sealed;     :]
 | STATIC     [: (*yynode)->modifiers |= modifiers::mod_static;     :]
 | READONLY   [: (*yynode)->modifiers |= modifiers::mod_readonly;   :]
 | VOLATILE   [: (*yynode)->modifiers |= modifiers::mod_volatile;   :]
 | VIRTUAL    [: (*yynode)->modifiers |= modifiers::mod_virtual;    :]
 | OVERRIDE   [: (*yynode)->modifiers |= modifiers::mod_override;   :]
 | EXTERN     [: (*yynode)->modifiers |= modifiers::mod_extern;     :]
 -- unsafe grammar extension: "unsafe" keyword
 | UNSAFE     [: (*yynode)->modifiers |= modifiers::mod_unsafe;     :]
 -- unspecified unsafe modifier, but used by MS and Mono, so accept it here as well:
 | ?[: compatibility_mode() >= csharp20_compatibility :]
   FIXED      [: (*yynode)->modifiers |= modifiers::mod_fixed;      :]
 )*
-> optional_modifiers [
  member variable modifiers: unsigned int; -- using the modifier_enum values
  member variable access_policy: access_policy::access_policy_enum;
] ;;

 (
   keyword=ABSTRACT | keyword=AS | keyword=BASE | keyword=BOOL
 | keyword=BREAK | keyword=BYTE | keyword=CASE | keyword=CATCH | keyword=CHAR
 | keyword=CHECKED | keyword=CLASS | keyword=CONST | keyword=CONTINUE
 | keyword=DECIMAL | keyword=DEFAULT | keyword=DELEGATE | keyword=DO
 | keyword=DOUBLE | keyword=ELSE | keyword=ENUM | keyword=EVENT
 | keyword=EXPLICIT | keyword=EXTERN | keyword=FINALLY | keyword=FIXED
 | keyword=FLOAT | keyword=FOREACH | keyword=FOR | keyword=GOTO | keyword=IF
 | keyword=IMPLICIT | keyword=IN | keyword=INT | keyword=INTERFACE
 | keyword=INTERNAL | keyword=IS | keyword=LOCK | keyword=LONG
 | keyword=NAMESPACE | keyword=NEW | keyword=OBJECT | keyword=OPERATOR
 | keyword=OUT | keyword=OVERRIDE | keyword=PARAMS | keyword=PRIVATE
 | keyword=PROTECTED | keyword=PUBLIC | keyword=READONLY | keyword=REF
 | keyword=RETURN | keyword=SBYTE | keyword=SEALED | keyword=SHORT
 | keyword=SIZEOF | keyword=STACKALLOC | keyword=STATIC | keyword=STRING
 | keyword=STRUCT | keyword=SWITCH | keyword=THIS | keyword=THROW | keyword=TRY
 | keyword=TYPEOF | keyword=UINT | keyword=ULONG | keyword=UNCHECKED
 | keyword=UNSAFE | keyword=USHORT | keyword=USING | keyword=VIRTUAL
 | keyword=VOID | keyword=VOLATILE | keyword=WHILE
 )
-> keyword ;;

 (
   ident=IDENTIFIER
 | ident=ADD
 | ident=ALIAS
 | ident=GET
 | ident=GLOBAL
 | ident=PARTIAL
 | ident=REMOVE
 | ident=SET
 | ident=VALUE
 | ident=WHERE
 | ident=YIELD
 | ident=ASSEMBLY
 )
-> identifier ;;

 (
   TRUE   [: (*yynode)->literal_type = literal::type_true;  :]
 | FALSE  [: (*yynode)->literal_type = literal::type_false; :]
 | NULL   [: (*yynode)->literal_type = literal::type_null;  :]
 |
   integer_literal=INTEGER_LITERAL
   [: (*yynode)->literal_type = literal::type_integer;  :]
 |
   floating_point_literal=REAL_LITERAL
   [: (*yynode)->literal_type = literal::type_real;  :]
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
#include "csharp_lexer.h"


namespace csharp
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
    while (kind != parser::Token_EOF);

    this->yylex(); // produce the look ahead token
}


parser::csharp_compatibility_mode parser::compatibility_mode()
{
    return _M_compatibility_mode;
}
void parser::set_compatibility_mode( parser::csharp_compatibility_mode mode )
{
    _M_compatibility_mode = mode;
}

void parser::pp_define_symbol( std::string symbol_name )
{
    _M_pp_defined_symbols.insert( symbol_name );
}

void parser::pp_undefine_symbol( std::string symbol_name )
{
    _M_pp_defined_symbols.erase( symbol_name );
}

bool parser::pp_is_symbol_defined( std::string symbol_name )
{
    return (_M_pp_defined_symbols.find(symbol_name) != _M_pp_defined_symbols.end());
}


// Rather hackish solution for recognizing expressions like
// "a is sometype ? if_exp : else_exp", see conditional_expression.
// Needs three methods to fix parsing for about 0.2% of all C# source files.

bool parser::is_nullable_type( type_ast *type )
{
    if ( !type )
        return false;
    else if ( !type->unmanaged_type )
        return false;
    else if ( !type->unmanaged_type->regular_type || type->unmanaged_type->unmanaged_type_suffix_sequence )
        return false;
    else if ( !type->unmanaged_type->regular_type->optionally_nullable_type )
        return false;
    else if ( type->unmanaged_type->regular_type->optionally_nullable_type->nullable == false )
        return false;
    else // if ( optionally_nullable_type->nullable == true )
        return true;
}

// This method is only to be called after is_nullable_type(type) returns true,
// and therefore expects all the appropriate members not to be 0.
void parser::unset_nullable_type( type_ast *type )
{
    type->unmanaged_type->regular_type->optionally_nullable_type->nullable = false;
}

// This method expects null_coalescing_expression to be fully parsed and valid.
// (Otherwise, this method is not called at all.
type_ast *parser::last_relational_expression_rest_type(
        null_coalescing_expression_ast *null_coalescing_expression )
{
    relational_expression_ast *relexp =
        null_coalescing_expression
        ->expression_sequence->to_back()->element // gets a logical_or_expression
        ->expression_sequence->to_back()->element // gets a logical_and_expression
        ->expression_sequence->to_back()->element // gets a bit_or_expression
        ->expression_sequence->to_back()->element // gets a bit_xor_expression
        ->expression_sequence->to_back()->element // gets a bit_and_expression
        ->expression_sequence->to_back()->element // gets an equality_expression
        ->expression                              // gets a relational_expression
    ;

    if ( relexp->additional_expression_sequence != 0 )
        return relexp->additional_expression_sequence->to_back()->element->type;
    else
        return 0;
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

} // end of namespace csharp

:]
