// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef java_H_INCLUDED
#define java_H_INCLUDED

#include "java_ast.h"
#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-token-stream.h"

namespace java
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
        Token_ABSTRACT = 1000,
        Token_ASSERT = 1001,
        Token_ASSIGN = 1002,
        Token_AT = 1003,
        Token_BANG = 1004,
        Token_BIT_AND = 1005,
        Token_BIT_AND_ASSIGN = 1006,
        Token_BIT_OR = 1007,
        Token_BIT_OR_ASSIGN = 1008,
        Token_BIT_XOR = 1009,
        Token_BIT_XOR_ASSIGN = 1010,
        Token_BOOLEAN = 1011,
        Token_BREAK = 1012,
        Token_BYTE = 1013,
        Token_CASE = 1014,
        Token_CATCH = 1015,
        Token_CHAR = 1016,
        Token_CHARACTER_LITERAL = 1017,
        Token_CLASS = 1018,
        Token_COLON = 1019,
        Token_COMMA = 1020,
        Token_CONST = 1021,
        Token_CONTINUE = 1022,
        Token_DECREMENT = 1023,
        Token_DEFAULT = 1024,
        Token_DO = 1025,
        Token_DOT = 1026,
        Token_DOUBLE = 1027,
        Token_ELLIPSIS = 1028,
        Token_ELSE = 1029,
        Token_ENUM = 1030,
        Token_EOF = 1031,
        Token_EQUAL = 1032,
        Token_EXTENDS = 1033,
        Token_FALSE = 1034,
        Token_FINAL = 1035,
        Token_FINALLY = 1036,
        Token_FLOAT = 1037,
        Token_FLOATING_POINT_LITERAL = 1038,
        Token_FOR = 1039,
        Token_GOTO = 1040,
        Token_GREATER_EQUAL = 1041,
        Token_GREATER_THAN = 1042,
        Token_IDENTIFIER = 1043,
        Token_IF = 1044,
        Token_IMPLEMENTS = 1045,
        Token_IMPORT = 1046,
        Token_INCREMENT = 1047,
        Token_INSTANCEOF = 1048,
        Token_INT = 1049,
        Token_INTEGER_LITERAL = 1050,
        Token_INTERFACE = 1051,
        Token_INVALID = 1052,
        Token_LBRACE = 1053,
        Token_LBRACKET = 1054,
        Token_LESS_EQUAL = 1055,
        Token_LESS_THAN = 1056,
        Token_LOG_AND = 1057,
        Token_LOG_OR = 1058,
        Token_LONG = 1059,
        Token_LPAREN = 1060,
        Token_LSHIFT = 1061,
        Token_LSHIFT_ASSIGN = 1062,
        Token_MINUS = 1063,
        Token_MINUS_ASSIGN = 1064,
        Token_NATIVE = 1065,
        Token_NEW = 1066,
        Token_NOT_EQUAL = 1067,
        Token_NULL = 1068,
        Token_PACKAGE = 1069,
        Token_PLUS = 1070,
        Token_PLUS_ASSIGN = 1071,
        Token_PRIVATE = 1072,
        Token_PROTECTED = 1073,
        Token_PUBLIC = 1074,
        Token_QUESTION = 1075,
        Token_RBRACE = 1076,
        Token_RBRACKET = 1077,
        Token_REMAINDER = 1078,
        Token_REMAINDER_ASSIGN = 1079,
        Token_RETURN = 1080,
        Token_RPAREN = 1081,
        Token_SEMICOLON = 1082,
        Token_SHORT = 1083,
        Token_SIGNED_RSHIFT = 1084,
        Token_SIGNED_RSHIFT_ASSIGN = 1085,
        Token_SLASH = 1086,
        Token_SLASH_ASSIGN = 1087,
        Token_STAR = 1088,
        Token_STAR_ASSIGN = 1089,
        Token_STATIC = 1090,
        Token_STRICTFP = 1091,
        Token_STRING_LITERAL = 1092,
        Token_SUPER = 1093,
        Token_SWITCH = 1094,
        Token_SYNCHRONIZED = 1095,
        Token_THIS = 1096,
        Token_THROW = 1097,
        Token_THROWS = 1098,
        Token_TILDE = 1099,
        Token_TRANSIENT = 1100,
        Token_TRUE = 1101,
        Token_TRY = 1102,
        Token_UNSIGNED_RSHIFT = 1103,
        Token_UNSIGNED_RSHIFT_ASSIGN = 1104,
        Token_VOID = 1105,
        Token_VOLATILE = 1106,
        Token_WHILE = 1107,
        token_type_size
      }; // token_type_enum

      // user defined declarations:
    public:

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

    private:

      parser::java_compatibility_mode _M_compatibility_mode;

      struct parser_state
        {
          // ltCounter stores the amount of currently open type arguments rules,
          // all of which are beginning with a less than ("<") character.
          // This way, also SIGNED_RSHIFT (">>") and UNSIGNED_RSHIFT (">>>") can be used
          // to close type arguments rules, in addition to GREATER_THAN (">").
          int ltCounter;
        };
      parser_state _M_state;


    public:
      // The copy_current_state() and restore_state() methods are only declared
      // if you are using try blocks in your grammar, and have to be
      // implemented by yourself, and you also have to define a
      // "struct parser_state" inside a %parserclass directive.

      // This method should create a new parser_state object and return it,
      // or return 0 if no state variables need to be saved.
      parser_state *copy_current_state();

      // This method is only called for parser_state objects != 0
      // and should restore the parser state given as argument.
      void restore_state(parser_state *state);
      parser()
      {
        memory_pool = 0;
        token_stream = 0;
        yytoken = Token_EOF;
        yy_block_errors = false;

        // user defined constructor code:
        _M_compatibility_mode = java15_compatibility;
      }

      virtual ~parser()
      {}

      bool parse_additive_expression(additive_expression_ast **yynode);
      bool parse_additive_expression_rest(additive_expression_rest_ast **yynode);
      bool parse_annotation(annotation_ast **yynode);
      bool parse_annotation_arguments(annotation_arguments_ast **yynode);
      bool parse_annotation_element_array_initializer(annotation_element_array_initializer_ast **yynode);
      bool parse_annotation_element_array_value(annotation_element_array_value_ast **yynode);
      bool parse_annotation_element_value(annotation_element_value_ast **yynode);
      bool parse_annotation_element_value_pair(annotation_element_value_pair_ast **yynode);
      bool parse_annotation_method_declaration(annotation_method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_ast *return_type);
      bool parse_annotation_type_body(annotation_type_body_ast **yynode);
      bool parse_annotation_type_declaration(annotation_type_declaration_ast **yynode, optional_modifiers_ast *modifiers);
      bool parse_annotation_type_field(annotation_type_field_ast **yynode);
      bool parse_array_access(array_access_ast **yynode);
      bool parse_array_creator_rest(array_creator_rest_ast **yynode);
      bool parse_array_type_dot_class(array_type_dot_class_ast **yynode);
      bool parse_assert_statement(assert_statement_ast **yynode);
      bool parse_bit_and_expression(bit_and_expression_ast **yynode);
      bool parse_bit_or_expression(bit_or_expression_ast **yynode);
      bool parse_bit_xor_expression(bit_xor_expression_ast **yynode);
      bool parse_block(block_ast **yynode);
      bool parse_block_statement(block_statement_ast **yynode);
      bool parse_break_statement(break_statement_ast **yynode);
      bool parse_builtin_type(builtin_type_ast **yynode);
      bool parse_builtin_type_dot_class(builtin_type_dot_class_ast **yynode);
      bool parse_cast_expression(cast_expression_ast **yynode);
      bool parse_catch_clause(catch_clause_ast **yynode);
      bool parse_class_access_data(class_access_data_ast **yynode);
      bool parse_class_body(class_body_ast **yynode);
      bool parse_class_declaration(class_declaration_ast **yynode, optional_modifiers_ast *modifiers);
      bool parse_class_extends_clause(class_extends_clause_ast **yynode);
      bool parse_class_field(class_field_ast **yynode);
      bool parse_class_or_interface_type_name(class_or_interface_type_name_ast **yynode);
      bool parse_class_or_interface_type_name_part(class_or_interface_type_name_part_ast **yynode);
      bool parse_class_type(class_type_ast **yynode);
      bool parse_compilation_unit(compilation_unit_ast **yynode);
      bool parse_conditional_expression(conditional_expression_ast **yynode);
      bool parse_constructor_declaration(constructor_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters);
      bool parse_continue_statement(continue_statement_ast **yynode);
      bool parse_do_while_statement(do_while_statement_ast **yynode);
      bool parse_embedded_statement(embedded_statement_ast **yynode);
      bool parse_enum_body(enum_body_ast **yynode);
      bool parse_enum_constant(enum_constant_ast **yynode);
      bool parse_enum_constant_body(enum_constant_body_ast **yynode);
      bool parse_enum_constant_field(enum_constant_field_ast **yynode);
      bool parse_enum_declaration(enum_declaration_ast **yynode, optional_modifiers_ast *modifiers);
      bool parse_equality_expression(equality_expression_ast **yynode);
      bool parse_equality_expression_rest(equality_expression_rest_ast **yynode);
      bool parse_expression(expression_ast **yynode);
      bool parse_for_clause_traditional_rest(for_clause_traditional_rest_ast **yynode);
      bool parse_for_control(for_control_ast **yynode);
      bool parse_for_statement(for_statement_ast **yynode);
      bool parse_foreach_declaration_data(foreach_declaration_data_ast **yynode, parameter_declaration_ast *foreach_parameter, expression_ast *iterable_expression);
      bool parse_identifier(identifier_ast **yynode);
      bool parse_if_statement(if_statement_ast **yynode);
      bool parse_implements_clause(implements_clause_ast **yynode);
      bool parse_import_declaration(import_declaration_ast **yynode);
      bool parse_interface_body(interface_body_ast **yynode);
      bool parse_interface_declaration(interface_declaration_ast **yynode, optional_modifiers_ast *modifiers);
      bool parse_interface_extends_clause(interface_extends_clause_ast **yynode);
      bool parse_interface_field(interface_field_ast **yynode);
      bool parse_interface_method_declaration(interface_method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters, type_ast *return_type);
      bool parse_labeled_statement(labeled_statement_ast **yynode);
      bool parse_literal(literal_ast **yynode);
      bool parse_logical_and_expression(logical_and_expression_ast **yynode);
      bool parse_logical_or_expression(logical_or_expression_ast **yynode);
      bool parse_mandatory_array_builtin_type(mandatory_array_builtin_type_ast **yynode);
      bool parse_mandatory_declarator_brackets(mandatory_declarator_brackets_ast **yynode);
      bool parse_method_call_data(method_call_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, identifier_ast *method_name, optional_argument_list_ast *arguments);
      bool parse_method_declaration(method_declaration_ast **yynode, optional_modifiers_ast *modifiers, type_parameters_ast *type_parameters, type_ast *return_type);
      bool parse_multiplicative_expression(multiplicative_expression_ast **yynode);
      bool parse_multiplicative_expression_rest(multiplicative_expression_rest_ast **yynode);
      bool parse_new_expression(new_expression_ast **yynode);
      bool parse_non_array_type(non_array_type_ast **yynode);
      bool parse_non_wildcard_type_arguments(non_wildcard_type_arguments_ast **yynode);
      bool parse_optional_argument_list(optional_argument_list_ast **yynode);
      bool parse_optional_array_builtin_type(optional_array_builtin_type_ast **yynode);
      bool parse_optional_declarator_brackets(optional_declarator_brackets_ast **yynode);
      bool parse_optional_modifiers(optional_modifiers_ast **yynode);
      bool parse_optional_parameter_declaration_list(optional_parameter_declaration_list_ast **yynode);
      bool parse_optional_parameter_modifiers(optional_parameter_modifiers_ast **yynode);
      bool parse_package_declaration(package_declaration_ast **yynode);
      bool parse_parameter_declaration(parameter_declaration_ast **yynode);
      bool parse_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast **yynode, bool* ellipsis_occurred);
      bool parse_postfix_operator(postfix_operator_ast **yynode);
      bool parse_primary_atom(primary_atom_ast **yynode);
      bool parse_primary_expression(primary_expression_ast **yynode);
      bool parse_primary_selector(primary_selector_ast **yynode);
      bool parse_qualified_identifier(qualified_identifier_ast **yynode);
      bool parse_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast **yynode);
      bool parse_relational_expression(relational_expression_ast **yynode);
      bool parse_relational_expression_rest(relational_expression_rest_ast **yynode);
      bool parse_return_statement(return_statement_ast **yynode);
      bool parse_shift_expression(shift_expression_ast **yynode);
      bool parse_shift_expression_rest(shift_expression_rest_ast **yynode);
      bool parse_simple_name_access_data(simple_name_access_data_ast **yynode, identifier_ast *name);
      bool parse_statement_expression(statement_expression_ast **yynode);
      bool parse_super_access_data(super_access_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, super_suffix_ast *super_suffix);
      bool parse_super_suffix(super_suffix_ast **yynode);
      bool parse_switch_label(switch_label_ast **yynode);
      bool parse_switch_section(switch_section_ast **yynode);
      bool parse_switch_statement(switch_statement_ast **yynode);
      bool parse_synchronized_statement(synchronized_statement_ast **yynode);
      bool parse_this_access_data(this_access_data_ast **yynode);
      bool parse_this_call_data(this_call_data_ast **yynode, non_wildcard_type_arguments_ast *type_arguments, optional_argument_list_ast *arguments);
      bool parse_throw_statement(throw_statement_ast **yynode);
      bool parse_throws_clause(throws_clause_ast **yynode);
      bool parse_try_statement(try_statement_ast **yynode);
      bool parse_type(type_ast **yynode);
      bool parse_type_argument(type_argument_ast **yynode);
      bool parse_type_argument_type(type_argument_type_ast **yynode);
      bool parse_type_arguments(type_arguments_ast **yynode);
      bool parse_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast **yynode);
      bool parse_type_declaration(type_declaration_ast **yynode);
      bool parse_type_parameter(type_parameter_ast **yynode);
      bool parse_type_parameters(type_parameters_ast **yynode);
      bool parse_unary_expression(unary_expression_ast **yynode);
      bool parse_unary_expression_not_plusminus(unary_expression_not_plusminus_ast **yynode);
      bool parse_variable_array_initializer(variable_array_initializer_ast **yynode);
      bool parse_variable_declaration(variable_declaration_ast **yynode);
      bool parse_variable_declaration_data(variable_declaration_data_ast **yynode, optional_modifiers_ast *modifiers, type_ast *type, const list_node<variable_declarator_ast *> *declarator_sequence);
      bool parse_variable_declaration_rest(variable_declaration_rest_ast **yynode);
      bool parse_variable_declaration_split_data(variable_declaration_split_data_ast **yynode, parameter_declaration_ast *parameter_declaration, variable_declaration_rest_ast *rest);
      bool parse_variable_declaration_statement(variable_declaration_statement_ast **yynode);
      bool parse_variable_declarator(variable_declarator_ast **yynode);
      bool parse_variable_initializer(variable_initializer_ast **yynode);
      bool parse_while_statement(while_statement_ast **yynode);
      bool parse_wildcard_type(wildcard_type_ast **yynode);
      bool parse_wildcard_type_bounds(wildcard_type_bounds_ast **yynode);
    };

} // end of namespace java

#endif


