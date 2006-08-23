// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_H_INCLUDED
#define csharp_H_INCLUDED

#include "csharp_ast.h"
#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-token-stream.h"

namespace csharp
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
        Token_ADD = 1001,
        Token_ALIAS = 1002,
        Token_ARROW_RIGHT = 1003,
        Token_AS = 1004,
        Token_ASSEMBLY = 1005,
        Token_ASSIGN = 1006,
        Token_BANG = 1007,
        Token_BASE = 1008,
        Token_BIT_AND = 1009,
        Token_BIT_AND_ASSIGN = 1010,
        Token_BIT_OR = 1011,
        Token_BIT_OR_ASSIGN = 1012,
        Token_BIT_XOR = 1013,
        Token_BIT_XOR_ASSIGN = 1014,
        Token_BOOL = 1015,
        Token_BREAK = 1016,
        Token_BYTE = 1017,
        Token_CASE = 1018,
        Token_CATCH = 1019,
        Token_CHAR = 1020,
        Token_CHARACTER_LITERAL = 1021,
        Token_CHECKED = 1022,
        Token_CLASS = 1023,
        Token_COLON = 1024,
        Token_COMMA = 1025,
        Token_CONST = 1026,
        Token_CONTINUE = 1027,
        Token_DECIMAL = 1028,
        Token_DECREMENT = 1029,
        Token_DEFAULT = 1030,
        Token_DELEGATE = 1031,
        Token_DO = 1032,
        Token_DOT = 1033,
        Token_DOUBLE = 1034,
        Token_ELSE = 1035,
        Token_ENUM = 1036,
        Token_EOF = 1037,
        Token_EQUAL = 1038,
        Token_EVENT = 1039,
        Token_EXPLICIT = 1040,
        Token_EXTERN = 1041,
        Token_FALSE = 1042,
        Token_FINALLY = 1043,
        Token_FIXED = 1044,
        Token_FLOAT = 1045,
        Token_FOR = 1046,
        Token_FOREACH = 1047,
        Token_GET = 1048,
        Token_GLOBAL = 1049,
        Token_GOTO = 1050,
        Token_GREATER_EQUAL = 1051,
        Token_GREATER_THAN = 1052,
        Token_IDENTIFIER = 1053,
        Token_IF = 1054,
        Token_IMPLICIT = 1055,
        Token_IN = 1056,
        Token_INCREMENT = 1057,
        Token_INT = 1058,
        Token_INTEGER_LITERAL = 1059,
        Token_INTERFACE = 1060,
        Token_INTERNAL = 1061,
        Token_INVALID = 1062,
        Token_IS = 1063,
        Token_LBRACE = 1064,
        Token_LBRACKET = 1065,
        Token_LESS_EQUAL = 1066,
        Token_LESS_THAN = 1067,
        Token_LOCK = 1068,
        Token_LOG_AND = 1069,
        Token_LOG_OR = 1070,
        Token_LONG = 1071,
        Token_LPAREN = 1072,
        Token_LSHIFT = 1073,
        Token_LSHIFT_ASSIGN = 1074,
        Token_MINUS = 1075,
        Token_MINUS_ASSIGN = 1076,
        Token_NAMESPACE = 1077,
        Token_NEW = 1078,
        Token_NOT_EQUAL = 1079,
        Token_NULL = 1080,
        Token_OBJECT = 1081,
        Token_OPERATOR = 1082,
        Token_OUT = 1083,
        Token_OVERRIDE = 1084,
        Token_PARAMS = 1085,
        Token_PARTIAL = 1086,
        Token_PLUS = 1087,
        Token_PLUS_ASSIGN = 1088,
        Token_PRIVATE = 1089,
        Token_PROTECTED = 1090,
        Token_PUBLIC = 1091,
        Token_QUESTION = 1092,
        Token_QUESTIONQUESTION = 1093,
        Token_RBRACE = 1094,
        Token_RBRACKET = 1095,
        Token_READONLY = 1096,
        Token_REAL_LITERAL = 1097,
        Token_REF = 1098,
        Token_REMAINDER = 1099,
        Token_REMAINDER_ASSIGN = 1100,
        Token_REMOVE = 1101,
        Token_RETURN = 1102,
        Token_RPAREN = 1103,
        Token_RSHIFT = 1104,
        Token_RSHIFT_ASSIGN = 1105,
        Token_SBYTE = 1106,
        Token_SCOPE = 1107,
        Token_SEALED = 1108,
        Token_SEMICOLON = 1109,
        Token_SET = 1110,
        Token_SHORT = 1111,
        Token_SIZEOF = 1112,
        Token_SLASH = 1113,
        Token_SLASH_ASSIGN = 1114,
        Token_STACKALLOC = 1115,
        Token_STAR = 1116,
        Token_STAR_ASSIGN = 1117,
        Token_STATIC = 1118,
        Token_STRING = 1119,
        Token_STRING_LITERAL = 1120,
        Token_STRUCT = 1121,
        Token_SWITCH = 1122,
        Token_THIS = 1123,
        Token_THROW = 1124,
        Token_TILDE = 1125,
        Token_TRUE = 1126,
        Token_TRY = 1127,
        Token_TYPEOF = 1128,
        Token_UINT = 1129,
        Token_ULONG = 1130,
        Token_UNCHECKED = 1131,
        Token_UNSAFE = 1132,
        Token_USHORT = 1133,
        Token_USING = 1134,
        Token_VALUE = 1135,
        Token_VIRTUAL = 1136,
        Token_VOID = 1137,
        Token_VOLATILE = 1138,
        Token_WHERE = 1139,
        Token_WHILE = 1140,
        Token_YIELD = 1141,
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

    protected:

      friend class ::csharp_pp::handler_visitor; // calls the pp_*() methods

      /** Called when an #error or #warning directive has been found.
       *  @param type   Either parser::error or parser::warning.
       *  @param label  The error/warning text.
       */
      virtual void pp_diagnostic( parser::problem_type /*type*/, std::string /*message*/ )
      {}
      virtual void pp_diagnostic( parser::problem_type /*type*/ )
      {}

    private:

      void pp_undefine_symbol( std::string symbol_name );
      bool pp_is_symbol_defined( std::string symbol_name );

      parser::csharp_compatibility_mode _M_compatibility_mode;
      std::set <std::string>
      _M_pp_defined_symbols;

      struct parser_state
        {
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

        _M_compatibility_mode = csharp20_compatibility;

      }

      virtual ~parser()
      {}

      bool parse_accessor_declarations(accessor_declarations_ast **yynode);
      bool parse_accessor_modifier(accessor_modifier_ast **yynode);
      bool parse_additive_expression(additive_expression_ast **yynode);
      bool parse_additive_expression_rest(additive_expression_rest_ast **yynode);
      bool parse_anonymous_method_expression(anonymous_method_expression_ast **yynode);
      bool parse_anonymous_method_parameter(anonymous_method_parameter_ast **yynode);
      bool parse_anonymous_method_signature(anonymous_method_signature_ast **yynode);
      bool parse_argument(argument_ast **yynode);
      bool parse_array_creation_expression_rest(array_creation_expression_rest_ast **yynode, type_ast *type);
      bool parse_array_initializer(array_initializer_ast **yynode);
      bool parse_array_type(array_type_ast **yynode);
      bool parse_attribute(attribute_ast **yynode);
      bool parse_attribute_arguments(attribute_arguments_ast **yynode);
      bool parse_attribute_section(attribute_section_ast **yynode);
      bool parse_attribute_target(attribute_target_ast **yynode);
      bool parse_base_access(base_access_ast **yynode);
      bool parse_bit_and_expression(bit_and_expression_ast **yynode);
      bool parse_bit_or_expression(bit_or_expression_ast **yynode);
      bool parse_bit_xor_expression(bit_xor_expression_ast **yynode);
      bool parse_block(block_ast **yynode);
      bool parse_block_statement(block_statement_ast **yynode);
      bool parse_boolean_expression(boolean_expression_ast **yynode);
      bool parse_break_statement(break_statement_ast **yynode);
      bool parse_builtin_class_type(builtin_class_type_ast **yynode);
      bool parse_cast_expression(cast_expression_ast **yynode);
      bool parse_catch_clauses(catch_clauses_ast **yynode);
      bool parse_checked_statement(checked_statement_ast **yynode);
      bool parse_class_base(class_base_ast **yynode);
      bool parse_class_body(class_body_ast **yynode);
      bool parse_class_declaration(class_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial);
      bool parse_class_member_declaration(class_member_declaration_ast **yynode);
      bool parse_class_or_struct_member_declaration(class_or_struct_member_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_class_type(class_type_ast **yynode);
      bool parse_compilation_unit(compilation_unit_ast **yynode);
      bool parse_conditional_expression(conditional_expression_ast **yynode);
      bool parse_constant_declaration(constant_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_constant_declaration_data(constant_declaration_data_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, const list_node<constant_declarator_ast *> *constant_declarator_sequence);
      bool parse_constant_declarator(constant_declarator_ast **yynode);
      bool parse_constant_expression(constant_expression_ast **yynode);
      bool parse_constructor_constraint(constructor_constraint_ast **yynode);
      bool parse_constructor_declaration(constructor_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_constructor_initializer(constructor_initializer_ast **yynode);
      bool parse_continue_statement(continue_statement_ast **yynode);
      bool parse_conversion_operator_declaration(conversion_operator_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_delegate_declaration(delegate_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_do_while_statement(do_while_statement_ast **yynode);
      bool parse_embedded_statement(embedded_statement_ast **yynode);
      bool parse_enum_base(enum_base_ast **yynode);
      bool parse_enum_body(enum_body_ast **yynode);
      bool parse_enum_declaration(enum_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_enum_member_declaration(enum_member_declaration_ast **yynode);
      bool parse_equality_expression(equality_expression_ast **yynode);
      bool parse_equality_expression_rest(equality_expression_rest_ast **yynode);
      bool parse_event_accessor_declaration(event_accessor_declaration_ast **yynode, optional_attribute_sections_ast *attributes, block_ast *body);
      bool parse_event_accessor_declarations(event_accessor_declarations_ast **yynode);
      bool parse_event_declaration(event_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_expression(expression_ast **yynode);
      bool parse_extern_alias_directive(extern_alias_directive_ast **yynode);
      bool parse_finalizer_declaration(finalizer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_fixed_pointer_declarator(fixed_pointer_declarator_ast **yynode);
      bool parse_fixed_statement(fixed_statement_ast **yynode);
      bool parse_floating_point_type(floating_point_type_ast **yynode);
      bool parse_for_control(for_control_ast **yynode);
      bool parse_for_statement(for_statement_ast **yynode);
      bool parse_foreach_statement(foreach_statement_ast **yynode);
      bool parse_formal_parameter(formal_parameter_ast **yynode, bool* parameter_array_occurred);
      bool parse_formal_parameter_list(formal_parameter_list_ast **yynode);
      bool parse_general_catch_clause(general_catch_clause_ast **yynode);
      bool parse_generic_dimension_specifier(generic_dimension_specifier_ast **yynode);
      bool parse_global_attribute_section(global_attribute_section_ast **yynode);
      bool parse_goto_statement(goto_statement_ast **yynode);
      bool parse_identifier(identifier_ast **yynode);
      bool parse_if_statement(if_statement_ast **yynode);
      bool parse_indexer_declaration(indexer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, type_name_safe_ast *interface_type);
      bool parse_integral_type(integral_type_ast **yynode);
      bool parse_interface_accessors(interface_accessors_ast **yynode);
      bool parse_interface_base(interface_base_ast **yynode);
      bool parse_interface_body(interface_body_ast **yynode);
      bool parse_interface_declaration(interface_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial);
      bool parse_interface_event_declaration(interface_event_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new);
      bool parse_interface_indexer_declaration(interface_indexer_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, type_ast *type);
      bool parse_interface_member_declaration(interface_member_declaration_ast **yynode);
      bool parse_interface_method_declaration(interface_method_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, return_type_ast *return_type, identifier_ast *method_name);
      bool parse_interface_property_declaration(interface_property_declaration_ast **yynode, optional_attribute_sections_ast *attributes, bool decl_new, type_ast *type, identifier_ast *property_name);
      bool parse_keyword(keyword_ast **yynode);
      bool parse_labeled_statement(labeled_statement_ast **yynode);
      bool parse_literal(literal_ast **yynode);
      bool parse_local_constant_declaration(local_constant_declaration_ast **yynode);
      bool parse_local_variable_declaration(local_variable_declaration_ast **yynode);
      bool parse_local_variable_declaration_statement(local_variable_declaration_statement_ast **yynode);
      bool parse_lock_statement(lock_statement_ast **yynode);
      bool parse_logical_and_expression(logical_and_expression_ast **yynode);
      bool parse_logical_or_expression(logical_or_expression_ast **yynode);
      bool parse_managed_type(managed_type_ast **yynode);
      bool parse_method_declaration(method_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, return_type_ast *return_type, type_name_safe_ast *method_name);
      bool parse_multiplicative_expression(multiplicative_expression_ast **yynode);
      bool parse_multiplicative_expression_rest(multiplicative_expression_rest_ast **yynode);
      bool parse_named_argument(named_argument_ast **yynode);
      bool parse_namespace_body(namespace_body_ast **yynode);
      bool parse_namespace_declaration(namespace_declaration_ast **yynode);
      bool parse_namespace_member_declaration(namespace_member_declaration_ast **yynode);
      bool parse_namespace_name(namespace_name_ast **yynode);
      bool parse_namespace_or_type_name(namespace_or_type_name_ast **yynode);
      bool parse_namespace_or_type_name_part(namespace_or_type_name_part_ast **yynode);
      bool parse_namespace_or_type_name_safe(namespace_or_type_name_safe_ast **yynode);
      bool parse_new_expression(new_expression_ast **yynode);
      bool parse_non_array_type(non_array_type_ast **yynode);
      bool parse_non_nullable_type(non_nullable_type_ast **yynode);
      bool parse_null_coalescing_expression(null_coalescing_expression_ast **yynode);
      bool parse_numeric_type(numeric_type_ast **yynode);
      bool parse_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast **yynode, type_ast *type);
      bool parse_optional_argument_list(optional_argument_list_ast **yynode);
      bool parse_optional_attribute_sections(optional_attribute_sections_ast **yynode);
      bool parse_optional_modifiers(optional_modifiers_ast **yynode);
      bool parse_optional_parameter_modifier(optional_parameter_modifier_ast **yynode);
      bool parse_optionally_nullable_type(optionally_nullable_type_ast **yynode);
      bool parse_overloadable_binary_only_operator(overloadable_binary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_overloadable_unary_only_operator(overloadable_unary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_pointer_type(pointer_type_ast **yynode);
      bool parse_positional_argument(positional_argument_ast **yynode);
      bool parse_predefined_type(predefined_type_ast **yynode);
      bool parse_primary_atom(primary_atom_ast **yynode);
      bool parse_primary_expression(primary_expression_ast **yynode);
      bool parse_primary_or_secondary_constraint(primary_or_secondary_constraint_ast **yynode);
      bool parse_primary_suffix(primary_suffix_ast **yynode);
      bool parse_property_declaration(property_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, type_name_safe_ast *property_name);
      bool parse_qualified_identifier(qualified_identifier_ast **yynode);
      bool parse_rank_specifier(rank_specifier_ast **yynode);
      bool parse_relational_expression(relational_expression_ast **yynode);
      bool parse_relational_expression_rest(relational_expression_rest_ast **yynode);
      bool parse_resource_acquisition(resource_acquisition_ast **yynode);
      bool parse_return_statement(return_statement_ast **yynode);
      bool parse_return_type(return_type_ast **yynode);
      bool parse_secondary_constraint(secondary_constraint_ast **yynode);
      bool parse_shift_expression(shift_expression_ast **yynode);
      bool parse_shift_expression_rest(shift_expression_rest_ast **yynode);
      bool parse_simple_name_or_member_access(simple_name_or_member_access_ast **yynode);
      bool parse_simple_type(simple_type_ast **yynode);
      bool parse_specific_catch_clause(specific_catch_clause_ast **yynode);
      bool parse_stackalloc_initializer(stackalloc_initializer_ast **yynode);
      bool parse_statement_expression(statement_expression_ast **yynode);
      bool parse_struct_body(struct_body_ast **yynode);
      bool parse_struct_declaration(struct_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial);
      bool parse_struct_member_declaration(struct_member_declaration_ast **yynode);
      bool parse_switch_label(switch_label_ast **yynode);
      bool parse_switch_section(switch_section_ast **yynode);
      bool parse_switch_statement(switch_statement_ast **yynode);
      bool parse_throw_statement(throw_statement_ast **yynode);
      bool parse_try_statement(try_statement_ast **yynode);
      bool parse_type(type_ast **yynode);
      bool parse_type_arguments(type_arguments_ast **yynode);
      bool parse_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast **yynode);
      bool parse_type_declaration(type_declaration_ast **yynode);
      bool parse_type_declaration_rest(type_declaration_rest_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers);
      bool parse_type_name(type_name_ast **yynode);
      bool parse_type_name_safe(type_name_safe_ast **yynode);
      bool parse_type_parameter(type_parameter_ast **yynode);
      bool parse_type_parameter_constraints(type_parameter_constraints_ast **yynode);
      bool parse_type_parameter_constraints_clause(type_parameter_constraints_clause_ast **yynode);
      bool parse_type_parameters(type_parameters_ast **yynode);
      bool parse_typeof_expression(typeof_expression_ast **yynode);
      bool parse_unary_expression(unary_expression_ast **yynode);
      bool parse_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *return_type);
      bool parse_unbound_type_name(unbound_type_name_ast **yynode);
      bool parse_unbound_type_name_part(unbound_type_name_part_ast **yynode);
      bool parse_unchecked_statement(unchecked_statement_ast **yynode);
      bool parse_unmanaged_type(unmanaged_type_ast **yynode);
      bool parse_unmanaged_type_suffix(unmanaged_type_suffix_ast **yynode);
      bool parse_unsafe_statement(unsafe_statement_ast **yynode);
      bool parse_using_alias_directive_data(using_alias_directive_data_ast **yynode, identifier_ast *alias, namespace_or_type_name_ast *namespace_or_type_name);
      bool parse_using_directive(using_directive_ast **yynode);
      bool parse_using_namespace_directive_data(using_namespace_directive_data_ast **yynode, namespace_name_ast *namespace_name);
      bool parse_using_statement(using_statement_ast **yynode);
      bool parse_variable_declaration_data(variable_declaration_data_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, const list_node<variable_declarator_ast *> *variable_declarator_sequence);
      bool parse_variable_declarator(variable_declarator_ast **yynode, bool fixed_size_buffer);
      bool parse_variable_initializer(variable_initializer_ast **yynode);
      bool parse_while_statement(while_statement_ast **yynode);
      bool parse_yield_statement(yield_statement_ast **yynode);
    };

} // end of namespace csharp

#endif


