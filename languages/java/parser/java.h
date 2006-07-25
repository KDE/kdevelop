// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef java_h_INCLUDED
#define java_h_INCLUDED

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-list.h"
#include "kdev-pg-token-stream.h"
#include <cassert>

namespace java
  {

  struct additive_expression_ast;
  struct additive_expression_rest_ast;
  struct annotation_ast;
  struct annotation_arguments_ast;
  struct annotation_element_array_initializer_ast;
  struct annotation_element_array_value_ast;
  struct annotation_element_value_ast;
  struct annotation_element_value_pair_ast;
  struct annotation_method_declaration_ast;
  struct annotation_type_body_ast;
  struct annotation_type_declaration_ast;
  struct annotation_type_field_ast;
  struct array_access_ast;
  struct array_creator_rest_ast;
  struct array_type_dot_class_ast;
  struct assert_statement_ast;
  struct bit_and_expression_ast;
  struct bit_or_expression_ast;
  struct bit_xor_expression_ast;
  struct block_ast;
  struct block_statement_ast;
  struct break_statement_ast;
  struct builtin_type_ast;
  struct builtin_type_dot_class_ast;
  struct cast_expression_ast;
  struct catch_clause_ast;
  struct class_access_data_ast;
  struct class_body_ast;
  struct class_declaration_ast;
  struct class_extends_clause_ast;
  struct class_field_ast;
  struct class_or_interface_type_name_ast;
  struct class_or_interface_type_name_part_ast;
  struct class_type_ast;
  struct compilation_unit_ast;
  struct conditional_expression_ast;
  struct constructor_declaration_ast;
  struct continue_statement_ast;
  struct do_while_statement_ast;
  struct embedded_statement_ast;
  struct enum_body_ast;
  struct enum_constant_ast;
  struct enum_constant_body_ast;
  struct enum_constant_field_ast;
  struct enum_declaration_ast;
  struct equality_expression_ast;
  struct equality_expression_rest_ast;
  struct expression_ast;
  struct for_clause_traditional_rest_ast;
  struct for_control_ast;
  struct for_statement_ast;
  struct foreach_declaration_data_ast;
  struct identifier_ast;
  struct if_statement_ast;
  struct implements_clause_ast;
  struct import_declaration_ast;
  struct interface_body_ast;
  struct interface_declaration_ast;
  struct interface_extends_clause_ast;
  struct interface_field_ast;
  struct interface_method_declaration_ast;
  struct labeled_statement_ast;
  struct literal_ast;
  struct logical_and_expression_ast;
  struct logical_or_expression_ast;
  struct mandatory_array_builtin_type_ast;
  struct mandatory_declarator_brackets_ast;
  struct method_call_data_ast;
  struct method_declaration_ast;
  struct multiplicative_expression_ast;
  struct multiplicative_expression_rest_ast;
  struct new_expression_ast;
  struct non_array_type_ast;
  struct non_wildcard_type_arguments_ast;
  struct optional_argument_list_ast;
  struct optional_array_builtin_type_ast;
  struct optional_declarator_brackets_ast;
  struct optional_modifiers_ast;
  struct optional_parameter_declaration_list_ast;
  struct optional_parameter_modifiers_ast;
  struct package_declaration_ast;
  struct parameter_declaration_ast;
  struct parameter_declaration_ellipsis_ast;
  struct postfix_operator_ast;
  struct primary_atom_ast;
  struct primary_expression_ast;
  struct primary_selector_ast;
  struct qualified_identifier_ast;
  struct qualified_identifier_with_optional_star_ast;
  struct relational_expression_ast;
  struct relational_expression_rest_ast;
  struct return_statement_ast;
  struct shift_expression_ast;
  struct shift_expression_rest_ast;
  struct simple_name_access_data_ast;
  struct statement_expression_ast;
  struct super_access_data_ast;
  struct super_suffix_ast;
  struct switch_label_ast;
  struct switch_section_ast;
  struct switch_statement_ast;
  struct synchronized_statement_ast;
  struct this_access_data_ast;
  struct this_call_data_ast;
  struct throw_statement_ast;
  struct throws_clause_ast;
  struct try_statement_ast;
  struct type_ast;
  struct type_argument_ast;
  struct type_argument_type_ast;
  struct type_arguments_ast;
  struct type_arguments_or_parameters_end_ast;
  struct type_declaration_ast;
  struct type_parameter_ast;
  struct type_parameters_ast;
  struct unary_expression_ast;
  struct unary_expression_not_plusminus_ast;
  struct variable_array_initializer_ast;
  struct variable_declaration_ast;
  struct variable_declaration_data_ast;
  struct variable_declaration_rest_ast;
  struct variable_declaration_split_data_ast;
  struct variable_declarator_ast;
  struct variable_initializer_ast;
  struct while_statement_ast;
  struct wildcard_type_ast;
  struct wildcard_type_bounds_ast;

  namespace additive_expression_rest
    {
    enum additive_operator_enum {
      op_plus,
      op_minus,
    };
  }

  namespace builtin_type
    {
    enum builtin_type_enum {
      type_void,
      type_boolean,
      type_byte,
      type_char,
      type_short,
      type_int,
      type_float,
      type_long,
      type_double,
    };
  }

  namespace equality_expression_rest
    {
    enum equality_operator_enum {
      op_equal,
      op_not_equal,
    };
  }

  namespace expression
    {
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
      op_unsigned_rshift_assign,
    };
  }

  namespace literal
    {
    enum literal_type_enum {
      type_true,
      type_false,
      type_null,
      type_integer,
      type_floating_point,
      type_character,
      type_string,
    };
  }

  namespace modifiers
    {
    enum modifier_enum {
      mod_private      = 1,
      mod_public       = 2,
      mod_protected    = 4,
      mod_static       = 8,
      mod_transient    = 16,
      mod_final        = 32,
      mod_abstract     = 64,
      mod_native       = 128,
      mod_synchronized = 256,
      mod_volatile     = 512,
      mod_strictfp     = 1024,
    };
  }

  namespace multiplicative_expression_rest
    {
    enum multiplicative_operator_enum {
      op_star,
      op_slash,
      op_remainder,
    };
  }

  namespace postfix_operator
    {
    enum postfix_operator_enum {
      op_increment,
      op_decrement,
    };
  }

  namespace relational_expression_rest
    {
    enum relational_operator_enum {
      op_less_than,
      op_greater_than,
      op_less_equal,
      op_greater_equal,
    };
  }

  namespace shift_expression_rest
    {
    enum shift_operator_enum {
      op_lshift,
      op_signed_rshift,
      op_unsigned_rshift,
    };
  }

  namespace switch_label
    {
    enum branch_type_enum {
      case_branch,
      default_branch,
    };
  }

  namespace unary_expression
    {
    enum unary_expression_enum {
      type_incremented_expression,
      type_decremented_expression,
      type_unary_minus_expression,
      type_unary_plus_expression,
      type_unary_expression_not_plusminus,
    };
  }

  namespace unary_expression_not_plusminus
    {
    enum unary_expression_not_plusminus_enum {
      type_bitwise_not_expression,
      type_logical_not_expression,
      type_cast_expression,
      type_primary_expression,
    };
  }

  namespace wildcard_type_bounds
    {
    enum extends_or_super_enum {
      extends,
      super,
    };
  }


  struct ast_node
    {
      enum ast_node_kind_enum {
        Kind_additive_expression = 1000,
        Kind_additive_expression_rest = 1001,
        Kind_annotation = 1002,
        Kind_annotation_arguments = 1003,
        Kind_annotation_element_array_initializer = 1004,
        Kind_annotation_element_array_value = 1005,
        Kind_annotation_element_value = 1006,
        Kind_annotation_element_value_pair = 1007,
        Kind_annotation_method_declaration = 1008,
        Kind_annotation_type_body = 1009,
        Kind_annotation_type_declaration = 1010,
        Kind_annotation_type_field = 1011,
        Kind_array_access = 1012,
        Kind_array_creator_rest = 1013,
        Kind_array_type_dot_class = 1014,
        Kind_assert_statement = 1015,
        Kind_bit_and_expression = 1016,
        Kind_bit_or_expression = 1017,
        Kind_bit_xor_expression = 1018,
        Kind_block = 1019,
        Kind_block_statement = 1020,
        Kind_break_statement = 1021,
        Kind_builtin_type = 1022,
        Kind_builtin_type_dot_class = 1023,
        Kind_cast_expression = 1024,
        Kind_catch_clause = 1025,
        Kind_class_access_data = 1026,
        Kind_class_body = 1027,
        Kind_class_declaration = 1028,
        Kind_class_extends_clause = 1029,
        Kind_class_field = 1030,
        Kind_class_or_interface_type_name = 1031,
        Kind_class_or_interface_type_name_part = 1032,
        Kind_class_type = 1033,
        Kind_compilation_unit = 1034,
        Kind_conditional_expression = 1035,
        Kind_constructor_declaration = 1036,
        Kind_continue_statement = 1037,
        Kind_do_while_statement = 1038,
        Kind_embedded_statement = 1039,
        Kind_enum_body = 1040,
        Kind_enum_constant = 1041,
        Kind_enum_constant_body = 1042,
        Kind_enum_constant_field = 1043,
        Kind_enum_declaration = 1044,
        Kind_equality_expression = 1045,
        Kind_equality_expression_rest = 1046,
        Kind_expression = 1047,
        Kind_for_clause_traditional_rest = 1048,
        Kind_for_control = 1049,
        Kind_for_statement = 1050,
        Kind_foreach_declaration_data = 1051,
        Kind_identifier = 1052,
        Kind_if_statement = 1053,
        Kind_implements_clause = 1054,
        Kind_import_declaration = 1055,
        Kind_interface_body = 1056,
        Kind_interface_declaration = 1057,
        Kind_interface_extends_clause = 1058,
        Kind_interface_field = 1059,
        Kind_interface_method_declaration = 1060,
        Kind_labeled_statement = 1061,
        Kind_literal = 1062,
        Kind_logical_and_expression = 1063,
        Kind_logical_or_expression = 1064,
        Kind_mandatory_array_builtin_type = 1065,
        Kind_mandatory_declarator_brackets = 1066,
        Kind_method_call_data = 1067,
        Kind_method_declaration = 1068,
        Kind_multiplicative_expression = 1069,
        Kind_multiplicative_expression_rest = 1070,
        Kind_new_expression = 1071,
        Kind_non_array_type = 1072,
        Kind_non_wildcard_type_arguments = 1073,
        Kind_optional_argument_list = 1074,
        Kind_optional_array_builtin_type = 1075,
        Kind_optional_declarator_brackets = 1076,
        Kind_optional_modifiers = 1077,
        Kind_optional_parameter_declaration_list = 1078,
        Kind_optional_parameter_modifiers = 1079,
        Kind_package_declaration = 1080,
        Kind_parameter_declaration = 1081,
        Kind_parameter_declaration_ellipsis = 1082,
        Kind_postfix_operator = 1083,
        Kind_primary_atom = 1084,
        Kind_primary_expression = 1085,
        Kind_primary_selector = 1086,
        Kind_qualified_identifier = 1087,
        Kind_qualified_identifier_with_optional_star = 1088,
        Kind_relational_expression = 1089,
        Kind_relational_expression_rest = 1090,
        Kind_return_statement = 1091,
        Kind_shift_expression = 1092,
        Kind_shift_expression_rest = 1093,
        Kind_simple_name_access_data = 1094,
        Kind_statement_expression = 1095,
        Kind_super_access_data = 1096,
        Kind_super_suffix = 1097,
        Kind_switch_label = 1098,
        Kind_switch_section = 1099,
        Kind_switch_statement = 1100,
        Kind_synchronized_statement = 1101,
        Kind_this_access_data = 1102,
        Kind_this_call_data = 1103,
        Kind_throw_statement = 1104,
        Kind_throws_clause = 1105,
        Kind_try_statement = 1106,
        Kind_type = 1107,
        Kind_type_argument = 1108,
        Kind_type_argument_type = 1109,
        Kind_type_arguments = 1110,
        Kind_type_arguments_or_parameters_end = 1111,
        Kind_type_declaration = 1112,
        Kind_type_parameter = 1113,
        Kind_type_parameters = 1114,
        Kind_unary_expression = 1115,
        Kind_unary_expression_not_plusminus = 1116,
        Kind_variable_array_initializer = 1117,
        Kind_variable_declaration = 1118,
        Kind_variable_declaration_data = 1119,
        Kind_variable_declaration_rest = 1120,
        Kind_variable_declaration_split_data = 1121,
        Kind_variable_declarator = 1122,
        Kind_variable_initializer = 1123,
        Kind_while_statement = 1124,
        Kind_wildcard_type = 1125,
        Kind_wildcard_type_bounds = 1126,
        AST_NODE_KIND_COUNT
      };

      int kind;
      std::size_t start_token;
      std::size_t end_token;
    };

  struct additive_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_additive_expression
      };

      multiplicative_expression_ast *expression;
      const list_node<additive_expression_rest_ast *> *additional_expression_sequence;
    };

  struct additive_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_additive_expression_rest
      };

      additive_expression_rest::additive_operator_enum additive_operator;
      multiplicative_expression_ast *expression;
    };

  struct annotation_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation
      };

      bool has_parentheses;
      qualified_identifier_ast *type_name;
      annotation_arguments_ast *args;
    };

  struct annotation_arguments_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_arguments
      };

      const list_node<annotation_element_value_pair_ast *> *value_pair_sequence;
      annotation_element_value_ast *element_value;
    };

  struct annotation_element_array_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_element_array_initializer
      };

      const list_node<annotation_element_array_value_ast *> *element_value_sequence;
    };

  struct annotation_element_array_value_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_element_array_value
      };

      conditional_expression_ast *cond_expression;
      annotation_ast *annotation;
    };

  struct annotation_element_value_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_element_value
      };

      conditional_expression_ast *cond_expression;
      annotation_ast *annotation;
      annotation_element_array_initializer_ast *element_array_initializer;
    };

  struct annotation_element_value_pair_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_element_value_pair
      };

      identifier_ast *element_name;
      annotation_element_value_ast *element_value;
    };

  struct annotation_method_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_method_declaration
      };

      optional_modifiers_ast *modifiers;
      type_ast *return_type;
      identifier_ast *annotation_name;
      annotation_element_value_ast *annotation_element_value;
    };

  struct annotation_type_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_type_body
      };

      const list_node<annotation_type_field_ast *> *annotation_type_field_sequence;
    };

  struct annotation_type_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_type_declaration
      };

      optional_modifiers_ast *modifiers;
      identifier_ast *annotation_type_name;
      annotation_type_body_ast *body;
    };

  struct annotation_type_field_ast: public ast_node
    {
      enum
      {
        KIND = Kind_annotation_type_field
      };

      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
      annotation_method_declaration_ast *method_declaration;
      variable_declaration_data_ast *constant_declaration;
    };

  struct array_access_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_access
      };

      expression_ast *array_index_expression;
    };

  struct array_creator_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_creator_rest
      };

      mandatory_declarator_brackets_ast *mandatory_declarator_brackets;
      variable_array_initializer_ast *array_initializer;
      const list_node<expression_ast *> *index_expression_sequence;
      optional_declarator_brackets_ast *optional_declarator_brackets;
    };

  struct array_type_dot_class_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_type_dot_class
      };

      qualified_identifier_ast *qualified_identifier;
      mandatory_declarator_brackets_ast *declarator_brackets;
    };

  struct assert_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_assert_statement
      };

      expression_ast *condition;
      expression_ast *message;
    };

  struct bit_and_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_bit_and_expression
      };

      const list_node<equality_expression_ast *> *expression_sequence;
    };

  struct bit_or_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_bit_or_expression
      };

      const list_node<bit_xor_expression_ast *> *expression_sequence;
    };

  struct bit_xor_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_bit_xor_expression
      };

      const list_node<bit_and_expression_ast *> *expression_sequence;
    };

  struct block_ast: public ast_node
    {
      enum
      {
        KIND = Kind_block
      };

      const list_node<block_statement_ast *> *statement_sequence;
    };

  struct block_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_block_statement
      };

      variable_declaration_ast *variable_declaration;
      embedded_statement_ast *statement;
      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
    };

  struct break_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_break_statement
      };

      identifier_ast *label;
    };

  struct builtin_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_builtin_type
      };

      builtin_type::builtin_type_enum type;
    };

  struct builtin_type_dot_class_ast: public ast_node
    {
      enum
      {
        KIND = Kind_builtin_type_dot_class
      };

      optional_array_builtin_type_ast *builtin_type;
    };

  struct cast_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_cast_expression
      };

      optional_array_builtin_type_ast *builtin_type;
      unary_expression_ast *builtin_casted_expression;
      class_type_ast *class_type;
      unary_expression_not_plusminus_ast *class_casted_expression;
    };

  struct catch_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_catch_clause
      };

      variable_declaration_split_data_ast *exception_declaration;
      block_ast *body;
    };

  struct class_access_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_access_data
      };

    };

  struct class_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_body
      };

      const list_node<class_field_ast *> *declaration_sequence;
    };

  struct class_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_declaration
      };

      optional_modifiers_ast *modifiers;
      identifier_ast *class_name;
      type_parameters_ast *type_parameters;
      class_extends_clause_ast *extends;
      implements_clause_ast *implements;
      class_body_ast *body;
    };

  struct class_extends_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_extends_clause
      };

      class_or_interface_type_name_ast *type;
    };

  struct class_field_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_field
      };

      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
      constructor_declaration_ast *constructor_declaration;
      method_declaration_ast *method_declaration;
      variable_declaration_data_ast *variable_declaration;
      block_ast *instance_initializer_block;
      block_ast *static_initializer_block;
    };

  struct class_or_interface_type_name_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_or_interface_type_name
      };

      const list_node<class_or_interface_type_name_part_ast *> *part_sequence;
    };

  struct class_or_interface_type_name_part_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_or_interface_type_name_part
      };

      identifier_ast *identifier;
      type_arguments_ast *type_arguments;
    };

  struct class_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_type
      };

      class_or_interface_type_name_ast *type;
      optional_declarator_brackets_ast *declarator_brackets;
    };

  struct compilation_unit_ast: public ast_node
    {
      enum
      {
        KIND = Kind_compilation_unit
      };

      package_declaration_ast *package_declaration;
      const list_node<import_declaration_ast *> *import_declaration_sequence;
      const list_node<type_declaration_ast *> *type_declaration_sequence;
    };

  struct conditional_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_conditional_expression
      };

      logical_or_expression_ast *logical_or_expression;
      expression_ast *if_expression;
      conditional_expression_ast *else_expression;
    };

  struct constructor_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constructor_declaration
      };

      optional_modifiers_ast *modifiers;
      type_parameters_ast *type_parameters;
      identifier_ast *class_name;
      optional_parameter_declaration_list_ast *parameters;
      throws_clause_ast *throws_clause;
      block_ast *body;
    };

  struct continue_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_continue_statement
      };

      identifier_ast *label;
    };

  struct do_while_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_do_while_statement
      };

      embedded_statement_ast *body;
      expression_ast *condition;
    };

  struct embedded_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_embedded_statement
      };

      block_ast *block;
      assert_statement_ast *assert_statement;
      if_statement_ast *if_statement;
      for_statement_ast *for_statement;
      while_statement_ast *while_statement;
      do_while_statement_ast *do_while_statement;
      try_statement_ast *try_statement;
      switch_statement_ast *switch_statement;
      synchronized_statement_ast *synchronized_statement;
      return_statement_ast *return_statement;
      throw_statement_ast *throw_statement;
      break_statement_ast *break_statement;
      continue_statement_ast *continue_statement;
      labeled_statement_ast *labeled_statement;
      statement_expression_ast *expression_statement;
    };

  struct enum_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_body
      };

      const list_node<enum_constant_ast *> *enum_constant_sequence;
      const list_node<class_field_ast *> *class_field_sequence;
    };

  struct enum_constant_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_constant
      };

      const list_node<annotation_ast *> *annotation_sequence;
      identifier_ast *identifier;
      optional_argument_list_ast *arguments;
      enum_constant_body_ast *body;
    };

  struct enum_constant_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_constant_body
      };

      const list_node<enum_constant_field_ast *> *declaration_sequence;
    };

  struct enum_constant_field_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_constant_field
      };

      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
      method_declaration_ast *method_declaration;
      variable_declaration_data_ast *variable_declaration;
      block_ast *instance_initializer_block;
    };

  struct enum_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_declaration
      };

      optional_modifiers_ast *modifiers;
      identifier_ast *enum_name;
      implements_clause_ast *implements;
      enum_body_ast *body;
    };

  struct equality_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_equality_expression
      };

      relational_expression_ast *expression;
      const list_node<equality_expression_rest_ast *> *additional_expression_sequence;
    };

  struct equality_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_equality_expression_rest
      };

      equality_expression_rest::equality_operator_enum equality_operator;
      relational_expression_ast *expression;
    };

  struct expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_expression
      };

      expression::assignment_operator_enum assignment_operator;
      conditional_expression_ast *conditional_expression;
      expression_ast *assignment_expression;
    };

  struct for_clause_traditional_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_for_clause_traditional_rest
      };

      expression_ast *for_condition;
      const list_node<statement_expression_ast *> *for_update_expression_sequence;
    };

  struct for_control_ast: public ast_node
    {
      enum
      {
        KIND = Kind_for_control
      };

      foreach_declaration_data_ast *foreach_declaration;
      variable_declaration_split_data_ast *variable_declaration;
      for_clause_traditional_rest_ast *traditional_for_rest;
      const list_node<statement_expression_ast *> *statement_expression_sequence;
    };

  struct for_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_for_statement
      };

      for_control_ast *for_control;
      embedded_statement_ast *for_body;
    };

  struct foreach_declaration_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_foreach_declaration_data
      };

      parameter_declaration_ast *foreach_parameter;
      expression_ast *iterable_expression;
    };

  struct identifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_identifier
      };

      std::size_t ident;
    };

  struct if_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_if_statement
      };

      expression_ast *condition;
      embedded_statement_ast *if_body;
      embedded_statement_ast *else_body;
    };

  struct implements_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_implements_clause
      };

      const list_node<class_or_interface_type_name_ast *> *type_sequence;
    };

  struct import_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_import_declaration
      };

      bool static_import;
      qualified_identifier_with_optional_star_ast *identifier_name;
    };

  struct interface_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_body
      };

      const list_node<interface_field_ast *> *declaration_sequence;
    };

  struct interface_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_declaration
      };

      optional_modifiers_ast *modifiers;
      identifier_ast *interface_name;
      type_parameters_ast *type_parameters;
      interface_extends_clause_ast *extends;
      interface_body_ast *body;
    };

  struct interface_extends_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_extends_clause
      };

      const list_node<class_or_interface_type_name_ast *> *type_sequence;
    };

  struct interface_field_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_field
      };

      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
      interface_method_declaration_ast *interface_method_declaration;
      variable_declaration_data_ast *variable_declaration;
    };

  struct interface_method_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_method_declaration
      };

      optional_modifiers_ast *modifiers;
      type_parameters_ast *type_parameters;
      type_ast *return_type;
      identifier_ast *method_name;
      optional_parameter_declaration_list_ast *parameters;
      optional_declarator_brackets_ast *declarator_brackets;
      throws_clause_ast *throws_clause;
    };

  struct labeled_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_labeled_statement
      };

      identifier_ast *label;
      embedded_statement_ast *statement;
    };

  struct literal_ast: public ast_node
    {
      enum
      {
        KIND = Kind_literal
      };

      literal::literal_type_enum literal_type;
      std::size_t integer_literal;
      std::size_t floating_point_literal;
      std::size_t character_literal;
      std::size_t string_literal;
    };

  struct logical_and_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_logical_and_expression
      };

      const list_node<bit_or_expression_ast *> *expression_sequence;
    };

  struct logical_or_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_logical_or_expression
      };

      const list_node<logical_and_expression_ast *> *expression_sequence;
    };

  struct mandatory_array_builtin_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_mandatory_array_builtin_type
      };

      builtin_type_ast *type;
      mandatory_declarator_brackets_ast *declarator_brackets;
    };

  struct mandatory_declarator_brackets_ast: public ast_node
    {
      enum
      {
        KIND = Kind_mandatory_declarator_brackets
      };

      int bracket_count;
    };

  struct method_call_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_method_call_data
      };

      non_wildcard_type_arguments_ast *type_arguments;
      identifier_ast *method_name;
      optional_argument_list_ast *arguments;
    };

  struct method_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_method_declaration
      };

      optional_modifiers_ast *modifiers;
      type_parameters_ast *type_parameters;
      type_ast *return_type;
      identifier_ast *method_name;
      optional_parameter_declaration_list_ast *parameters;
      optional_declarator_brackets_ast *declarator_brackets;
      throws_clause_ast *throws_clause;
    };

  struct multiplicative_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_multiplicative_expression
      };

      unary_expression_ast *expression;
      const list_node<multiplicative_expression_rest_ast *> *additional_expression_sequence;
    };

  struct multiplicative_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_multiplicative_expression_rest
      };

      multiplicative_expression_rest::multiplicative_operator_enum multiplicative_operator;
      unary_expression_ast *expression;
    };

  struct new_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_new_expression
      };

      non_wildcard_type_arguments_ast *type_arguments;
      non_array_type_ast *type;
      optional_argument_list_ast *class_constructor_arguments;
      class_body_ast *class_body;
      array_creator_rest_ast *array_creator_rest;
    };

  struct non_array_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_non_array_type
      };

      class_or_interface_type_name_ast *class_or_interface_type;
      builtin_type_ast *builtin_type;
    };

  struct non_wildcard_type_arguments_ast: public ast_node
    {
      enum
      {
        KIND = Kind_non_wildcard_type_arguments
      };

      const list_node<type_argument_type_ast *> *type_argument_type_sequence;
    };

  struct optional_argument_list_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_argument_list
      };

      const list_node<expression_ast *> *expression_sequence;
    };

  struct optional_array_builtin_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_array_builtin_type
      };

      builtin_type_ast *type;
      optional_declarator_brackets_ast *declarator_brackets;
    };

  struct optional_declarator_brackets_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_declarator_brackets
      };

      int bracket_count;
    };

  struct optional_modifiers_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_modifiers
      };

      int modifiers;
      const list_node<annotation_ast *> *mod_annotation_sequence;
    };

  struct optional_parameter_declaration_list_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_parameter_declaration_list
      };

      const list_node<parameter_declaration_ellipsis_ast *> *parameter_declaration_sequence;
    };

  struct optional_parameter_modifiers_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_parameter_modifiers
      };

      bool has_mod_final;
      const list_node<annotation_ast *> *mod_annotation_sequence;
    };

  struct package_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_package_declaration
      };

      const list_node<annotation_ast *> *annotation_sequence;
      qualified_identifier_ast *package_name;
    };

  struct parameter_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_parameter_declaration
      };

      optional_modifiers_ast *parameter_modifiers;
      type_ast *type;
      identifier_ast *variable_name;
      optional_declarator_brackets_ast *declarator_brackets;
    };

  struct parameter_declaration_ellipsis_ast: public ast_node
    {
      enum
      {
        KIND = Kind_parameter_declaration_ellipsis
      };

      bool has_ellipsis;
      optional_parameter_modifiers_ast *parameter_modifiers;
      type_ast *type;
      identifier_ast *variable_name;
      optional_declarator_brackets_ast *declarator_brackets;
    };

  struct postfix_operator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_postfix_operator
      };

      postfix_operator::postfix_operator_enum postfix_operator;
    };

  struct primary_atom_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_atom
      };

      literal_ast *literal;
      new_expression_ast *new_expression;
      expression_ast *parenthesis_expression;
      builtin_type_dot_class_ast *builtin_type_dot_class;
      this_call_data_ast *this_call;
      this_access_data_ast *this_access;
      super_access_data_ast *super_access;
      method_call_data_ast *method_call;
      simple_name_access_data_ast *simple_name_access;
      array_type_dot_class_ast *array_type_dot_class;
    };

  struct primary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_expression
      };

      primary_atom_ast *primary_atom;
      const list_node<primary_selector_ast *> *selector_sequence;
    };

  struct primary_selector_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_selector
      };

      class_access_data_ast *class_access;
      this_access_data_ast *this_access;
      new_expression_ast *new_expression;
      simple_name_access_data_ast *simple_name_access;
      super_access_data_ast *super_access;
      method_call_data_ast *method_call;
      array_access_ast *array_access;
    };

  struct qualified_identifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_qualified_identifier
      };

      const list_node<identifier_ast *> *name_sequence;
    };

  struct qualified_identifier_with_optional_star_ast: public ast_node
    {
      enum
      {
        KIND = Kind_qualified_identifier_with_optional_star
      };

      bool has_star;
      const list_node<identifier_ast *> *name_sequence;
    };

  struct relational_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_relational_expression
      };

      shift_expression_ast *expression;
      const list_node<relational_expression_rest_ast *> *additional_expression_sequence;
      type_ast *instanceof_type;
    };

  struct relational_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_relational_expression_rest
      };

      relational_expression_rest::relational_operator_enum relational_operator;
      shift_expression_ast *expression;
    };

  struct return_statement_ast: public ast_node
                 {
                   enum
                   {
                     KIND = Kind_return_statement
                   };

                   expression_ast *return_expression;
                 };

  struct shift_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_shift_expression
      };

      additive_expression_ast *expression;
      const list_node<shift_expression_rest_ast *> *additional_expression_sequence;
    };

  struct shift_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_shift_expression_rest
      };

      shift_expression_rest::shift_operator_enum shift_operator;
      additive_expression_ast *expression;
    };

  struct simple_name_access_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_simple_name_access_data
      };

      identifier_ast *name;
    };

  struct statement_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_statement_expression
      };

      expression_ast *expression;
    };

  struct super_access_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_super_access_data
      };

      non_wildcard_type_arguments_ast *type_arguments;
      super_suffix_ast *super_suffix;
    };

  struct super_suffix_ast: public ast_node
    {
      enum
      {
        KIND = Kind_super_suffix
      };

      optional_argument_list_ast *constructor_arguments;
      simple_name_access_data_ast *simple_name_access;
      method_call_data_ast *method_call;
    };

  struct switch_label_ast: public ast_node
    {
      enum
      {
        KIND = Kind_switch_label
      };

      switch_label::branch_type_enum branch_type;
      expression_ast *case_expression;
    };

  struct switch_section_ast: public ast_node
    {
      enum
      {
        KIND = Kind_switch_section
      };

      const list_node<switch_label_ast *> *label_sequence;
      const list_node<block_statement_ast *> *statement_sequence;
    };

  struct switch_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_switch_statement
      };

      expression_ast *switch_expression;
      const list_node<switch_section_ast *> *switch_section_sequence;
    };

  struct synchronized_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_synchronized_statement
      };

      expression_ast *locked_type;
      block_ast *synchronized_body;
    };

  struct this_access_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_this_access_data
      };

    };

  struct this_call_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_this_call_data
      };

      non_wildcard_type_arguments_ast *type_arguments;
      optional_argument_list_ast *arguments;
    };

  struct throw_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_throw_statement
      };

      expression_ast *exception;
    };

  struct throws_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_throws_clause
      };

      const list_node<qualified_identifier_ast *> *identifier_sequence;
    };

  struct try_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_try_statement
      };

      block_ast *try_body;
      const list_node<catch_clause_ast *> *catch_clause_sequence;
      block_ast *finally_body;
    };

  struct type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type
      };

      class_type_ast *class_type;
      optional_array_builtin_type_ast *builtin_type;
    };

  struct type_argument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_argument
      };

      type_argument_type_ast *type_argument_type;
      wildcard_type_ast *wildcard_type;
    };

  struct type_argument_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_argument_type
      };

      class_type_ast *class_type;
      mandatory_array_builtin_type_ast *mandatory_array_builtin_type;
    };

  struct type_arguments_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_arguments
      };

      const list_node<type_argument_ast *> *type_argument_sequence;
    };

  struct type_arguments_or_parameters_end_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_arguments_or_parameters_end
      };

    };

  struct type_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_declaration
      };

      class_declaration_ast *class_declaration;
      enum_declaration_ast *enum_declaration;
      interface_declaration_ast *interface_declaration;
      annotation_type_declaration_ast *annotation_type_declaration;
    };

  struct type_parameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameter
      };

      identifier_ast *identifier;
      const list_node<class_or_interface_type_name_ast *> *extends_type_sequence;
    };

  struct type_parameters_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameters
      };

      const list_node<type_parameter_ast *> *type_parameter_sequence;
    };

  struct unary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unary_expression
      };

      unary_expression::unary_expression_enum rule_type;
      unary_expression_ast *unary_expression;
      unary_expression_not_plusminus_ast *unary_expression_not_plusminus;
    };

  struct unary_expression_not_plusminus_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unary_expression_not_plusminus
      };

      unary_expression_not_plusminus::unary_expression_not_plusminus_enum rule_type;
      unary_expression_ast *bitwise_not_expression;
      unary_expression_ast *logical_not_expression;
      cast_expression_ast *cast_expression;
      primary_expression_ast *primary_expression;
      const list_node<postfix_operator_ast *> *postfix_operator_sequence;
    };

  struct variable_array_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_array_initializer
      };

      const list_node<variable_initializer_ast *> *variable_initializer_sequence;
    };

  struct variable_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration
      };

      variable_declaration_data_ast *data;
    };

  struct variable_declaration_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration_data
      };

      optional_modifiers_ast *modifiers;
      type_ast *type;
      const list_node<variable_declarator_ast *> *declarator_sequence;
    };

  struct variable_declaration_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration_rest
      };

      variable_initializer_ast *first_initializer;
      const list_node<variable_declarator_ast *> *variable_declarator_sequence;
    };

  struct variable_declaration_split_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration_split_data
      };

      variable_declaration_data_ast *data;
    };

  struct variable_declarator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declarator
      };

      identifier_ast *variable_name;
      optional_declarator_brackets_ast *declarator_brackets;
      variable_initializer_ast *initializer;
    };

  struct variable_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_initializer
      };

      expression_ast *expression;
      variable_array_initializer_ast *array_initializer;
    };

  struct while_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_while_statement
      };

      expression_ast *condition;
      embedded_statement_ast *body;
    };

  struct wildcard_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_wildcard_type
      };

      wildcard_type_bounds_ast *bounds;
    };

  struct wildcard_type_bounds_ast: public ast_node
    {
      enum
      {
        KIND = Kind_wildcard_type_bounds
      };

      wildcard_type_bounds::extends_or_super_enum extends_or_super;
      class_type_ast *type;
    };



} // end of namespace java


