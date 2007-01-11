// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef java_AST_H_INCLUDED
#define java_AST_H_INCLUDED

#include <kdev-pg-list.h>

#include <kdevast.h>


#include <string>

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
  struct variable_declaration_statement_ast;
  struct variable_declarator_ast;
  struct variable_initializer_ast;
  struct while_statement_ast;
  struct wildcard_type_ast;
  struct wildcard_type_bounds_ast;

  namespace additive_expression_rest
    {
    enum additive_operator_enum {
      op_plus,
      op_minus
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
      type_double
    };
  }

  namespace equality_expression_rest
    {
    enum equality_operator_enum {
      op_equal,
      op_not_equal
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
      op_unsigned_rshift_assign
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
      type_string
    };
  }

  namespace modifiers
    {
    enum modifier_enum {
      mod_private = 1,
      mod_public = 1 << 1,
      mod_protected = 1 << 2,
      mod_static = 1 << 3,
      mod_transient = 1 << 4,
      mod_final = 1 << 5,
      mod_abstract = 1 << 6,
      mod_native = 1 << 7,
      mod_synchronized = 1 << 8,
      mod_volatile = 1 << 9,
      mod_strictfp = 1 << 10
    };
  }

  namespace multiplicative_expression_rest
    {
    enum multiplicative_operator_enum {
      op_star,
      op_slash,
      op_remainder
    };
  }

  namespace postfix_operator
    {
    enum postfix_operator_enum {
      op_increment,
      op_decrement
    };
  }

  namespace relational_expression_rest
    {
    enum relational_operator_enum {
      op_less_than,
      op_greater_than,
      op_less_equal,
      op_greater_equal
    };
  }

  namespace shift_expression_rest
    {
    enum shift_operator_enum {
      op_lshift,
      op_signed_rshift,
      op_unsigned_rshift
    };
  }

  namespace switch_label
    {
    enum branch_type_enum {
      case_branch,
      default_branch
    };
  }

  namespace unary_expression
    {
    enum unary_expression_enum {
      type_incremented_expression,
      type_decremented_expression,
      type_unary_minus_expression,
      type_unary_plus_expression,
      type_unary_expression_not_plusminus
    };
  }

  namespace unary_expression_not_plusminus
    {
    enum unary_expression_not_plusminus_enum {
      type_bitwise_not_expression,
      type_logical_not_expression,
      type_cast_expression,
      type_primary_expression
    };
  }

  namespace wildcard_type_bounds
    {
    enum extends_or_super_enum {
      extends,
      super
    };
  }


  struct ast_node: public KDevAST
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
        Kind_variable_declaration_statement = 1122,
        Kind_variable_declarator = 1123,
        Kind_variable_initializer = 1124,
        Kind_while_statement = 1125,
        Kind_wildcard_type = 1126,
        Kind_wildcard_type_bounds = 1127,
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

      variable_declaration_statement_ast *variable_declaration_statement;
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

      unsigned int modifiers;
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
      array_type_dot_class_ast *array_type_dot_class;
      simple_name_access_data_ast *simple_name_access;
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

  struct variable_declaration_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration_statement
      };

      variable_declaration_ast *variable_declaration;
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

#endif