#include <string>

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
      void tokenize();

      /**
       * The compatibility_mode status variable tells which version of Java
       * should be checked against.
       */
      enum java_compatibility_mode {
        java13_compatibility = 130,
        java14_compatibility = 140,
        java15_compatibility = 150,
      };

      parser::java_compatibility_mode compatibility_mode();
      void set_compatibility_mode( parser::java_compatibility_mode mode );

      enum problem_type {
        error,
        warning,
        info,
      };
      void report_problem( parser::problem_type type, const char* message );
      void report_problem( parser::problem_type type, std::string message );

    private:

      parser::java_compatibility_mode _M_compatibility_mode;

      // ltCounter stores the amount of currently open type arguments rules,
      // all of which are beginning with a less than ("<") character.
      // This way, also SIGNED_RSHIFT (">>") and UNSIGNED_RSHIFT (">>>") can be used
      // to close type arguments rules, in addition to GREATER_THAN (">").
      int ltCounter;

      // Lookahead hacks
      bool lookahead_is_package_declaration();
      bool lookahead_is_parameter_declaration();
      bool lookahead_is_cast_expression();
      bool lookahead_is_array_type_dot_class();


    public:
      parser()
      {
        memory_pool = 0;
        token_stream = 0;
        yytoken = Token_EOF;

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
      bool parse_variable_declarator(variable_declarator_ast **yynode);
      bool parse_variable_initializer(variable_initializer_ast **yynode);
      bool parse_while_statement(while_statement_ast **yynode);
      bool parse_wildcard_type(wildcard_type_ast **yynode);
      bool parse_wildcard_type_bounds(wildcard_type_bounds_ast **yynode);
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
      virtual void visit_additive_expression(additive_expression_ast *)
    {}
      virtual void visit_additive_expression_rest(additive_expression_rest_ast *)
      {}
      virtual void visit_annotation(annotation_ast *)
      {}
      virtual void visit_annotation_arguments(annotation_arguments_ast *)
      {}
      virtual void visit_annotation_element_array_initializer(annotation_element_array_initializer_ast *)
      {}
      virtual void visit_annotation_element_array_value(annotation_element_array_value_ast *)
      {}
      virtual void visit_annotation_element_value(annotation_element_value_ast *)
      {}
      virtual void visit_annotation_element_value_pair(annotation_element_value_pair_ast *)
      {}
      virtual void visit_annotation_method_declaration(annotation_method_declaration_ast *)
      {}
      virtual void visit_annotation_type_body(annotation_type_body_ast *)
      {}
      virtual void visit_annotation_type_declaration(annotation_type_declaration_ast *)
      {}
      virtual void visit_annotation_type_field(annotation_type_field_ast *)
      {}
      virtual void visit_array_access(array_access_ast *)
      {}
      virtual void visit_array_creator_rest(array_creator_rest_ast *)
      {}
      virtual void visit_array_type_dot_class(array_type_dot_class_ast *)
      {}
      virtual void visit_assert_statement(assert_statement_ast *)
      {}
      virtual void visit_bit_and_expression(bit_and_expression_ast *)
      {}
      virtual void visit_bit_or_expression(bit_or_expression_ast *)
      {}
      virtual void visit_bit_xor_expression(bit_xor_expression_ast *)
      {}
      virtual void visit_block(block_ast *)
      {}
      virtual void visit_block_statement(block_statement_ast *)
      {}
      virtual void visit_break_statement(break_statement_ast *)
      {}
      virtual void visit_builtin_type(builtin_type_ast *)
      {}
      virtual void visit_builtin_type_dot_class(builtin_type_dot_class_ast *)
      {}
      virtual void visit_cast_expression(cast_expression_ast *)
      {}
      virtual void visit_catch_clause(catch_clause_ast *)
      {}
      virtual void visit_class_access_data(class_access_data_ast *)
      {}
      virtual void visit_class_body(class_body_ast *)
      {}
      virtual void visit_class_declaration(class_declaration_ast *)
      {}
      virtual void visit_class_extends_clause(class_extends_clause_ast *)
      {}
      virtual void visit_class_field(class_field_ast *)
      {}
      virtual void visit_class_or_interface_type_name(class_or_interface_type_name_ast *)
      {}
      virtual void visit_class_or_interface_type_name_part(class_or_interface_type_name_part_ast *)
      {}
      virtual void visit_class_type(class_type_ast *)
      {}
      virtual void visit_compilation_unit(compilation_unit_ast *)
      {}
      virtual void visit_conditional_expression(conditional_expression_ast *)
      {}
      virtual void visit_constructor_declaration(constructor_declaration_ast *)
      {}
      virtual void visit_continue_statement(continue_statement_ast *)
      {}
      virtual void visit_do_while_statement(do_while_statement_ast *)
      {}
      virtual void visit_embedded_statement(embedded_statement_ast *)
      {}
      virtual void visit_enum_body(enum_body_ast *)
      {}
      virtual void visit_enum_constant(enum_constant_ast *)
      {}
      virtual void visit_enum_constant_body(enum_constant_body_ast *)
      {}
      virtual void visit_enum_constant_field(enum_constant_field_ast *)
      {}
      virtual void visit_enum_declaration(enum_declaration_ast *)
      {}
      virtual void visit_equality_expression(equality_expression_ast *)
      {}
      virtual void visit_equality_expression_rest(equality_expression_rest_ast *)
      {}
      virtual void visit_expression(expression_ast *)
      {}
      virtual void visit_for_clause_traditional_rest(for_clause_traditional_rest_ast *)
      {}
      virtual void visit_for_control(for_control_ast *)
      {}
      virtual void visit_for_statement(for_statement_ast *)
      {}
      virtual void visit_foreach_declaration_data(foreach_declaration_data_ast *)
      {}
      virtual void visit_identifier(identifier_ast *)
      {}
      virtual void visit_if_statement(if_statement_ast *)
      {}
      virtual void visit_implements_clause(implements_clause_ast *)
      {}
      virtual void visit_import_declaration(import_declaration_ast *)
      {}
      virtual void visit_interface_body(interface_body_ast *)
      {}
      virtual void visit_interface_declaration(interface_declaration_ast *)
      {}
      virtual void visit_interface_extends_clause(interface_extends_clause_ast *)
      {}
      virtual void visit_interface_field(interface_field_ast *)
      {}
      virtual void visit_interface_method_declaration(interface_method_declaration_ast *)
      {}
      virtual void visit_labeled_statement(labeled_statement_ast *)
      {}
      virtual void visit_literal(literal_ast *)
      {}
      virtual void visit_logical_and_expression(logical_and_expression_ast *)
      {}
      virtual void visit_logical_or_expression(logical_or_expression_ast *)
      {}
      virtual void visit_mandatory_array_builtin_type(mandatory_array_builtin_type_ast *)
      {}
      virtual void visit_mandatory_declarator_brackets(mandatory_declarator_brackets_ast *)
      {}
      virtual void visit_method_call_data(method_call_data_ast *)
      {}
      virtual void visit_method_declaration(method_declaration_ast *)
      {}
      virtual void visit_multiplicative_expression(multiplicative_expression_ast *)
      {}
      virtual void visit_multiplicative_expression_rest(multiplicative_expression_rest_ast *)
      {}
      virtual void visit_new_expression(new_expression_ast *)
      {}
      virtual void visit_non_array_type(non_array_type_ast *)
      {}
      virtual void visit_non_wildcard_type_arguments(non_wildcard_type_arguments_ast *)
      {}
      virtual void visit_optional_argument_list(optional_argument_list_ast *)
      {}
      virtual void visit_optional_array_builtin_type(optional_array_builtin_type_ast *)
      {}
      virtual void visit_optional_declarator_brackets(optional_declarator_brackets_ast *)
      {}
      virtual void visit_optional_modifiers(optional_modifiers_ast *)
      {}
      virtual void visit_optional_parameter_declaration_list(optional_parameter_declaration_list_ast *)
      {}
      virtual void visit_optional_parameter_modifiers(optional_parameter_modifiers_ast *)
      {}
      virtual void visit_package_declaration(package_declaration_ast *)
      {}
      virtual void visit_parameter_declaration(parameter_declaration_ast *)
      {}
      virtual void visit_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast *)
      {}
      virtual void visit_postfix_operator(postfix_operator_ast *)
      {}
      virtual void visit_primary_atom(primary_atom_ast *)
      {}
      virtual void visit_primary_expression(primary_expression_ast *)
      {}
      virtual void visit_primary_selector(primary_selector_ast *)
      {}
      virtual void visit_qualified_identifier(qualified_identifier_ast *)
      {}
      virtual void visit_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast *)
      {}
      virtual void visit_relational_expression(relational_expression_ast *)
      {}
      virtual void visit_relational_expression_rest(relational_expression_rest_ast *)
      {}
      virtual void visit_return_statement(return_statement_ast *)
                               {}
                               virtual void visit_shift_expression(shift_expression_ast *)
                               {}
                               virtual void visit_shift_expression_rest(shift_expression_rest_ast *)
                               {}
                               virtual void visit_simple_name_access_data(simple_name_access_data_ast *)
                               {}
                               virtual void visit_statement_expression(statement_expression_ast *)
                               {}
                               virtual void visit_super_access_data(super_access_data_ast *)
                               {}
                               virtual void visit_super_suffix(super_suffix_ast *)
                               {}
                               virtual void visit_switch_label(switch_label_ast *)
                               {}
                               virtual void visit_switch_section(switch_section_ast *)
                               {}
                               virtual void visit_switch_statement(switch_statement_ast *)
                               {}
                               virtual void visit_synchronized_statement(synchronized_statement_ast *)
                               {}
                               virtual void visit_this_access_data(this_access_data_ast *)
                               {}
                               virtual void visit_this_call_data(this_call_data_ast *)
                               {}
                               virtual void visit_throw_statement(throw_statement_ast *)
                               {}
                               virtual void visit_throws_clause(throws_clause_ast *)
                               {}
                               virtual void visit_try_statement(try_statement_ast *)
                               {}
                               virtual void visit_type(type_ast *)
                               {}
                               virtual void visit_type_argument(type_argument_ast *)
                               {}
                               virtual void visit_type_argument_type(type_argument_type_ast *)
                               {}
                               virtual void visit_type_arguments(type_arguments_ast *)
                               {}
                               virtual void visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *)
                               {}
                               virtual void visit_type_declaration(type_declaration_ast *)
                               {}
                               virtual void visit_type_parameter(type_parameter_ast *)
                               {}
                               virtual void visit_type_parameters(type_parameters_ast *)
                               {}
                               virtual void visit_unary_expression(unary_expression_ast *)
                               {}
                               virtual void visit_unary_expression_not_plusminus(unary_expression_not_plusminus_ast *)
                               {}
                               virtual void visit_variable_array_initializer(variable_array_initializer_ast *)
                               {}
                               virtual void visit_variable_declaration(variable_declaration_ast *)
                               {}
                               virtual void visit_variable_declaration_data(variable_declaration_data_ast *)
                               {}
                               virtual void visit_variable_declaration_rest(variable_declaration_rest_ast *)
                               {}
                               virtual void visit_variable_declaration_split_data(variable_declaration_split_data_ast *)
                               {}
                               virtual void visit_variable_declarator(variable_declarator_ast *)
                               {}
                               virtual void visit_variable_initializer(variable_initializer_ast *)
                               {}
                               virtual void visit_while_statement(while_statement_ast *)
                               {}
                               virtual void visit_wildcard_type(wildcard_type_ast *)
                               {}
                               virtual void visit_wildcard_type_bounds(wildcard_type_bounds_ast *)
                               {}
                             }
                           ;
  class default_visitor: public visitor
    {
    public:
      virtual void visit_additive_expression(additive_expression_ast *node)
      {
        visit_node(node->expression);
        if (node->additional_expression_sequence)
          {
            const list_node<additive_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_additive_expression_rest(additive_expression_rest_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_annotation(annotation_ast *node)
      {
        visit_node(node->type_name);
        visit_node(node->args);
      }

      virtual void visit_annotation_arguments(annotation_arguments_ast *node)
      {
        if (node->value_pair_sequence)
          {
            const list_node<annotation_element_value_pair_ast*> *__it = node->value_pair_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->element_value);
      }

      virtual void visit_annotation_element_array_initializer(annotation_element_array_initializer_ast *node)
      {
        if (node->element_value_sequence)
          {
            const list_node<annotation_element_array_value_ast*> *__it = node->element_value_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_annotation_element_array_value(annotation_element_array_value_ast *node)
      {
        visit_node(node->cond_expression);
        visit_node(node->annotation);
      }

      virtual void visit_annotation_element_value(annotation_element_value_ast *node)
      {
        visit_node(node->cond_expression);
        visit_node(node->annotation);
        visit_node(node->element_array_initializer);
      }

      virtual void visit_annotation_element_value_pair(annotation_element_value_pair_ast *node)
      {
        visit_node(node->element_name);
        visit_node(node->element_value);
      }

      virtual void visit_annotation_method_declaration(annotation_method_declaration_ast *node)
      {
        visit_node(node->return_type);
        visit_node(node->modifiers);
        visit_node(node->annotation_name);
        visit_node(node->annotation_element_value);
      }

      virtual void visit_annotation_type_body(annotation_type_body_ast *node)
      {
        if (node->annotation_type_field_sequence)
          {
            const list_node<annotation_type_field_ast*> *__it = node->annotation_type_field_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_annotation_type_declaration(annotation_type_declaration_ast *node)
      {
        visit_node(node->modifiers);
        visit_node(node->annotation_type_name);
        visit_node(node->body);
      }

      virtual void visit_annotation_type_field(annotation_type_field_ast *node)
      {
        visit_node(node->class_declaration);
        visit_node(node->enum_declaration);
        visit_node(node->interface_declaration);
        visit_node(node->annotation_type_declaration);
        visit_node(node->method_declaration);
        visit_node(node->constant_declaration);
      }

      virtual void visit_array_access(array_access_ast *node)
      {
        visit_node(node->array_index_expression);
      }

      virtual void visit_array_creator_rest(array_creator_rest_ast *node)
      {
        visit_node(node->mandatory_declarator_brackets);
        visit_node(node->array_initializer);
        if (node->index_expression_sequence)
          {
            const list_node<expression_ast*> *__it = node->index_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->optional_declarator_brackets);
      }

      virtual void visit_array_type_dot_class(array_type_dot_class_ast *node)
      {
        visit_node(node->qualified_identifier);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_assert_statement(assert_statement_ast *node)
      {
        visit_node(node->condition);
        visit_node(node->message);
      }

      virtual void visit_bit_and_expression(bit_and_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<equality_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_bit_or_expression(bit_or_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<bit_xor_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_bit_xor_expression(bit_xor_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<bit_and_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_block(block_ast *node)
      {
        if (node->statement_sequence)
          {
            const list_node<block_statement_ast*> *__it = node->statement_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_block_statement(block_statement_ast *node)
      {
        visit_node(node->variable_declaration);
        visit_node(node->statement);
        visit_node(node->class_declaration);
        visit_node(node->enum_declaration);
        visit_node(node->interface_declaration);
        visit_node(node->annotation_type_declaration);
      }

      virtual void visit_break_statement(break_statement_ast *node)
      {
        visit_node(node->label);
      }

      virtual void visit_builtin_type(builtin_type_ast *)
      {}

      virtual void visit_builtin_type_dot_class(builtin_type_dot_class_ast *node)
      {
        visit_node(node->builtin_type);
      }

      virtual void visit_cast_expression(cast_expression_ast *node)
      {
        visit_node(node->builtin_type);
        visit_node(node->builtin_casted_expression);
        visit_node(node->class_type);
        visit_node(node->class_casted_expression);
      }

      virtual void visit_catch_clause(catch_clause_ast *node)
      {
        visit_node(node->exception_declaration);
        visit_node(node->body);
      }

      virtual void visit_class_access_data(class_access_data_ast *)
      {}

      virtual void visit_class_body(class_body_ast *node)
      {
        if (node->declaration_sequence)
          {
            const list_node<class_field_ast*> *__it = node->declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_class_declaration(class_declaration_ast *node)
      {
        visit_node(node->modifiers);
        visit_node(node->class_name);
        visit_node(node->type_parameters);
        visit_node(node->extends);
        visit_node(node->implements);
        visit_node(node->body);
      }

      virtual void visit_class_extends_clause(class_extends_clause_ast *node)
      {
        visit_node(node->type);
      }

      virtual void visit_class_field(class_field_ast *node)
      {
        visit_node(node->class_declaration);
        visit_node(node->enum_declaration);
        visit_node(node->interface_declaration);
        visit_node(node->annotation_type_declaration);
        visit_node(node->constructor_declaration);
        visit_node(node->method_declaration);
        visit_node(node->variable_declaration);
        visit_node(node->instance_initializer_block);
        visit_node(node->static_initializer_block);
      }

      virtual void visit_class_or_interface_type_name(class_or_interface_type_name_ast *node)
      {
        if (node->part_sequence)
          {
            const list_node<class_or_interface_type_name_part_ast*> *__it = node->part_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_class_or_interface_type_name_part(class_or_interface_type_name_part_ast *node)
      {
        visit_node(node->identifier);
        visit_node(node->type_arguments);
      }

      virtual void visit_class_type(class_type_ast *node)
      {
        visit_node(node->type);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_compilation_unit(compilation_unit_ast *node)
      {
        visit_node(node->package_declaration);
        if (node->import_declaration_sequence)
          {
            const list_node<import_declaration_ast*> *__it = node->import_declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->type_declaration_sequence)
          {
            const list_node<type_declaration_ast*> *__it = node->type_declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_conditional_expression(conditional_expression_ast *node)
      {
        visit_node(node->logical_or_expression);
        visit_node(node->if_expression);
        visit_node(node->else_expression);
      }

      virtual void visit_constructor_declaration(constructor_declaration_ast *node)
      {
        visit_node(node->type_parameters);
        visit_node(node->modifiers);
        visit_node(node->class_name);
        visit_node(node->parameters);
        visit_node(node->throws_clause);
        visit_node(node->body);
      }

      virtual void visit_continue_statement(continue_statement_ast *node)
      {
        visit_node(node->label);
      }

      virtual void visit_do_while_statement(do_while_statement_ast *node)
      {
        visit_node(node->body);
        visit_node(node->condition);
      }

      virtual void visit_embedded_statement(embedded_statement_ast *node)
      {
        visit_node(node->block);
        visit_node(node->assert_statement);
        visit_node(node->if_statement);
        visit_node(node->for_statement);
        visit_node(node->while_statement);
        visit_node(node->do_while_statement);
        visit_node(node->try_statement);
        visit_node(node->switch_statement);
        visit_node(node->synchronized_statement);
        visit_node(node->return_statement);
        visit_node(node->throw_statement);
        visit_node(node->break_statement);
        visit_node(node->continue_statement);
        visit_node(node->labeled_statement);
        visit_node(node->expression_statement);
      }

      virtual void visit_enum_body(enum_body_ast *node)
      {
        if (node->enum_constant_sequence)
          {
            const list_node<enum_constant_ast*> *__it = node->enum_constant_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->class_field_sequence)
          {
            const list_node<class_field_ast*> *__it = node->class_field_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_enum_constant(enum_constant_ast *node)
      {
        if (node->annotation_sequence)
          {
            const list_node<annotation_ast*> *__it = node->annotation_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->identifier);
        visit_node(node->arguments);
        visit_node(node->body);
      }

      virtual void visit_enum_constant_body(enum_constant_body_ast *node)
      {
        if (node->declaration_sequence)
          {
            const list_node<enum_constant_field_ast*> *__it = node->declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_enum_constant_field(enum_constant_field_ast *node)
      {
        visit_node(node->class_declaration);
        visit_node(node->enum_declaration);
        visit_node(node->interface_declaration);
        visit_node(node->annotation_type_declaration);
        visit_node(node->method_declaration);
        visit_node(node->variable_declaration);
        visit_node(node->instance_initializer_block);
      }

      virtual void visit_enum_declaration(enum_declaration_ast *node)
      {
        visit_node(node->modifiers);
        visit_node(node->enum_name);
        visit_node(node->implements);
        visit_node(node->body);
      }

      virtual void visit_equality_expression(equality_expression_ast *node)
      {
        visit_node(node->expression);
        if (node->additional_expression_sequence)
          {
            const list_node<equality_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_equality_expression_rest(equality_expression_rest_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_expression(expression_ast *node)
      {
        visit_node(node->conditional_expression);
        visit_node(node->assignment_expression);
      }

      virtual void visit_for_clause_traditional_rest(for_clause_traditional_rest_ast *node)
      {
        visit_node(node->for_condition);
        if (node->for_update_expression_sequence)
          {
            const list_node<statement_expression_ast*> *__it = node->for_update_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_for_control(for_control_ast *node)
      {
        visit_node(node->foreach_declaration);
        visit_node(node->variable_declaration);
        visit_node(node->traditional_for_rest);
        if (node->statement_expression_sequence)
          {
            const list_node<statement_expression_ast*> *__it = node->statement_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_for_statement(for_statement_ast *node)
      {
        visit_node(node->for_control);
        visit_node(node->for_body);
      }

      virtual void visit_foreach_declaration_data(foreach_declaration_data_ast *node)
      {
        visit_node(node->iterable_expression);
        visit_node(node->foreach_parameter);
      }

      virtual void visit_identifier(identifier_ast *)
      {}

      virtual void visit_if_statement(if_statement_ast *node)
      {
        visit_node(node->condition);
        visit_node(node->if_body);
        visit_node(node->else_body);
      }

      virtual void visit_implements_clause(implements_clause_ast *node)
      {
        if (node->type_sequence)
          {
            const list_node<class_or_interface_type_name_ast*> *__it = node->type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_import_declaration(import_declaration_ast *node)
      {
        visit_node(node->identifier_name);
      }

      virtual void visit_interface_body(interface_body_ast *node)
      {
        if (node->declaration_sequence)
          {
            const list_node<interface_field_ast*> *__it = node->declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_interface_declaration(interface_declaration_ast *node)
      {
        visit_node(node->modifiers);
        visit_node(node->interface_name);
        visit_node(node->type_parameters);
        visit_node(node->extends);
        visit_node(node->body);
      }

      virtual void visit_interface_extends_clause(interface_extends_clause_ast *node)
      {
        if (node->type_sequence)
          {
            const list_node<class_or_interface_type_name_ast*> *__it = node->type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_interface_field(interface_field_ast *node)
      {
        visit_node(node->class_declaration);
        visit_node(node->enum_declaration);
        visit_node(node->interface_declaration);
        visit_node(node->annotation_type_declaration);
        visit_node(node->interface_method_declaration);
        visit_node(node->variable_declaration);
      }

      virtual void visit_interface_method_declaration(interface_method_declaration_ast *node)
      {
        visit_node(node->return_type);
        visit_node(node->type_parameters);
        visit_node(node->modifiers);
        visit_node(node->method_name);
        visit_node(node->parameters);
        visit_node(node->declarator_brackets);
        visit_node(node->throws_clause);
      }

      virtual void visit_labeled_statement(labeled_statement_ast *node)
      {
        visit_node(node->label);
        visit_node(node->statement);
      }

      virtual void visit_literal(literal_ast *)
      {}

      virtual void visit_logical_and_expression(logical_and_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<bit_or_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_logical_or_expression(logical_or_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<logical_and_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_mandatory_array_builtin_type(mandatory_array_builtin_type_ast *node)
      {
        visit_node(node->type);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_mandatory_declarator_brackets(mandatory_declarator_brackets_ast *)
      {}

      virtual void visit_method_call_data(method_call_data_ast *node)
      {
        visit_node(node->arguments);
        visit_node(node->method_name);
        visit_node(node->type_arguments);
      }

      virtual void visit_method_declaration(method_declaration_ast *node)
      {
        visit_node(node->return_type);
        visit_node(node->type_parameters);
        visit_node(node->modifiers);
        visit_node(node->method_name);
        visit_node(node->parameters);
        visit_node(node->declarator_brackets);
        visit_node(node->throws_clause);
      }

      virtual void visit_multiplicative_expression(multiplicative_expression_ast *node)
      {
        visit_node(node->expression);
        if (node->additional_expression_sequence)
          {
            const list_node<multiplicative_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_multiplicative_expression_rest(multiplicative_expression_rest_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_new_expression(new_expression_ast *node)
      {
        visit_node(node->type_arguments);
        visit_node(node->type);
        visit_node(node->class_constructor_arguments);
        visit_node(node->class_body);
        visit_node(node->array_creator_rest);
      }

      virtual void visit_non_array_type(non_array_type_ast *node)
      {
        visit_node(node->class_or_interface_type);
        visit_node(node->builtin_type);
      }

      virtual void visit_non_wildcard_type_arguments(non_wildcard_type_arguments_ast *node)
      {
        if (node->type_argument_type_sequence)
          {
            const list_node<type_argument_type_ast*> *__it = node->type_argument_type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_optional_argument_list(optional_argument_list_ast *node)
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

      virtual void visit_optional_array_builtin_type(optional_array_builtin_type_ast *node)
      {
        visit_node(node->type);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_optional_declarator_brackets(optional_declarator_brackets_ast *)
      {}

      virtual void visit_optional_modifiers(optional_modifiers_ast *node)
      {
        if (node->mod_annotation_sequence)
          {
            const list_node<annotation_ast*> *__it = node->mod_annotation_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_optional_parameter_declaration_list(optional_parameter_declaration_list_ast *node)
      {
        if (node->parameter_declaration_sequence)
          {
            const list_node<parameter_declaration_ellipsis_ast*> *__it = node->parameter_declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_optional_parameter_modifiers(optional_parameter_modifiers_ast *node)
      {
        if (node->mod_annotation_sequence)
          {
            const list_node<annotation_ast*> *__it = node->mod_annotation_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_package_declaration(package_declaration_ast *node)
      {
        if (node->annotation_sequence)
          {
            const list_node<annotation_ast*> *__it = node->annotation_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->package_name);
      }

      virtual void visit_parameter_declaration(parameter_declaration_ast *node)
      {
        visit_node(node->parameter_modifiers);
        visit_node(node->type);
        visit_node(node->variable_name);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast *node)
      {
        visit_node(node->parameter_modifiers);
        visit_node(node->type);
        visit_node(node->variable_name);
        visit_node(node->declarator_brackets);
      }

      virtual void visit_postfix_operator(postfix_operator_ast *)
      {}

      virtual void visit_primary_atom(primary_atom_ast *node)
      {
        visit_node(node->literal);
        visit_node(node->new_expression);
        visit_node(node->parenthesis_expression);
        visit_node(node->builtin_type_dot_class);
        visit_node(node->this_call);
        visit_node(node->this_access);
        visit_node(node->super_access);
        visit_node(node->method_call);
        visit_node(node->simple_name_access);
        visit_node(node->array_type_dot_class);
      }

      virtual void visit_primary_expression(primary_expression_ast *node)
      {
        visit_node(node->primary_atom);
        if (node->selector_sequence)
          {
            const list_node<primary_selector_ast*> *__it = node->selector_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_primary_selector(primary_selector_ast *node)
      {
        visit_node(node->class_access);
        visit_node(node->this_access);
        visit_node(node->new_expression);
        visit_node(node->simple_name_access);
        visit_node(node->super_access);
        visit_node(node->method_call);
        visit_node(node->array_access);
      }

      virtual void visit_qualified_identifier(qualified_identifier_ast *node)
      {
        if (node->name_sequence)
          {
            const list_node<identifier_ast*> *__it = node->name_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast *node)
      {
        if (node->name_sequence)
          {
            const list_node<identifier_ast*> *__it = node->name_sequence->to_front(), *__end = __it;
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
        visit_node(node->expression);
        if (node->additional_expression_sequence)
          {
            const list_node<relational_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->instanceof_type);
      }

      virtual void visit_relational_expression_rest(relational_expression_rest_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_return_statement(return_statement_ast *node)
                               {
                                 visit_node(node->return_expression);
                               }

                               virtual void visit_shift_expression(shift_expression_ast *node)
                               {
                                 visit_node(node->expression);
                                 if (node->additional_expression_sequence)
                                   {
                                     const list_node<shift_expression_rest_ast*> *__it = node->additional_expression_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_shift_expression_rest(shift_expression_rest_ast *node)
                               {
                                 visit_node(node->expression);
                               }

                               virtual void visit_simple_name_access_data(simple_name_access_data_ast *node)
                               {
                                 visit_node(node->name);
                               }

                               virtual void visit_statement_expression(statement_expression_ast *node)
                               {
                                 visit_node(node->expression);
                               }

                               virtual void visit_super_access_data(super_access_data_ast *node)
                               {
                                 visit_node(node->super_suffix);
                                 visit_node(node->type_arguments);
                               }

                               virtual void visit_super_suffix(super_suffix_ast *node)
                               {
                                 visit_node(node->constructor_arguments);
                                 visit_node(node->simple_name_access);
                                 visit_node(node->method_call);
                               }

                               virtual void visit_switch_label(switch_label_ast *node)
                               {
                                 visit_node(node->case_expression);
                               }

                               virtual void visit_switch_section(switch_section_ast *node)
                               {
                                 if (node->label_sequence)
                                   {
                                     const list_node<switch_label_ast*> *__it = node->label_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                                 if (node->statement_sequence)
                                   {
                                     const list_node<block_statement_ast*> *__it = node->statement_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_switch_statement(switch_statement_ast *node)
                               {
                                 visit_node(node->switch_expression);
                                 if (node->switch_section_sequence)
                                   {
                                     const list_node<switch_section_ast*> *__it = node->switch_section_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_synchronized_statement(synchronized_statement_ast *node)
                               {
                                 visit_node(node->locked_type);
                                 visit_node(node->synchronized_body);
                               }

                               virtual void visit_this_access_data(this_access_data_ast *)
                               {}

                               virtual void visit_this_call_data(this_call_data_ast *node)
                               {
                                 visit_node(node->arguments);
                                 visit_node(node->type_arguments);
                               }

                               virtual void visit_throw_statement(throw_statement_ast *node)
                               {
                                 visit_node(node->exception);
                               }

                               virtual void visit_throws_clause(throws_clause_ast *node)
                               {
                                 if (node->identifier_sequence)
                                   {
                                     const list_node<qualified_identifier_ast*> *__it = node->identifier_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_try_statement(try_statement_ast *node)
                               {
                                 visit_node(node->try_body);
                                 if (node->catch_clause_sequence)
                                   {
                                     const list_node<catch_clause_ast*> *__it = node->catch_clause_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                                 visit_node(node->finally_body);
                               }

                               virtual void visit_type(type_ast *node)
                               {
                                 visit_node(node->class_type);
                                 visit_node(node->builtin_type);
                               }

                               virtual void visit_type_argument(type_argument_ast *node)
                               {
                                 visit_node(node->type_argument_type);
                                 visit_node(node->wildcard_type);
                               }

                               virtual void visit_type_argument_type(type_argument_type_ast *node)
                               {
                                 visit_node(node->class_type);
                                 visit_node(node->mandatory_array_builtin_type);
                               }

                               virtual void visit_type_arguments(type_arguments_ast *node)
                               {
                                 if (node->type_argument_sequence)
                                   {
                                     const list_node<type_argument_ast*> *__it = node->type_argument_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *)
                             {}

                               virtual void visit_type_declaration(type_declaration_ast *node)
                               {
                                 visit_node(node->class_declaration);
                                 visit_node(node->enum_declaration);
                                 visit_node(node->interface_declaration);
                                 visit_node(node->annotation_type_declaration);
                               }

                               virtual void visit_type_parameter(type_parameter_ast *node)
                               {
                                 visit_node(node->identifier);
                                 if (node->extends_type_sequence)
                                   {
                                     const list_node<class_or_interface_type_name_ast*> *__it = node->extends_type_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_type_parameters(type_parameters_ast *node)
                               {
                                 if (node->type_parameter_sequence)
                                   {
                                     const list_node<type_parameter_ast*> *__it = node->type_parameter_sequence->to_front(), *__end = __it;
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
                                 visit_node(node->unary_expression);
                                 visit_node(node->unary_expression_not_plusminus);
                               }

                               virtual void visit_unary_expression_not_plusminus(unary_expression_not_plusminus_ast *node)
                               {
                                 visit_node(node->bitwise_not_expression);
                                 visit_node(node->logical_not_expression);
                                 visit_node(node->cast_expression);
                                 visit_node(node->primary_expression);
                                 if (node->postfix_operator_sequence)
                                   {
                                     const list_node<postfix_operator_ast*> *__it = node->postfix_operator_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_variable_array_initializer(variable_array_initializer_ast *node)
                               {
                                 if (node->variable_initializer_sequence)
                                   {
                                     const list_node<variable_initializer_ast*> *__it = node->variable_initializer_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_variable_declaration(variable_declaration_ast *node)
                               {
                                 visit_node(node->data);
                               }

                               virtual void visit_variable_declaration_data(variable_declaration_data_ast *node)
                               {
                                 if (node->declarator_sequence)
                                   {
                                     const list_node<variable_declarator_ast*> *__it = node->declarator_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                                 visit_node(node->type);
                                 visit_node(node->modifiers);
                               }

                               virtual void visit_variable_declaration_rest(variable_declaration_rest_ast *node)
                               {
                                 visit_node(node->first_initializer);
                                 if (node->variable_declarator_sequence)
                                   {
                                     const list_node<variable_declarator_ast*> *__it = node->variable_declarator_sequence->to_front(), *__end = __it;
                                     do
                                       {
                                         visit_node(__it->element);
                                         __it = __it->next;
                                       }
                                     while (__it != __end);
                                   }
                               }

                               virtual void visit_variable_declaration_split_data(variable_declaration_split_data_ast *node)
                               {
                                 visit_node(node->data);
                               }

                               virtual void visit_variable_declarator(variable_declarator_ast *node)
                               {
                                 visit_node(node->variable_name);
                                 visit_node(node->declarator_brackets);
                                 visit_node(node->initializer);
                               }

                               virtual void visit_variable_initializer(variable_initializer_ast *node)
                               {
                                 visit_node(node->expression);
                                 visit_node(node->array_initializer);
                               }

                               virtual void visit_while_statement(while_statement_ast *node)
                               {
                                 visit_node(node->condition);
                                 visit_node(node->body);
                               }

                               virtual void visit_wildcard_type(wildcard_type_ast *node)
                               {
                                 visit_node(node->bounds);
                               }

                               virtual void visit_wildcard_type_bounds(wildcard_type_bounds_ast *node)
                               {
                                 visit_node(node->type);
                               }

                             };

} // end of namespace java

#endif


