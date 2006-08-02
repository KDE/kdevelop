// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_h_INCLUDED
#define csharp_h_INCLUDED

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-list.h"
#include "kdev-pg-token-stream.h"

#include <kdevast.h>

#include <cassert>

namespace csharp
  {

  struct accessor_declarations_ast;
  struct accessor_modifier_ast;
  struct additive_expression_ast;
  struct additive_expression_rest_ast;
  struct anonymous_method_expression_ast;
  struct anonymous_method_parameter_ast;
  struct anonymous_method_signature_ast;
  struct argument_ast;
  struct array_creation_expression_rest_ast;
  struct array_initializer_ast;
  struct array_type_ast;
  struct attribute_ast;
  struct attribute_arguments_ast;
  struct attribute_section_ast;
  struct attribute_target_ast;
  struct base_access_ast;
  struct bit_and_expression_ast;
  struct bit_or_expression_ast;
  struct bit_xor_expression_ast;
  struct block_ast;
  struct block_statement_ast;
  struct boolean_expression_ast;
  struct break_statement_ast;
  struct builtin_class_type_ast;
  struct cast_expression_ast;
  struct catch_clauses_ast;
  struct checked_statement_ast;
  struct class_base_ast;
  struct class_body_ast;
  struct class_declaration_ast;
  struct class_member_declaration_ast;
  struct class_or_struct_member_declaration_ast;
  struct class_type_ast;
  struct compilation_unit_ast;
  struct conditional_expression_ast;
  struct constant_declaration_ast;
  struct constant_declaration_data_ast;
  struct constant_declarator_ast;
  struct constant_expression_ast;
  struct constructor_constraint_ast;
  struct constructor_declaration_ast;
  struct constructor_initializer_ast;
  struct continue_statement_ast;
  struct conversion_operator_declaration_ast;
  struct delegate_declaration_ast;
  struct do_while_statement_ast;
  struct embedded_statement_ast;
  struct enum_base_ast;
  struct enum_body_ast;
  struct enum_declaration_ast;
  struct enum_member_declaration_ast;
  struct equality_expression_ast;
  struct equality_expression_rest_ast;
  struct event_accessor_declarations_ast;
  struct event_declaration_ast;
  struct expression_ast;
  struct extern_alias_directive_ast;
  struct finalizer_declaration_ast;
  struct fixed_pointer_declarator_ast;
  struct fixed_statement_ast;
  struct floating_point_type_ast;
  struct for_control_ast;
  struct for_statement_ast;
  struct foreach_statement_ast;
  struct formal_parameter_ast;
  struct formal_parameter_list_ast;
  struct general_catch_clause_ast;
  struct generic_dimension_specifier_ast;
  struct global_attribute_section_ast;
  struct goto_statement_ast;
  struct identifier_ast;
  struct if_statement_ast;
  struct indexer_declaration_ast;
  struct integral_type_ast;
  struct interface_accessors_ast;
  struct interface_base_ast;
  struct interface_body_ast;
  struct interface_declaration_ast;
  struct interface_event_declaration_ast;
  struct interface_indexer_declaration_ast;
  struct interface_member_declaration_ast;
  struct interface_method_declaration_ast;
  struct interface_property_declaration_ast;
  struct keyword_ast;
  struct labeled_statement_ast;
  struct literal_ast;
  struct local_constant_declaration_ast;
  struct local_variable_declaration_ast;
  struct local_variable_declaration_statement_ast;
  struct lock_statement_ast;
  struct logical_and_expression_ast;
  struct logical_or_expression_ast;
  struct managed_type_ast;
  struct method_declaration_ast;
  struct multiplicative_expression_ast;
  struct multiplicative_expression_rest_ast;
  struct named_argument_ast;
  struct namespace_body_ast;
  struct namespace_declaration_ast;
  struct namespace_member_declaration_ast;
  struct namespace_name_ast;
  struct namespace_or_type_name_ast;
  struct namespace_or_type_name_part_ast;
  struct namespace_or_type_name_safe_ast;
  struct new_expression_ast;
  struct non_array_type_ast;
  struct non_nullable_type_ast;
  struct null_coalescing_expression_ast;
  struct numeric_type_ast;
  struct object_or_delegate_creation_expression_rest_ast;
  struct optional_argument_list_ast;
  struct optional_attribute_sections_ast;
  struct optional_modifiers_ast;
  struct optionally_nullable_type_ast;
  struct overloadable_binary_only_operator_ast;
  struct overloadable_unary_only_operator_ast;
  struct overloadable_unary_or_binary_operator_ast;
  struct parameter_array_ast;
  struct parameter_modifier_ast;
  struct pointer_type_ast;
  struct positional_argument_ast;
  struct predefined_type_ast;
  struct primary_atom_ast;
  struct primary_expression_ast;
  struct primary_or_secondary_constraint_ast;
  struct primary_suffix_ast;
  struct property_declaration_ast;
  struct qualified_identifier_ast;
  struct rank_specifier_ast;
  struct relational_expression_ast;
  struct relational_expression_rest_ast;
  struct resource_acquisition_ast;
  struct return_statement_ast;
  struct return_type_ast;
  struct secondary_constraints_ast;
  struct shift_expression_ast;
  struct shift_expression_rest_ast;
  struct simple_name_or_member_access_ast;
  struct simple_type_ast;
  struct specific_catch_clause_ast;
  struct stackalloc_initializer_ast;
  struct statement_expression_ast;
  struct struct_body_ast;
  struct struct_declaration_ast;
  struct struct_interfaces_ast;
  struct struct_member_declaration_ast;
  struct switch_label_ast;
  struct switch_section_ast;
  struct switch_statement_ast;
  struct throw_statement_ast;
  struct try_statement_ast;
  struct type_ast;
  struct type_arguments_ast;
  struct type_arguments_or_parameters_end_ast;
  struct type_declaration_ast;
  struct type_declaration_rest_ast;
  struct type_name_ast;
  struct type_name_safe_ast;
  struct type_parameter_ast;
  struct type_parameter_constraints_ast;
  struct type_parameter_constraints_clause_ast;
  struct type_parameters_ast;
  struct typeof_expression_ast;
  struct unary_expression_ast;
  struct unary_or_binary_operator_declaration_ast;
  struct unbound_type_name_ast;
  struct unbound_type_name_part_ast;
  struct unchecked_statement_ast;
  struct unmanaged_type_ast;
  struct unmanaged_type_suffix_ast;
  struct unsafe_statement_ast;
  struct using_directive_ast;
  struct using_statement_ast;
  struct variable_declaration_data_ast;
  struct variable_declarator_ast;
  struct variable_initializer_ast;
  struct while_statement_ast;
  struct yield_statement_ast;

  namespace accessor_declarations
    {
    enum accessor_declarations_enum {
      type_get,
      type_set,
      type_none,  // only possible for the second, optional accessor
    };
  }

  namespace additive_expression_rest
    {
    enum additive_operator_enum {
      op_plus,
      op_minus
    };
  }

  namespace argument
    {
    enum argument_type_enum {
      type_value_parameter,
      type_reference_parameter,
      type_output_parameter,
    };
  }

  namespace base_access
    {
    enum base_access_enum {
      type_base_member_access,
      type_base_indexer_access,
    };
  }

  namespace builtin_class_type
    {
    enum builtin_class_type_enum {
      type_object,
      type_string,
    };
  }

  namespace constructor_initializer
    {
    enum constructor_initializer_type_enum {
      type_base,
      type_this,
    };
  }

  namespace conversion_operator_declaration
    {
    enum conversion_type_enum {
      conversion_implicit,
      conversion_explicit,
    };
  }

  namespace equality_expression_rest
    {
    enum equality_operator_enum {
      op_equal,
      op_not_equal,
    };
  }

  namespace event_accessor_declarations
    {
    enum event_accessor_declarations_enum {
      order_add_remove,
      order_remove_add,
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
      op_remainder_assign,
      op_bit_and_assign,
      op_bit_or_assign,
      op_bit_xor_assign,
      op_lshift_assign,
      op_rshift_assign,
    };
  }

  namespace floating_point_type
    {
    enum floating_point_type_enum {
      type_float,
      type_double,
    };
  }

  namespace goto_statement
    {
    enum goto_statement_enum {
      type_labeled_statement,
      type_switch_case,
      type_switch_default,
    };
  }

  namespace integral_type
    {
    enum integral_type_enum {
      type_sbyte,
      type_byte,
      type_short,
      type_ushort,
      type_int,
      type_uint,
      type_long,
      type_ulong,
      type_char,
    };
  }

  namespace literal
    {
    enum literal_type_enum {
      type_true,
      type_false,
      type_null,
      type_integer,
      type_real,
      type_character,
      type_string
    };
  }

  namespace modifiers
    {
    enum modifier_enum {
      mod_new = 1,
      mod_public = 1 << 1,
      mod_protected = 1 << 2,
      mod_internal = 1 << 3,
      mod_private = 1 << 4,
      mod_abstract = 1 << 5,
      mod_sealed = 1 << 6,
      mod_static = 1 << 7,
      mod_readonly = 1 << 8,
      mod_volatile = 1 << 9,
      mod_virtual = 1 << 10,
      mod_override = 1 << 11,
      mod_extern = 1 << 12,
      mod_unsafe = 1 << 13,
      mod_fixed = 1 << 14,
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

  namespace numeric_type
    {
    enum numeric_type_enum {
      type_integral,
      type_floating_point,
      type_decimal,
    };
  }

  namespace overloadable_operator
    {
    enum unary_or_binary_enum {
      type_unary,
      type_binary,
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
      op_less_equal,
    };
  }

  namespace parameter_modifier
    {
    enum parameter_modifier_enum {
      mod_ref,
      mod_out,
    };
  }

  namespace pointer_type
    {
    enum pointer_type_enum {
      type_regular,
      type_void_star,
    };
  }

  namespace predefined_type
    {
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
      type_ushort,
    };
  }

  namespace primary_atom
    {
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
      type_sizeof_expression,
    };
  }

  namespace primary_or_secondary_constraint
    {
    enum primary_or_secondary_constraint_enum {
      type_type,
      type_class,
      type_struct,
    };
  }

  namespace primary_suffix
    {
    enum primary_suffix_enum {
      type_member_access,
      type_pointer_member_access,
      type_invocation,
      type_element_access,
      type_increment,
      type_decrement,
    };
  }

  namespace relational_expression_rest
    {
    enum relational_operator_enum {
      op_less_than,
      op_greater_than,
      op_less_equal,
      op_greater_equal,
      op_is,
      op_as,
    };
  }

  namespace return_type
                    {
                    enum return_type_enum {
                                 type_regular,
                                 type_void,
                               };
                  }

                  namespace shift_expression_rest
                    {
                    enum shift_operator_enum {
                      op_lshift,
                      op_rshift,
                    };
                  }

                  namespace simple_type
                    {
                    enum simple_type_enum {
                      type_numeric,
                      type_bool,
                    };
                  }

                  namespace switch_label
                    {
                    enum branch_type_enum {
                      case_branch,
                      default_branch
                    };
                  }

                  namespace typeof_expression
                    {
                    enum typeof_expression_enum {
                      type_void,
                      type_unbound_type_name,
                      type_type,
                    };
                  }

                  namespace unary_expression
                    {
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
                      type_addressof_expression,
                    };
                  }

                  namespace unmanaged_type_suffix
                    {
                    enum suffix_type {
                      type_star,
                      type_rank_specifier
                    };
                  }

                  namespace yield_statement
                    {
                    enum yield_statement_enum {
                      type_yield_return,
                      type_yield_break,
                    };
                  }


                  struct ast_node: public KDevAST
                    {
                      enum ast_node_kind_enum {
                        Kind_accessor_declarations = 1000,
                        Kind_accessor_modifier = 1001,
                        Kind_additive_expression = 1002,
                        Kind_additive_expression_rest = 1003,
                        Kind_anonymous_method_expression = 1004,
                        Kind_anonymous_method_parameter = 1005,
                        Kind_anonymous_method_signature = 1006,
                        Kind_argument = 1007,
                        Kind_array_creation_expression_rest = 1008,
                        Kind_array_initializer = 1009,
                        Kind_array_type = 1010,
                        Kind_attribute = 1011,
                        Kind_attribute_arguments = 1012,
                        Kind_attribute_section = 1013,
                        Kind_attribute_target = 1014,
                        Kind_base_access = 1015,
                        Kind_bit_and_expression = 1016,
                        Kind_bit_or_expression = 1017,
                        Kind_bit_xor_expression = 1018,
                        Kind_block = 1019,
                        Kind_block_statement = 1020,
                        Kind_boolean_expression = 1021,
                        Kind_break_statement = 1022,
                        Kind_builtin_class_type = 1023,
                        Kind_cast_expression = 1024,
                        Kind_catch_clauses = 1025,
                        Kind_checked_statement = 1026,
                        Kind_class_base = 1027,
                        Kind_class_body = 1028,
                        Kind_class_declaration = 1029,
                        Kind_class_member_declaration = 1030,
                        Kind_class_or_struct_member_declaration = 1031,
                        Kind_class_type = 1032,
                        Kind_compilation_unit = 1033,
                        Kind_conditional_expression = 1034,
                        Kind_constant_declaration = 1035,
                        Kind_constant_declaration_data = 1036,
                        Kind_constant_declarator = 1037,
                        Kind_constant_expression = 1038,
                        Kind_constructor_constraint = 1039,
                        Kind_constructor_declaration = 1040,
                        Kind_constructor_initializer = 1041,
                        Kind_continue_statement = 1042,
                        Kind_conversion_operator_declaration = 1043,
                        Kind_delegate_declaration = 1044,
                        Kind_do_while_statement = 1045,
                        Kind_embedded_statement = 1046,
                        Kind_enum_base = 1047,
                        Kind_enum_body = 1048,
                        Kind_enum_declaration = 1049,
                        Kind_enum_member_declaration = 1050,
                        Kind_equality_expression = 1051,
                        Kind_equality_expression_rest = 1052,
                        Kind_event_accessor_declarations = 1053,
                        Kind_event_declaration = 1054,
                        Kind_expression = 1055,
                        Kind_extern_alias_directive = 1056,
                        Kind_finalizer_declaration = 1057,
                        Kind_fixed_pointer_declarator = 1058,
                        Kind_fixed_statement = 1059,
                        Kind_floating_point_type = 1060,
                        Kind_for_control = 1061,
                        Kind_for_statement = 1062,
                        Kind_foreach_statement = 1063,
                        Kind_formal_parameter = 1064,
                        Kind_formal_parameter_list = 1065,
                        Kind_general_catch_clause = 1066,
                        Kind_generic_dimension_specifier = 1067,
                        Kind_global_attribute_section = 1068,
                        Kind_goto_statement = 1069,
                        Kind_identifier = 1070,
                        Kind_if_statement = 1071,
                        Kind_indexer_declaration = 1072,
                        Kind_integral_type = 1073,
                        Kind_interface_accessors = 1074,
                        Kind_interface_base = 1075,
                        Kind_interface_body = 1076,
                        Kind_interface_declaration = 1077,
                        Kind_interface_event_declaration = 1078,
                        Kind_interface_indexer_declaration = 1079,
                        Kind_interface_member_declaration = 1080,
                        Kind_interface_method_declaration = 1081,
                        Kind_interface_property_declaration = 1082,
                        Kind_keyword = 1083,
                        Kind_labeled_statement = 1084,
                        Kind_literal = 1085,
                        Kind_local_constant_declaration = 1086,
                        Kind_local_variable_declaration = 1087,
                        Kind_local_variable_declaration_statement = 1088,
                        Kind_lock_statement = 1089,
                        Kind_logical_and_expression = 1090,
                        Kind_logical_or_expression = 1091,
                        Kind_managed_type = 1092,
                        Kind_method_declaration = 1093,
                        Kind_multiplicative_expression = 1094,
                        Kind_multiplicative_expression_rest = 1095,
                        Kind_named_argument = 1096,
                        Kind_namespace_body = 1097,
                        Kind_namespace_declaration = 1098,
                        Kind_namespace_member_declaration = 1099,
                        Kind_namespace_name = 1100,
                        Kind_namespace_or_type_name = 1101,
                        Kind_namespace_or_type_name_part = 1102,
                        Kind_namespace_or_type_name_safe = 1103,
                        Kind_new_expression = 1104,
                        Kind_non_array_type = 1105,
                        Kind_non_nullable_type = 1106,
                        Kind_null_coalescing_expression = 1107,
                        Kind_numeric_type = 1108,
                        Kind_object_or_delegate_creation_expression_rest = 1109,
                        Kind_optional_argument_list = 1110,
                        Kind_optional_attribute_sections = 1111,
                        Kind_optional_modifiers = 1112,
                        Kind_optionally_nullable_type = 1113,
                        Kind_overloadable_binary_only_operator = 1114,
                        Kind_overloadable_unary_only_operator = 1115,
                        Kind_overloadable_unary_or_binary_operator = 1116,
                        Kind_parameter_array = 1117,
                        Kind_parameter_modifier = 1118,
                        Kind_pointer_type = 1119,
                        Kind_positional_argument = 1120,
                        Kind_predefined_type = 1121,
                        Kind_primary_atom = 1122,
                        Kind_primary_expression = 1123,
                        Kind_primary_or_secondary_constraint = 1124,
                        Kind_primary_suffix = 1125,
                        Kind_property_declaration = 1126,
                        Kind_qualified_identifier = 1127,
                        Kind_rank_specifier = 1128,
                        Kind_relational_expression = 1129,
                        Kind_relational_expression_rest = 1130,
                        Kind_resource_acquisition = 1131,
                        Kind_return_statement = 1132,
                        Kind_return_type = 1133,
                        Kind_secondary_constraints = 1134,
                        Kind_shift_expression = 1135,
                        Kind_shift_expression_rest = 1136,
                        Kind_simple_name_or_member_access = 1137,
                        Kind_simple_type = 1138,
                        Kind_specific_catch_clause = 1139,
                        Kind_stackalloc_initializer = 1140,
                        Kind_statement_expression = 1141,
                        Kind_struct_body = 1142,
                        Kind_struct_declaration = 1143,
                        Kind_struct_interfaces = 1144,
                        Kind_struct_member_declaration = 1145,
                        Kind_switch_label = 1146,
                        Kind_switch_section = 1147,
                        Kind_switch_statement = 1148,
                        Kind_throw_statement = 1149,
                        Kind_try_statement = 1150,
                        Kind_type = 1151,
                        Kind_type_arguments = 1152,
                        Kind_type_arguments_or_parameters_end = 1153,
                        Kind_type_declaration = 1154,
                        Kind_type_declaration_rest = 1155,
                        Kind_type_name = 1156,
                        Kind_type_name_safe = 1157,
                        Kind_type_parameter = 1158,
                        Kind_type_parameter_constraints = 1159,
                        Kind_type_parameter_constraints_clause = 1160,
                        Kind_type_parameters = 1161,
                        Kind_typeof_expression = 1162,
                        Kind_unary_expression = 1163,
                        Kind_unary_or_binary_operator_declaration = 1164,
                        Kind_unbound_type_name = 1165,
                        Kind_unbound_type_name_part = 1166,
                        Kind_unchecked_statement = 1167,
                        Kind_unmanaged_type = 1168,
                        Kind_unmanaged_type_suffix = 1169,
                        Kind_unsafe_statement = 1170,
                        Kind_using_directive = 1171,
                        Kind_using_statement = 1172,
                        Kind_variable_declaration_data = 1173,
                        Kind_variable_declarator = 1174,
                        Kind_variable_initializer = 1175,
                        Kind_while_statement = 1176,
                        Kind_yield_statement = 1177,
                        AST_NODE_KIND_COUNT
                      };

                      int kind;
                      std::size_t start_token;
                      std::size_t end_token;
                    };

  struct accessor_declarations_ast: public ast_node
    {
      enum
      {
        KIND = Kind_accessor_declarations
      };

      accessor_declarations::accessor_declarations_enum accessor1_type;
      accessor_declarations::accessor_declarations_enum accessor2_type;
      optional_attribute_sections_ast *accessor1_attributes;
      accessor_modifier_ast *accessor1_modifier;
      block_ast *accessor1_body;
      optional_attribute_sections_ast *accessor2_attributes;
      accessor_modifier_ast *accessor2_modifier;
      block_ast *accessor2_body;
    };

  struct accessor_modifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_accessor_modifier
      };

      unsigned int modifiers;
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

  struct anonymous_method_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_anonymous_method_expression
      };

      anonymous_method_signature_ast *anonymous_method_signature;
      block_ast *body;
    };

  struct anonymous_method_parameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_anonymous_method_parameter
      };

      parameter_modifier_ast *modifier;
      type_ast *type;
      identifier_ast *variable_name;
    };

  struct anonymous_method_signature_ast: public ast_node
    {
      enum
      {
        KIND = Kind_anonymous_method_signature
      };

      const list_node<anonymous_method_parameter_ast *> *anonymous_method_parameter_sequence;
    };

  struct argument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_argument
      };

      argument::argument_type_enum argument_type;
      expression_ast *expression;
    };

  struct array_creation_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_creation_expression_rest
      };

      type_ast *type;
      array_initializer_ast *array_initializer;
      const list_node<expression_ast *> *expression_sequence;
      const list_node<rank_specifier_ast *> *rank_specifier_sequence;
    };

  struct array_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_initializer
      };

      const list_node<variable_initializer_ast *> *variable_initializer_sequence;
    };

  struct array_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_array_type
      };

      non_array_type_ast *non_array_type;
      const list_node<rank_specifier_ast *> *rank_specifier_sequence;
    };

  struct attribute_ast: public ast_node
    {
      enum
      {
        KIND = Kind_attribute
      };

      type_name_ast *name;
      attribute_arguments_ast *arguments;
    };

  struct attribute_arguments_ast: public ast_node
    {
      enum
      {
        KIND = Kind_attribute_arguments
      };

      const list_node<named_argument_ast *> *named_argument_sequence;
      const list_node<positional_argument_ast *> *positional_argument_sequence;
    };

  struct attribute_section_ast: public ast_node
    {
      enum
      {
        KIND = Kind_attribute_section
      };

      attribute_target_ast *target;
      const list_node<attribute_ast *> *attribute_sequence;
    };

  struct attribute_target_ast: public ast_node
    {
      enum
      {
        KIND = Kind_attribute_target
      };

      identifier_ast *identifier;
      keyword_ast *keyword;
    };

  struct base_access_ast: public ast_node
    {
      enum
      {
        KIND = Kind_base_access
      };

      base_access::base_access_enum access_type;
      identifier_ast *identifier;
      type_arguments_ast *type_arguments;
      const list_node<expression_ast *> *expression_sequence;
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

      labeled_statement_ast *labeled_statement;
      local_constant_declaration_ast *local_constant_declaration_statement;
      local_variable_declaration_statement_ast *local_variable_declaration_statement;
      embedded_statement_ast *statement;
    };

  struct boolean_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_boolean_expression
      };

      expression_ast *expression;
    };

  struct break_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_break_statement
      };

    };

  struct builtin_class_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_builtin_class_type
      };

      builtin_class_type::builtin_class_type_enum type;
    };

  struct cast_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_cast_expression
      };

      type_ast *type;
      unary_expression_ast *casted_expression;
    };

  struct catch_clauses_ast: public ast_node
    {
      enum
      {
        KIND = Kind_catch_clauses
      };

      general_catch_clause_ast *general_catch_clause;
      const list_node<specific_catch_clause_ast *> *specific_catch_clause_sequence;
    };

  struct checked_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_checked_statement
      };

      block_ast *body;
    };

  struct class_base_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_base
      };

      builtin_class_type_ast *builtin_class_type;
      const list_node<type_name_ast *> *interface_type_sequence;
      const list_node<type_name_ast *> *base_type_sequence;
    };

  struct class_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_body
      };

      const list_node<class_member_declaration_ast *> *member_declaration_sequence;
    };

  struct class_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      bool partial;
      identifier_ast *class_name;
      type_parameters_ast *type_parameters;
      class_base_ast *class_base;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
      class_body_ast *body;
    };

  struct class_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_member_declaration
      };

      finalizer_declaration_ast *finalizer_declaration;
      class_or_struct_member_declaration_ast *other_declaration;
    };

  struct class_or_struct_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_or_struct_member_declaration
      };

      constant_declaration_ast *constant_declaration;
      event_declaration_ast *event_declaration;
      conversion_operator_declaration_ast *conversion_operator_declaration;
      constructor_declaration_ast *constructor_declaration;
      type_declaration_rest_ast *type_declaration_rest;
      unary_or_binary_operator_declaration_ast *unary_or_binary_operator_declaration;
      indexer_declaration_ast *indexer_declaration;
      variable_declaration_data_ast *field_declaration;
      property_declaration_ast *property_declaration;
      method_declaration_ast *method_declaration;
    };

  struct class_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_class_type
      };

      type_name_ast *type_name;
      builtin_class_type_ast *builtin_class_type;
    };

  struct compilation_unit_ast: public ast_node
    {
      enum
      {
        KIND = Kind_compilation_unit
      };

      const list_node<extern_alias_directive_ast *> *extern_alias_sequence;
      const list_node<using_directive_ast *> *using_sequence;
      const list_node<global_attribute_section_ast *> *global_attribute_sequence;
      const list_node<namespace_member_declaration_ast *> *namespace_sequence;
    };

  struct conditional_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_conditional_expression
      };

      null_coalescing_expression_ast *null_coalescing_expression;
      expression_ast *if_expression;
      expression_ast *else_expression;
    };

  struct constant_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constant_declaration
      };

      constant_declaration_data_ast *data;
    };

  struct constant_declaration_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constant_declaration_data
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *type;
      const list_node<constant_declarator_ast *> *constant_declarator_sequence;
    };

  struct constant_declarator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constant_declarator
      };

      identifier_ast *constant_name;
      constant_expression_ast *expression;
    };

  struct constant_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constant_expression
      };

      expression_ast *expression;
    };

  struct constructor_constraint_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constructor_constraint
      };

    };

  struct constructor_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constructor_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      identifier_ast *class_name;
      formal_parameter_list_ast *formal_parameters;
      constructor_initializer_ast *constructor_initializer;
      block_ast *body;
    };

  struct constructor_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_constructor_initializer
      };

      constructor_initializer::constructor_initializer_type_enum initializer_type;
      optional_argument_list_ast *arguments;
    };

  struct continue_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_continue_statement
      };

    };

  struct conversion_operator_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_conversion_operator_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      conversion_operator_declaration::conversion_type_enum conversion_type;
      type_ast *target_type;
      type_ast *source_type;
      identifier_ast *source_name;
      block_ast *body;
    };

  struct delegate_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_delegate_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      return_type_ast *return_type;
      identifier_ast *delegate_name;
      type_parameters_ast *type_parameters;
      formal_parameter_list_ast *formal_parameters;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
    };

  struct do_while_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_do_while_statement
      };

      embedded_statement_ast *body;
      boolean_expression_ast *condition;
    };

  struct embedded_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_embedded_statement
      };

      block_ast *block;
      if_statement_ast *if_statement;
      switch_statement_ast *switch_statement;
      while_statement_ast *while_statement;
      do_while_statement_ast *do_while_statement;
      for_statement_ast *for_statement;
      foreach_statement_ast *foreach_statement;
      break_statement_ast *break_statement;
      continue_statement_ast *continue_statement;
      goto_statement_ast *goto_statement;
      return_statement_ast *return_statement;
      throw_statement_ast *throw_statement;
      try_statement_ast *try_statement;
      lock_statement_ast *lock_statement;
      using_statement_ast *using_statement;
      checked_statement_ast *checked_statement;
      unchecked_statement_ast *unchecked_statement;
      yield_statement_ast *yield_statement;
      statement_expression_ast *expression_statement;
      unsafe_statement_ast *unsafe_statement;
      fixed_statement_ast *fixed_statement;
    };

  struct enum_base_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_base
      };

      integral_type_ast *integral_type;
    };

  struct enum_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_body
      };

      const list_node<enum_member_declaration_ast *> *member_declaration_sequence;
    };

  struct enum_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      identifier_ast *enum_name;
      enum_base_ast *enum_base;
      enum_body_ast *body;
    };

  struct enum_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_enum_member_declaration
      };

      optional_attribute_sections_ast *attributes;
      identifier_ast *member_name;
      constant_expression_ast *constant_expression;
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

  struct event_accessor_declarations_ast: public ast_node
    {
      enum
      {
        KIND = Kind_event_accessor_declarations
      };

      event_accessor_declarations::event_accessor_declarations_enum order;
      optional_attribute_sections_ast *accessor1_attributes;
      block_ast *accessor1_body;
      optional_attribute_sections_ast *accessor2_attributes;
      block_ast *accessor2_body;
    };

  struct event_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_event_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *type;
      const list_node<variable_declarator_ast *> *variable_declarator_sequence;
      type_name_ast *event_name;
      event_accessor_declarations_ast *event_accessor_declarations;
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

  struct extern_alias_directive_ast: public ast_node
    {
      enum
      {
        KIND = Kind_extern_alias_directive
      };

      identifier_ast *identifier;
    };

  struct finalizer_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_finalizer_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      identifier_ast *class_name;
      block_ast *finalizer_body;
    };

  struct fixed_pointer_declarator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_fixed_pointer_declarator
      };

      identifier_ast *pointer_name;
      expression_ast *initializer;
    };

  struct fixed_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_fixed_statement
      };

      pointer_type_ast *pointer_type;
      fixed_pointer_declarator_ast *fixed_pointer_declarator;
      embedded_statement_ast *body;
    };

  struct floating_point_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_floating_point_type
      };

      floating_point_type::floating_point_type_enum type;
    };

  struct for_control_ast: public ast_node
    {
      enum
      {
        KIND = Kind_for_control
      };

      local_variable_declaration_ast *local_variable_declaration;
      const list_node<statement_expression_ast *> *statement_expression_sequence;
      boolean_expression_ast *for_condition;
      const list_node<statement_expression_ast *> *for_iterator_sequence;
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

  struct foreach_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_foreach_statement
      };

      type_ast *variable_type;
      identifier_ast *variable_name;
      expression_ast *collection;
      embedded_statement_ast *body;
    };

  struct formal_parameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_formal_parameter
      };

      optional_attribute_sections_ast *attributes;
      parameter_array_ast *parameter_array;
      parameter_modifier_ast *modifier;
      type_ast *type;
      identifier_ast *variable_name;
    };

  struct formal_parameter_list_ast: public ast_node
    {
      enum
      {
        KIND = Kind_formal_parameter_list
      };

      const list_node<formal_parameter_ast *> *formal_parameter_sequence;
    };

  struct general_catch_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_general_catch_clause
      };

      block_ast *body;
    };

  struct generic_dimension_specifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_generic_dimension_specifier
      };

      int comma_count;
    };

  struct global_attribute_section_ast: public ast_node
    {
      enum
      {
        KIND = Kind_global_attribute_section
      };

      const list_node<attribute_ast *> *attribute_sequence;
    };

  struct goto_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_goto_statement
      };

      goto_statement::goto_statement_enum goto_type;
      identifier_ast *label;
      constant_expression_ast *constant_expression;
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

      boolean_expression_ast *condition;
      embedded_statement_ast *if_body;
      embedded_statement_ast *else_body;
    };

  struct indexer_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_indexer_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *type;
      type_name_safe_ast *interface_type;
      formal_parameter_list_ast *formal_parameters;
      accessor_declarations_ast *accessor_declarations;
    };

  struct integral_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_integral_type
      };

      integral_type::integral_type_enum type;
    };

  struct interface_accessors_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_accessors
      };

      accessor_declarations::accessor_declarations_enum accessor1_type;
      accessor_declarations::accessor_declarations_enum accessor2_type;
      optional_attribute_sections_ast *accessor1_attributes;
      optional_attribute_sections_ast *accessor2_attributes;
    };

  struct interface_base_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_base
      };

      const list_node<type_name_ast *> *interface_type_sequence;
    };

  struct interface_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_body
      };

      const list_node<interface_member_declaration_ast *> *member_declaration_sequence;
    };

  struct interface_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      bool partial;
      identifier_ast *interface_name;
      type_parameters_ast *type_parameters;
      interface_base_ast *interface_base;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
      interface_body_ast *body;
    };

  struct interface_event_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_event_declaration
      };

      optional_attribute_sections_ast *attributes;
      bool decl_new;
      type_ast *event_type;
      identifier_ast *event_name;
    };

  struct interface_indexer_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_indexer_declaration
      };

      optional_attribute_sections_ast *attributes;
      bool decl_new;
      type_ast *type;
      formal_parameter_list_ast *formal_parameters;
      interface_accessors_ast *interface_accessors;
    };

  struct interface_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_member_declaration
      };

      interface_event_declaration_ast *event_declaration;
      interface_indexer_declaration_ast *indexer_declaration;
      interface_property_declaration_ast *interface_property_declaration;
      interface_method_declaration_ast *interface_method_declaration;
    };

  struct interface_method_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_method_declaration
      };

      optional_attribute_sections_ast *attributes;
      bool decl_new;
      return_type_ast *return_type;
      identifier_ast *method_name;
      type_parameters_ast *type_parameters;
      formal_parameter_list_ast *formal_parameters;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
    };

  struct interface_property_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_interface_property_declaration
      };

      optional_attribute_sections_ast *attributes;
      bool decl_new;
      type_ast *type;
      identifier_ast *property_name;
      interface_accessors_ast *interface_accessors;
    };

  struct keyword_ast: public ast_node
    {
      enum
      {
        KIND = Kind_keyword
      };

      std::size_t keyword;
    };

  struct labeled_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_labeled_statement
      };

      identifier_ast *label;
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

  struct local_constant_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_local_constant_declaration
      };

      constant_declaration_data_ast *data;
    };

  struct local_variable_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_local_variable_declaration
      };

      variable_declaration_data_ast *data;
    };

  struct local_variable_declaration_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_local_variable_declaration_statement
      };

      local_variable_declaration_ast *declaration;
    };

  struct lock_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_lock_statement
      };

      expression_ast *lock_expression;
      embedded_statement_ast *body;
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

  struct managed_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_managed_type
      };

      non_array_type_ast *non_array_type;
      const list_node<rank_specifier_ast *> *rank_specifier_sequence;
    };

  struct method_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_method_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      return_type_ast *return_type;
      type_name_safe_ast *method_name;
      type_parameters_ast *type_parameters;
      formal_parameter_list_ast *formal_parameters;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
      block_ast *method_body;
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

  struct named_argument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_named_argument
      };

      identifier_ast *argument_name;
      expression_ast *attribute_argument_expression;
    };

  struct namespace_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_body
      };

      const list_node<extern_alias_directive_ast *> *extern_alias_sequence;
      const list_node<using_directive_ast *> *using_sequence;
      const list_node<namespace_member_declaration_ast *> *namespace_sequence;
    };

  struct namespace_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_declaration
      };

      qualified_identifier_ast *name;
      namespace_body_ast *body;
    };

  struct namespace_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_member_declaration
      };

      namespace_declaration_ast *namespace_declaration;
      type_declaration_ast *type_declaration;
    };

  struct namespace_name_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_name
      };

    };

  struct namespace_or_type_name_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_or_type_name
      };

      identifier_ast *qualified_alias_label;
      const list_node<namespace_or_type_name_part_ast *> *name_part_sequence;
    };

  struct namespace_or_type_name_part_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_or_type_name_part
      };

      identifier_ast *identifier;
      type_arguments_ast *type_arguments;
    };

  struct namespace_or_type_name_safe_ast: public ast_node
    {
      enum
      {
        KIND = Kind_namespace_or_type_name_safe
      };

      identifier_ast *qualified_alias_label;
      const list_node<namespace_or_type_name_part_ast *> *name_part_sequence;
    };

  struct new_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_new_expression
      };

      array_creation_expression_rest_ast *array_creation_expression;
      object_or_delegate_creation_expression_rest_ast *object_or_delegate_creation_expression;
    };

  struct non_array_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_non_array_type
      };

      builtin_class_type_ast *builtin_class_type;
      optionally_nullable_type_ast *optionally_nullable_type;
    };

  struct non_nullable_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_non_nullable_type
      };

      type_name_ast *type_name;
      simple_type_ast *simple_type;
    };

  struct null_coalescing_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_null_coalescing_expression
      };

      const list_node<logical_or_expression_ast *> *expression_sequence;
    };

  struct numeric_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_numeric_type
      };

      numeric_type::numeric_type_enum type;
      integral_type_ast *int_type;
      floating_point_type_ast *float_type;
    };

  struct object_or_delegate_creation_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_object_or_delegate_creation_expression_rest
      };

      type_ast *type;
      optional_argument_list_ast *argument_list_or_expression;
    };

  struct optional_argument_list_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_argument_list
      };

      const list_node<argument_ast *> *argument_sequence;
    };

  struct optional_attribute_sections_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_attribute_sections
      };

      const list_node<attribute_section_ast *> *attribute_sequence;
    };

  struct optional_modifiers_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optional_modifiers
      };

      unsigned int modifiers;
    };

  struct optionally_nullable_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_optionally_nullable_type
      };

      bool nullable;
      non_nullable_type_ast *non_nullable_type;
    };

  struct overloadable_binary_only_operator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_overloadable_binary_only_operator
      };

      std::size_t op;
    };

  struct overloadable_unary_only_operator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_overloadable_unary_only_operator
      };

      std::size_t op;
    };

  struct overloadable_unary_or_binary_operator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_overloadable_unary_or_binary_operator
      };

      std::size_t op;
    };

  struct parameter_array_ast: public ast_node
    {
      enum
      {
        KIND = Kind_parameter_array
      };

      array_type_ast *type;
      identifier_ast *variable_name;
    };

  struct parameter_modifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_parameter_modifier
      };

      parameter_modifier::parameter_modifier_enum modifier;
    };

  struct pointer_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_pointer_type
      };

      pointer_type::pointer_type_enum type;
      int star_count;
      non_array_type_ast *regular_type;
      const list_node<unmanaged_type_suffix_ast *> *unmanaged_type_suffix_sequence;
    };

  struct positional_argument_ast: public ast_node
    {
      enum
      {
        KIND = Kind_positional_argument
      };

      expression_ast *attribute_argument_expression;
    };

  struct predefined_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_predefined_type
      };

      predefined_type::predefined_type_enum type;
    };

  struct primary_atom_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_atom
      };

      primary_atom::primary_atom_enum rule_type;
      literal_ast *literal;
      expression_ast *expression;
      simple_name_or_member_access_ast *simple_name_or_member_access;
      base_access_ast *base_access;
      new_expression_ast *new_expression;
      typeof_expression_ast *typeof_expression;
      type_ast *type;
      anonymous_method_expression_ast *anonymous_method_expression;
      unmanaged_type_ast *unmanaged_type;
    };

  struct primary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_expression
      };

      primary_atom_ast *primary_atom;
      const list_node<primary_suffix_ast *> *primary_suffix_sequence;
    };

  struct primary_or_secondary_constraint_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_or_secondary_constraint
      };

      primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type;
      class_type_ast *class_type_or_secondary_constraint;
    };

  struct primary_suffix_ast: public ast_node
    {
      enum
      {
        KIND = Kind_primary_suffix
      };

      primary_suffix::primary_suffix_enum suffix_type;
      identifier_ast *member_name;
      type_arguments_ast *type_arguments;
      optional_argument_list_ast *arguments;
      const list_node<expression_ast *> *expression_sequence;
    };

  struct property_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_property_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *type;
      type_name_safe_ast *property_name;
      accessor_declarations_ast *accessor_declarations;
    };

  struct qualified_identifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_qualified_identifier
      };

      const list_node<identifier_ast *> *name_sequence;
    };

  struct rank_specifier_ast: public ast_node
    {
      enum
      {
        KIND = Kind_rank_specifier
      };

      int dimension_seperator_count;
    };

  struct relational_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_relational_expression
      };

      shift_expression_ast *expression;
      const list_node<relational_expression_rest_ast *> *additional_expression_sequence;
    };

  struct relational_expression_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_relational_expression_rest
      };

      relational_expression_rest::relational_operator_enum relational_operator;
      shift_expression_ast *expression;
      type_ast *type;
    };

  struct resource_acquisition_ast: public ast_node
    {
      enum
      {
        KIND = Kind_resource_acquisition
      };

      local_variable_declaration_ast *local_variable_declaration;
      expression_ast *expression;
    };

  struct return_statement_ast: public ast_node
                 {
                   enum
                   {
                     KIND = Kind_return_statement
                   };

                   expression_ast *return_expression;
                 };

  struct return_type_ast: public ast_node
                 {
                   enum
                   {
                     KIND = Kind_return_type
                   };

                   return_type::return_type_enum type;
                   type_ast *regular_type;
                 };

  struct secondary_constraints_ast: public ast_node
    {
      enum
      {
        KIND = Kind_secondary_constraints
      };

      const list_node<type_name_ast *> *interface_type_or_type_parameter_sequence;
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

  struct simple_name_or_member_access_ast: public ast_node
    {
      enum
      {
        KIND = Kind_simple_name_or_member_access
      };

      identifier_ast *qualified_alias_label;
      identifier_ast *member_name;
      type_arguments_ast *type_arguments;
      predefined_type_ast *predefined_type;
    };

  struct simple_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_simple_type
      };

      simple_type::simple_type_enum type;
      numeric_type_ast *numeric_type;
    };

  struct specific_catch_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_specific_catch_clause
      };

      class_type_ast *exception_type;
      identifier_ast *exception_name;
      block_ast *body;
    };

  struct stackalloc_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_stackalloc_initializer
      };

      expression_ast *expression;
    };

  struct statement_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_statement_expression
      };

      expression_ast *expression;
    };

  struct struct_body_ast: public ast_node
    {
      enum
      {
        KIND = Kind_struct_body
      };

      const list_node<struct_member_declaration_ast *> *member_declaration_sequence;
    };

  struct struct_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_struct_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      bool partial;
      identifier_ast *struct_name;
      type_parameters_ast *type_parameters;
      struct_interfaces_ast *struct_interfaces;
      const list_node<type_parameter_constraints_clause_ast *> *type_parameter_constraints_sequence;
      struct_body_ast *body;
    };

  struct struct_interfaces_ast: public ast_node
    {
      enum
      {
        KIND = Kind_struct_interfaces
      };

      const list_node<type_name_ast *> *interface_type_sequence;
    };

  struct struct_member_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_struct_member_declaration
      };

      class_or_struct_member_declaration_ast *declaration;
    };

  struct switch_label_ast: public ast_node
    {
      enum
      {
        KIND = Kind_switch_label
      };

      switch_label::branch_type_enum branch_type;
      constant_expression_ast *case_expression;
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

  struct throw_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_throw_statement
      };

      expression_ast *exception;
    };

  struct try_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_try_statement
      };

      block_ast *try_body;
      catch_clauses_ast *catch_clauses;
      block_ast *finally_body;
    };

  struct type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type
      };

      unmanaged_type_ast *unmanaged_type;
    };

  struct type_arguments_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_arguments
      };

      const list_node<type_ast *> *type_argument_sequence;
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

      type_declaration_rest_ast *rest;
    };

  struct type_declaration_rest_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_declaration_rest
      };

      class_declaration_ast *class_declaration;
      struct_declaration_ast *struct_declaration;
      interface_declaration_ast *interface_declaration;
      enum_declaration_ast *enum_declaration;
      delegate_declaration_ast *delegate_declaration;
    };

  struct type_name_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_name
      };

    };

  struct type_name_safe_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_name_safe
      };

    };

  struct type_parameter_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameter
      };

      optional_attribute_sections_ast *attributes;
      identifier_ast *parameter_name;
    };

  struct type_parameter_constraints_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameter_constraints
      };

      primary_or_secondary_constraint_ast *primary_or_secondary_constraint;
      secondary_constraints_ast *secondary_constraints;
      constructor_constraint_ast *constructor_constraint;
    };

  struct type_parameter_constraints_clause_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameter_constraints_clause
      };

      identifier_ast *type_parameter;
      type_parameter_constraints_ast *constraints;
    };

  struct type_parameters_ast: public ast_node
    {
      enum
      {
        KIND = Kind_type_parameters
      };

      const list_node<type_parameter_ast *> *type_parameter_sequence;
    };

  struct typeof_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_typeof_expression
      };

      typeof_expression::typeof_expression_enum typeof_type;
      unbound_type_name_ast *unbound_type_name;
      type_ast *other_type;
    };

  struct unary_expression_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unary_expression
      };

      unary_expression::unary_expression_enum rule_type;
      unary_expression_ast *unary_expression;
      cast_expression_ast *cast_expression;
      primary_expression_ast *primary_expression;
    };

  struct unary_or_binary_operator_declaration_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unary_or_binary_operator_declaration
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *return_type;
      overloadable_operator::overloadable_operator_enum overloadable_operator_type;
      overloadable_operator::unary_or_binary_enum unary_or_binary;
      std::size_t overloadable_operator_token;
      type_ast *source1_type;
      identifier_ast *source1_name;
      type_ast *source2_type;
      identifier_ast *source2_name;
      block_ast *body;
    };

  struct unbound_type_name_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unbound_type_name
      };

      identifier_ast *qualified_alias_label;
      const list_node<unbound_type_name_part_ast *> *name_part_sequence;
    };

  struct unbound_type_name_part_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unbound_type_name_part
      };

      identifier_ast *identifier;
      generic_dimension_specifier_ast *generic_dimension_specifier;
    };

  struct unchecked_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unchecked_statement
      };

      block_ast *body;
    };

  struct unmanaged_type_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unmanaged_type
      };

      pointer_type::pointer_type_enum type;
      non_array_type_ast *regular_type;
      const list_node<unmanaged_type_suffix_ast *> *unmanaged_type_suffix_sequence;
    };

  struct unmanaged_type_suffix_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unmanaged_type_suffix
      };

      unmanaged_type_suffix::suffix_type type;
      rank_specifier_ast *rank_specifier;
    };

  struct unsafe_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_unsafe_statement
      };

      block_ast *body;
    };

  struct using_directive_ast: public ast_node
    {
      enum
      {
        KIND = Kind_using_directive
      };

      identifier_ast *alias;
      namespace_or_type_name_ast *namespace_or_type_name;
      namespace_or_type_name_ast *namespace_name;
    };

  struct using_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_using_statement
      };

      resource_acquisition_ast *resource_acquisition;
      embedded_statement_ast *body;
    };

  struct variable_declaration_data_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declaration_data
      };

      optional_attribute_sections_ast *attributes;
      optional_modifiers_ast *modifiers;
      type_ast *type;
      const list_node<variable_declarator_ast *> *variable_declarator_sequence;
    };

  struct variable_declarator_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_declarator
      };

      identifier_ast *variable_name;
      expression_ast *array_size;
      variable_initializer_ast *variable_initializer;
    };

  struct variable_initializer_ast: public ast_node
    {
      enum
      {
        KIND = Kind_variable_initializer
      };

      expression_ast *expression;
      array_initializer_ast *array_initializer;
      stackalloc_initializer_ast *stackalloc_initializer;
    };

  struct while_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_while_statement
      };

      boolean_expression_ast *condition;
      embedded_statement_ast *body;
    };

  struct yield_statement_ast: public ast_node
    {
      enum
      {
        KIND = Kind_yield_statement
      };

      yield_statement::yield_statement_enum yield_type;
      expression_ast *return_expression;
    };



} // end of namespace csharp


#include <string>
#include <set>

namespace csharp_pp
  {
  class handler_visitor;
}

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
      void tokenize();

      /**
       * The compatibility_mode status variable tells which version of C#
       * should be checked against.
       */
      enum csharp_compatibility_mode {
        csharp10_compatibility = 100,
        csharp20_compatibility = 200,
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
      bool is_nullable_type(type_ast *type);
      void unset_nullable_type(type_ast *type);
      type_ast *last_relational_expression_rest_type(
        null_coalescing_expression_ast *null_coalescing_expression);


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
      bool parse_optionally_nullable_type(optionally_nullable_type_ast **yynode);
      bool parse_overloadable_binary_only_operator(overloadable_binary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_overloadable_unary_only_operator(overloadable_unary_only_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast **yynode, overloadable_operator::overloadable_operator_enum* op);
      bool parse_parameter_array(parameter_array_ast **yynode);
      bool parse_parameter_modifier(parameter_modifier_ast **yynode);
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
      bool parse_secondary_constraints(secondary_constraints_ast **yynode);
      bool parse_shift_expression(shift_expression_ast **yynode);
      bool parse_shift_expression_rest(shift_expression_rest_ast **yynode);
      bool parse_simple_name_or_member_access(simple_name_or_member_access_ast **yynode);
      bool parse_simple_type(simple_type_ast **yynode);
      bool parse_specific_catch_clause(specific_catch_clause_ast **yynode);
      bool parse_stackalloc_initializer(stackalloc_initializer_ast **yynode);
      bool parse_statement_expression(statement_expression_ast **yynode);
      bool parse_struct_body(struct_body_ast **yynode);
      bool parse_struct_declaration(struct_declaration_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, bool partial);
      bool parse_struct_interfaces(struct_interfaces_ast **yynode);
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
      bool parse_using_directive(using_directive_ast **yynode);
      bool parse_using_statement(using_statement_ast **yynode);
      bool parse_variable_declaration_data(variable_declaration_data_ast **yynode, optional_attribute_sections_ast *attributes, optional_modifiers_ast *modifiers, type_ast *type, const list_node<variable_declarator_ast *> *variable_declarator_sequence);
      bool parse_variable_declarator(variable_declarator_ast **yynode, bool fixed_size_buffer);
      bool parse_variable_initializer(variable_initializer_ast **yynode);
      bool parse_while_statement(while_statement_ast **yynode);
      bool parse_yield_statement(yield_statement_ast **yynode);
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
      virtual void visit_accessor_declarations(accessor_declarations_ast *)
    {}
      virtual void visit_accessor_modifier(accessor_modifier_ast *)
      {}
      virtual void visit_additive_expression(additive_expression_ast *)
      {}
      virtual void visit_additive_expression_rest(additive_expression_rest_ast *)
      {}
      virtual void visit_anonymous_method_expression(anonymous_method_expression_ast *)
      {}
      virtual void visit_anonymous_method_parameter(anonymous_method_parameter_ast *)
      {}
      virtual void visit_anonymous_method_signature(anonymous_method_signature_ast *)
      {}
      virtual void visit_argument(argument_ast *)
      {}
      virtual void visit_array_creation_expression_rest(array_creation_expression_rest_ast *)
      {}
      virtual void visit_array_initializer(array_initializer_ast *)
      {}
      virtual void visit_array_type(array_type_ast *)
      {}
      virtual void visit_attribute(attribute_ast *)
      {}
      virtual void visit_attribute_arguments(attribute_arguments_ast *)
      {}
      virtual void visit_attribute_section(attribute_section_ast *)
      {}
      virtual void visit_attribute_target(attribute_target_ast *)
      {}
      virtual void visit_base_access(base_access_ast *)
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
      virtual void visit_boolean_expression(boolean_expression_ast *)
      {}
      virtual void visit_break_statement(break_statement_ast *)
      {}
      virtual void visit_builtin_class_type(builtin_class_type_ast *)
      {}
      virtual void visit_cast_expression(cast_expression_ast *)
      {}
      virtual void visit_catch_clauses(catch_clauses_ast *)
      {}
      virtual void visit_checked_statement(checked_statement_ast *)
      {}
      virtual void visit_class_base(class_base_ast *)
      {}
      virtual void visit_class_body(class_body_ast *)
      {}
      virtual void visit_class_declaration(class_declaration_ast *)
      {}
      virtual void visit_class_member_declaration(class_member_declaration_ast *)
      {}
      virtual void visit_class_or_struct_member_declaration(class_or_struct_member_declaration_ast *)
      {}
      virtual void visit_class_type(class_type_ast *)
      {}
      virtual void visit_compilation_unit(compilation_unit_ast *)
      {}
      virtual void visit_conditional_expression(conditional_expression_ast *)
      {}
      virtual void visit_constant_declaration(constant_declaration_ast *)
      {}
      virtual void visit_constant_declaration_data(constant_declaration_data_ast *)
      {}
      virtual void visit_constant_declarator(constant_declarator_ast *)
      {}
      virtual void visit_constant_expression(constant_expression_ast *)
      {}
      virtual void visit_constructor_constraint(constructor_constraint_ast *)
      {}
      virtual void visit_constructor_declaration(constructor_declaration_ast *)
      {}
      virtual void visit_constructor_initializer(constructor_initializer_ast *)
      {}
      virtual void visit_continue_statement(continue_statement_ast *)
      {}
      virtual void visit_conversion_operator_declaration(conversion_operator_declaration_ast *)
      {}
      virtual void visit_delegate_declaration(delegate_declaration_ast *)
      {}
      virtual void visit_do_while_statement(do_while_statement_ast *)
      {}
      virtual void visit_embedded_statement(embedded_statement_ast *)
      {}
      virtual void visit_enum_base(enum_base_ast *)
      {}
      virtual void visit_enum_body(enum_body_ast *)
      {}
      virtual void visit_enum_declaration(enum_declaration_ast *)
      {}
      virtual void visit_enum_member_declaration(enum_member_declaration_ast *)
      {}
      virtual void visit_equality_expression(equality_expression_ast *)
      {}
      virtual void visit_equality_expression_rest(equality_expression_rest_ast *)
      {}
      virtual void visit_event_accessor_declarations(event_accessor_declarations_ast *)
      {}
      virtual void visit_event_declaration(event_declaration_ast *)
      {}
      virtual void visit_expression(expression_ast *)
      {}
      virtual void visit_extern_alias_directive(extern_alias_directive_ast *)
      {}
      virtual void visit_finalizer_declaration(finalizer_declaration_ast *)
      {}
      virtual void visit_fixed_pointer_declarator(fixed_pointer_declarator_ast *)
      {}
      virtual void visit_fixed_statement(fixed_statement_ast *)
      {}
      virtual void visit_floating_point_type(floating_point_type_ast *)
      {}
      virtual void visit_for_control(for_control_ast *)
      {}
      virtual void visit_for_statement(for_statement_ast *)
      {}
      virtual void visit_foreach_statement(foreach_statement_ast *)
      {}
      virtual void visit_formal_parameter(formal_parameter_ast *)
      {}
      virtual void visit_formal_parameter_list(formal_parameter_list_ast *)
      {}
      virtual void visit_general_catch_clause(general_catch_clause_ast *)
      {}
      virtual void visit_generic_dimension_specifier(generic_dimension_specifier_ast *)
      {}
      virtual void visit_global_attribute_section(global_attribute_section_ast *)
      {}
      virtual void visit_goto_statement(goto_statement_ast *)
      {}
      virtual void visit_identifier(identifier_ast *)
      {}
      virtual void visit_if_statement(if_statement_ast *)
      {}
      virtual void visit_indexer_declaration(indexer_declaration_ast *)
      {}
      virtual void visit_integral_type(integral_type_ast *)
      {}
      virtual void visit_interface_accessors(interface_accessors_ast *)
      {}
      virtual void visit_interface_base(interface_base_ast *)
      {}
      virtual void visit_interface_body(interface_body_ast *)
      {}
      virtual void visit_interface_declaration(interface_declaration_ast *)
      {}
      virtual void visit_interface_event_declaration(interface_event_declaration_ast *)
      {}
      virtual void visit_interface_indexer_declaration(interface_indexer_declaration_ast *)
      {}
      virtual void visit_interface_member_declaration(interface_member_declaration_ast *)
      {}
      virtual void visit_interface_method_declaration(interface_method_declaration_ast *)
      {}
      virtual void visit_interface_property_declaration(interface_property_declaration_ast *)
      {}
      virtual void visit_keyword(keyword_ast *)
      {}
      virtual void visit_labeled_statement(labeled_statement_ast *)
      {}
      virtual void visit_literal(literal_ast *)
      {}
      virtual void visit_local_constant_declaration(local_constant_declaration_ast *)
      {}
      virtual void visit_local_variable_declaration(local_variable_declaration_ast *)
      {}
      virtual void visit_local_variable_declaration_statement(local_variable_declaration_statement_ast *)
      {}
      virtual void visit_lock_statement(lock_statement_ast *)
      {}
      virtual void visit_logical_and_expression(logical_and_expression_ast *)
      {}
      virtual void visit_logical_or_expression(logical_or_expression_ast *)
      {}
      virtual void visit_managed_type(managed_type_ast *)
      {}
      virtual void visit_method_declaration(method_declaration_ast *)
      {}
      virtual void visit_multiplicative_expression(multiplicative_expression_ast *)
      {}
      virtual void visit_multiplicative_expression_rest(multiplicative_expression_rest_ast *)
      {}
      virtual void visit_named_argument(named_argument_ast *)
      {}
      virtual void visit_namespace_body(namespace_body_ast *)
      {}
      virtual void visit_namespace_declaration(namespace_declaration_ast *)
      {}
      virtual void visit_namespace_member_declaration(namespace_member_declaration_ast *)
      {}
      virtual void visit_namespace_name(namespace_name_ast *)
      {}
      virtual void visit_namespace_or_type_name(namespace_or_type_name_ast *)
      {}
      virtual void visit_namespace_or_type_name_part(namespace_or_type_name_part_ast *)
      {}
      virtual void visit_namespace_or_type_name_safe(namespace_or_type_name_safe_ast *)
      {}
      virtual void visit_new_expression(new_expression_ast *)
      {}
      virtual void visit_non_array_type(non_array_type_ast *)
      {}
      virtual void visit_non_nullable_type(non_nullable_type_ast *)
      {}
      virtual void visit_null_coalescing_expression(null_coalescing_expression_ast *)
      {}
      virtual void visit_numeric_type(numeric_type_ast *)
      {}
      virtual void visit_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast *)
      {}
      virtual void visit_optional_argument_list(optional_argument_list_ast *)
      {}
      virtual void visit_optional_attribute_sections(optional_attribute_sections_ast *)
      {}
      virtual void visit_optional_modifiers(optional_modifiers_ast *)
      {}
      virtual void visit_optionally_nullable_type(optionally_nullable_type_ast *)
      {}
      virtual void visit_overloadable_binary_only_operator(overloadable_binary_only_operator_ast *)
      {}
      virtual void visit_overloadable_unary_only_operator(overloadable_unary_only_operator_ast *)
      {}
      virtual void visit_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast *)
      {}
      virtual void visit_parameter_array(parameter_array_ast *)
      {}
      virtual void visit_parameter_modifier(parameter_modifier_ast *)
      {}
      virtual void visit_pointer_type(pointer_type_ast *)
      {}
      virtual void visit_positional_argument(positional_argument_ast *)
      {}
      virtual void visit_predefined_type(predefined_type_ast *)
      {}
      virtual void visit_primary_atom(primary_atom_ast *)
      {}
      virtual void visit_primary_expression(primary_expression_ast *)
      {}
      virtual void visit_primary_or_secondary_constraint(primary_or_secondary_constraint_ast *)
      {}
      virtual void visit_primary_suffix(primary_suffix_ast *)
      {}
      virtual void visit_property_declaration(property_declaration_ast *)
      {}
      virtual void visit_qualified_identifier(qualified_identifier_ast *)
      {}
      virtual void visit_rank_specifier(rank_specifier_ast *)
      {}
      virtual void visit_relational_expression(relational_expression_ast *)
      {}
      virtual void visit_relational_expression_rest(relational_expression_rest_ast *)
      {}
      virtual void visit_resource_acquisition(resource_acquisition_ast *)
      {}
      virtual void visit_return_statement(return_statement_ast *)
                               {}
                               virtual void visit_return_type(return_type_ast *)
                                                        {}
                                                        virtual void visit_secondary_constraints(secondary_constraints_ast *)
                                                        {}
                                                        virtual void visit_shift_expression(shift_expression_ast *)
                                                        {}
                                                        virtual void visit_shift_expression_rest(shift_expression_rest_ast *)
                                                        {}
                                                        virtual void visit_simple_name_or_member_access(simple_name_or_member_access_ast *)
                                                        {}
                                                        virtual void visit_simple_type(simple_type_ast *)
                                                        {}
                                                        virtual void visit_specific_catch_clause(specific_catch_clause_ast *)
                                                        {}
                                                        virtual void visit_stackalloc_initializer(stackalloc_initializer_ast *)
                                                        {}
                                                        virtual void visit_statement_expression(statement_expression_ast *)
                                                        {}
                                                        virtual void visit_struct_body(struct_body_ast *)
                                                        {}
                                                        virtual void visit_struct_declaration(struct_declaration_ast *)
                                                        {}
                                                        virtual void visit_struct_interfaces(struct_interfaces_ast *)
                                                        {}
                                                        virtual void visit_struct_member_declaration(struct_member_declaration_ast *)
                                                        {}
                                                        virtual void visit_switch_label(switch_label_ast *)
                                                        {}
                                                        virtual void visit_switch_section(switch_section_ast *)
                                                        {}
                                                        virtual void visit_switch_statement(switch_statement_ast *)
                                                        {}
                                                        virtual void visit_throw_statement(throw_statement_ast *)
                                                        {}
                                                        virtual void visit_try_statement(try_statement_ast *)
                                                        {}
                                                        virtual void visit_type(type_ast *)
                                                        {}
                                                        virtual void visit_type_arguments(type_arguments_ast *)
                                                        {}
                                                        virtual void visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *)
                                                        {}
                                                        virtual void visit_type_declaration(type_declaration_ast *)
                                                        {}
                                                        virtual void visit_type_declaration_rest(type_declaration_rest_ast *)
                                                        {}
                                                        virtual void visit_type_name(type_name_ast *)
                                                        {}
                                                        virtual void visit_type_name_safe(type_name_safe_ast *)
                                                        {}
                                                        virtual void visit_type_parameter(type_parameter_ast *)
                                                        {}
                                                        virtual void visit_type_parameter_constraints(type_parameter_constraints_ast *)
                                                        {}
                                                        virtual void visit_type_parameter_constraints_clause(type_parameter_constraints_clause_ast *)
                                                        {}
                                                        virtual void visit_type_parameters(type_parameters_ast *)
                                                        {}
                                                        virtual void visit_typeof_expression(typeof_expression_ast *)
                                                        {}
                                                        virtual void visit_unary_expression(unary_expression_ast *)
                                                        {}
                                                        virtual void visit_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast *)
                                                        {}
                                                        virtual void visit_unbound_type_name(unbound_type_name_ast *)
                                                        {}
                                                        virtual void visit_unbound_type_name_part(unbound_type_name_part_ast *)
                                                        {}
                                                        virtual void visit_unchecked_statement(unchecked_statement_ast *)
                                                        {}
                                                        virtual void visit_unmanaged_type(unmanaged_type_ast *)
                                                        {}
                                                        virtual void visit_unmanaged_type_suffix(unmanaged_type_suffix_ast *)
                                                        {}
                                                        virtual void visit_unsafe_statement(unsafe_statement_ast *)
                                                        {}
                                                        virtual void visit_using_directive(using_directive_ast *)
                                                        {}
                                                        virtual void visit_using_statement(using_statement_ast *)
                                                        {}
                                                        virtual void visit_variable_declaration_data(variable_declaration_data_ast *)
                                                        {}
                                                        virtual void visit_variable_declarator(variable_declarator_ast *)
                                                        {}
                                                        virtual void visit_variable_initializer(variable_initializer_ast *)
                                                        {}
                                                        virtual void visit_while_statement(while_statement_ast *)
                                                        {}
                                                        virtual void visit_yield_statement(yield_statement_ast *)
                                                        {}
                                                      }
                                                    ;
  class default_visitor: public visitor
    {
    public:
      virtual void visit_accessor_declarations(accessor_declarations_ast *node)
      {
        visit_node(node->accessor1_attributes);
        visit_node(node->accessor1_modifier);
        visit_node(node->accessor1_body);
        visit_node(node->accessor2_attributes);
        visit_node(node->accessor2_modifier);
        visit_node(node->accessor2_body);
      }

      virtual void visit_accessor_modifier(accessor_modifier_ast *)
      {}

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

      virtual void visit_anonymous_method_expression(anonymous_method_expression_ast *node)
      {
        visit_node(node->anonymous_method_signature);
        visit_node(node->body);
      }

      virtual void visit_anonymous_method_parameter(anonymous_method_parameter_ast *node)
      {
        visit_node(node->modifier);
        visit_node(node->type);
        visit_node(node->variable_name);
      }

      virtual void visit_anonymous_method_signature(anonymous_method_signature_ast *node)
      {
        if (node->anonymous_method_parameter_sequence)
          {
            const list_node<anonymous_method_parameter_ast*> *__it = node->anonymous_method_parameter_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_argument(argument_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_array_creation_expression_rest(array_creation_expression_rest_ast *node)
      {
        visit_node(node->type);
        visit_node(node->array_initializer);
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
        if (node->rank_specifier_sequence)
          {
            const list_node<rank_specifier_ast*> *__it = node->rank_specifier_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_array_initializer(array_initializer_ast *node)
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

      virtual void visit_array_type(array_type_ast *node)
      {
        visit_node(node->non_array_type);
        if (node->rank_specifier_sequence)
          {
            const list_node<rank_specifier_ast*> *__it = node->rank_specifier_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_attribute(attribute_ast *node)
      {
        visit_node(node->name);
        visit_node(node->arguments);
      }

      virtual void visit_attribute_arguments(attribute_arguments_ast *node)
      {
        if (node->named_argument_sequence)
          {
            const list_node<named_argument_ast*> *__it = node->named_argument_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->positional_argument_sequence)
          {
            const list_node<positional_argument_ast*> *__it = node->positional_argument_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_attribute_section(attribute_section_ast *node)
      {
        visit_node(node->target);
        if (node->attribute_sequence)
          {
            const list_node<attribute_ast*> *__it = node->attribute_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_attribute_target(attribute_target_ast *node)
      {
        visit_node(node->identifier);
        visit_node(node->keyword);
      }

      virtual void visit_base_access(base_access_ast *node)
      {
        visit_node(node->identifier);
        visit_node(node->type_arguments);
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
        visit_node(node->labeled_statement);
        visit_node(node->local_constant_declaration_statement);
        visit_node(node->local_variable_declaration_statement);
        visit_node(node->statement);
      }

      virtual void visit_boolean_expression(boolean_expression_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_break_statement(break_statement_ast *)
      {}

      virtual void visit_builtin_class_type(builtin_class_type_ast *)
      {}

      virtual void visit_cast_expression(cast_expression_ast *node)
      {
        visit_node(node->type);
        visit_node(node->casted_expression);
      }

      virtual void visit_catch_clauses(catch_clauses_ast *node)
      {
        visit_node(node->general_catch_clause);
        if (node->specific_catch_clause_sequence)
          {
            const list_node<specific_catch_clause_ast*> *__it = node->specific_catch_clause_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_checked_statement(checked_statement_ast *node)
      {
        visit_node(node->body);
      }

      virtual void visit_class_base(class_base_ast *node)
      {
        visit_node(node->builtin_class_type);
        if (node->interface_type_sequence)
          {
            const list_node<type_name_ast*> *__it = node->interface_type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->base_type_sequence)
          {
            const list_node<type_name_ast*> *__it = node->base_type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_class_body(class_body_ast *node)
      {
        if (node->member_declaration_sequence)
          {
            const list_node<class_member_declaration_ast*> *__it = node->member_declaration_sequence->to_front(), *__end = __it;
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
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->class_name);
        visit_node(node->type_parameters);
        visit_node(node->class_base);
        if (node->type_parameter_constraints_sequence)
          {
            const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->body);
      }

      virtual void visit_class_member_declaration(class_member_declaration_ast *node)
      {
        visit_node(node->finalizer_declaration);
        visit_node(node->other_declaration);
      }

      virtual void visit_class_or_struct_member_declaration(class_or_struct_member_declaration_ast *node)
      {
        visit_node(node->constant_declaration);
        visit_node(node->event_declaration);
        visit_node(node->conversion_operator_declaration);
        visit_node(node->constructor_declaration);
        visit_node(node->type_declaration_rest);
        visit_node(node->unary_or_binary_operator_declaration);
        visit_node(node->indexer_declaration);
        visit_node(node->field_declaration);
        visit_node(node->property_declaration);
        visit_node(node->method_declaration);
      }

      virtual void visit_class_type(class_type_ast *node)
      {
        visit_node(node->type_name);
        visit_node(node->builtin_class_type);
      }

      virtual void visit_compilation_unit(compilation_unit_ast *node)
      {
        if (node->extern_alias_sequence)
          {
            const list_node<extern_alias_directive_ast*> *__it = node->extern_alias_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->using_sequence)
          {
            const list_node<using_directive_ast*> *__it = node->using_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->global_attribute_sequence)
          {
            const list_node<global_attribute_section_ast*> *__it = node->global_attribute_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->namespace_sequence)
          {
            const list_node<namespace_member_declaration_ast*> *__it = node->namespace_sequence->to_front(), *__end = __it;
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
        visit_node(node->null_coalescing_expression);
        visit_node(node->if_expression);
        visit_node(node->else_expression);
      }

      virtual void visit_constant_declaration(constant_declaration_ast *node)
      {
        visit_node(node->data);
      }

      virtual void visit_constant_declaration_data(constant_declaration_data_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->type);
        if (node->constant_declarator_sequence)
          {
            const list_node<constant_declarator_ast*> *__it = node->constant_declarator_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_constant_declarator(constant_declarator_ast *node)
      {
        visit_node(node->constant_name);
        visit_node(node->expression);
      }

      virtual void visit_constant_expression(constant_expression_ast *node)
      {
        visit_node(node->expression);
      }

      virtual void visit_constructor_constraint(constructor_constraint_ast *)
      {}

      virtual void visit_constructor_declaration(constructor_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->class_name);
        visit_node(node->formal_parameters);
        visit_node(node->constructor_initializer);
        visit_node(node->body);
      }

      virtual void visit_constructor_initializer(constructor_initializer_ast *node)
      {
        visit_node(node->arguments);
      }

      virtual void visit_continue_statement(continue_statement_ast *)
      {}

      virtual void visit_conversion_operator_declaration(conversion_operator_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->target_type);
        visit_node(node->source_type);
        visit_node(node->source_name);
        visit_node(node->body);
      }

      virtual void visit_delegate_declaration(delegate_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->return_type);
        visit_node(node->delegate_name);
        visit_node(node->type_parameters);
        visit_node(node->formal_parameters);
        if (node->type_parameter_constraints_sequence)
          {
            const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_do_while_statement(do_while_statement_ast *node)
      {
        visit_node(node->body);
        visit_node(node->condition);
      }

      virtual void visit_embedded_statement(embedded_statement_ast *node)
      {
        visit_node(node->block);
        visit_node(node->if_statement);
        visit_node(node->switch_statement);
        visit_node(node->while_statement);
        visit_node(node->do_while_statement);
        visit_node(node->for_statement);
        visit_node(node->foreach_statement);
        visit_node(node->break_statement);
        visit_node(node->continue_statement);
        visit_node(node->goto_statement);
        visit_node(node->return_statement);
        visit_node(node->throw_statement);
        visit_node(node->try_statement);
        visit_node(node->lock_statement);
        visit_node(node->using_statement);
        visit_node(node->checked_statement);
        visit_node(node->unchecked_statement);
        visit_node(node->yield_statement);
        visit_node(node->expression_statement);
        visit_node(node->unsafe_statement);
        visit_node(node->fixed_statement);
      }

      virtual void visit_enum_base(enum_base_ast *node)
      {
        visit_node(node->integral_type);
      }

      virtual void visit_enum_body(enum_body_ast *node)
      {
        if (node->member_declaration_sequence)
          {
            const list_node<enum_member_declaration_ast*> *__it = node->member_declaration_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_enum_declaration(enum_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->enum_name);
        visit_node(node->enum_base);
        visit_node(node->body);
      }

      virtual void visit_enum_member_declaration(enum_member_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->member_name);
        visit_node(node->constant_expression);
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

      virtual void visit_event_accessor_declarations(event_accessor_declarations_ast *node)
      {
        visit_node(node->accessor1_attributes);
        visit_node(node->accessor1_body);
        visit_node(node->accessor2_attributes);
        visit_node(node->accessor2_body);
      }

      virtual void visit_event_declaration(event_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->type);
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
        visit_node(node->event_name);
        visit_node(node->event_accessor_declarations);
      }

      virtual void visit_expression(expression_ast *node)
      {
        visit_node(node->conditional_expression);
        visit_node(node->assignment_expression);
      }

      virtual void visit_extern_alias_directive(extern_alias_directive_ast *node)
      {
        visit_node(node->identifier);
      }

      virtual void visit_finalizer_declaration(finalizer_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->class_name);
        visit_node(node->finalizer_body);
      }

      virtual void visit_fixed_pointer_declarator(fixed_pointer_declarator_ast *node)
      {
        visit_node(node->pointer_name);
        visit_node(node->initializer);
      }

      virtual void visit_fixed_statement(fixed_statement_ast *node)
      {
        visit_node(node->pointer_type);
        visit_node(node->fixed_pointer_declarator);
        visit_node(node->body);
      }

      virtual void visit_floating_point_type(floating_point_type_ast *)
      {}

      virtual void visit_for_control(for_control_ast *node)
      {
        visit_node(node->local_variable_declaration);
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
        visit_node(node->for_condition);
        if (node->for_iterator_sequence)
          {
            const list_node<statement_expression_ast*> *__it = node->for_iterator_sequence->to_front(), *__end = __it;
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

      virtual void visit_foreach_statement(foreach_statement_ast *node)
      {
        visit_node(node->variable_type);
        visit_node(node->variable_name);
        visit_node(node->collection);
        visit_node(node->body);
      }

      virtual void visit_formal_parameter(formal_parameter_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->parameter_array);
        visit_node(node->modifier);
        visit_node(node->type);
        visit_node(node->variable_name);
      }

      virtual void visit_formal_parameter_list(formal_parameter_list_ast *node)
      {
        if (node->formal_parameter_sequence)
          {
            const list_node<formal_parameter_ast*> *__it = node->formal_parameter_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_general_catch_clause(general_catch_clause_ast *node)
      {
        visit_node(node->body);
      }

      virtual void visit_generic_dimension_specifier(generic_dimension_specifier_ast *)
      {}

      virtual void visit_global_attribute_section(global_attribute_section_ast *node)
      {
        if (node->attribute_sequence)
          {
            const list_node<attribute_ast*> *__it = node->attribute_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_goto_statement(goto_statement_ast *node)
      {
        visit_node(node->label);
        visit_node(node->constant_expression);
      }

      virtual void visit_identifier(identifier_ast *)
      {}

      virtual void visit_if_statement(if_statement_ast *node)
      {
        visit_node(node->condition);
        visit_node(node->if_body);
        visit_node(node->else_body);
      }

      virtual void visit_indexer_declaration(indexer_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->type);
        visit_node(node->interface_type);
        visit_node(node->formal_parameters);
        visit_node(node->accessor_declarations);
      }

      virtual void visit_integral_type(integral_type_ast *)
      {}

      virtual void visit_interface_accessors(interface_accessors_ast *node)
      {
        visit_node(node->accessor1_attributes);
        visit_node(node->accessor2_attributes);
      }

      virtual void visit_interface_base(interface_base_ast *node)
      {
        if (node->interface_type_sequence)
          {
            const list_node<type_name_ast*> *__it = node->interface_type_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_interface_body(interface_body_ast *node)
      {
        if (node->member_declaration_sequence)
          {
            const list_node<interface_member_declaration_ast*> *__it = node->member_declaration_sequence->to_front(), *__end = __it;
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
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->interface_name);
        visit_node(node->type_parameters);
        visit_node(node->interface_base);
        if (node->type_parameter_constraints_sequence)
          {
            const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->body);
      }

      virtual void visit_interface_event_declaration(interface_event_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->event_type);
        visit_node(node->event_name);
      }

      virtual void visit_interface_indexer_declaration(interface_indexer_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->type);
        visit_node(node->formal_parameters);
        visit_node(node->interface_accessors);
      }

      virtual void visit_interface_member_declaration(interface_member_declaration_ast *node)
      {
        visit_node(node->event_declaration);
        visit_node(node->indexer_declaration);
        visit_node(node->interface_property_declaration);
        visit_node(node->interface_method_declaration);
      }

      virtual void visit_interface_method_declaration(interface_method_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->return_type);
        visit_node(node->method_name);
        visit_node(node->type_parameters);
        visit_node(node->formal_parameters);
        if (node->type_parameter_constraints_sequence)
          {
            const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_interface_property_declaration(interface_property_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->type);
        visit_node(node->property_name);
        visit_node(node->interface_accessors);
      }

      virtual void visit_keyword(keyword_ast *)
      {}

      virtual void visit_labeled_statement(labeled_statement_ast *node)
      {
        visit_node(node->label);
      }

      virtual void visit_literal(literal_ast *)
      {}

      virtual void visit_local_constant_declaration(local_constant_declaration_ast *node)
      {
        visit_node(node->data);
      }

      virtual void visit_local_variable_declaration(local_variable_declaration_ast *node)
      {
        visit_node(node->data);
      }

      virtual void visit_local_variable_declaration_statement(local_variable_declaration_statement_ast *node)
      {
        visit_node(node->declaration);
      }

      virtual void visit_lock_statement(lock_statement_ast *node)
      {
        visit_node(node->lock_expression);
        visit_node(node->body);
      }

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

      virtual void visit_managed_type(managed_type_ast *node)
      {
        visit_node(node->non_array_type);
        if (node->rank_specifier_sequence)
          {
            const list_node<rank_specifier_ast*> *__it = node->rank_specifier_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_method_declaration(method_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->return_type);
        visit_node(node->method_name);
        visit_node(node->type_parameters);
        visit_node(node->formal_parameters);
        if (node->type_parameter_constraints_sequence)
          {
            const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        visit_node(node->method_body);
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

      virtual void visit_named_argument(named_argument_ast *node)
      {
        visit_node(node->argument_name);
        visit_node(node->attribute_argument_expression);
      }

      virtual void visit_namespace_body(namespace_body_ast *node)
      {
        if (node->extern_alias_sequence)
          {
            const list_node<extern_alias_directive_ast*> *__it = node->extern_alias_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->using_sequence)
          {
            const list_node<using_directive_ast*> *__it = node->using_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
        if (node->namespace_sequence)
          {
            const list_node<namespace_member_declaration_ast*> *__it = node->namespace_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_namespace_declaration(namespace_declaration_ast *node)
      {
        visit_node(node->name);
        visit_node(node->body);
      }

      virtual void visit_namespace_member_declaration(namespace_member_declaration_ast *node)
      {
        visit_node(node->namespace_declaration);
        visit_node(node->type_declaration);
      }

      virtual void visit_namespace_name(namespace_name_ast *)
      {}

      virtual void visit_namespace_or_type_name(namespace_or_type_name_ast *node)
      {
        visit_node(node->qualified_alias_label);
        if (node->name_part_sequence)
          {
            const list_node<namespace_or_type_name_part_ast*> *__it = node->name_part_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_namespace_or_type_name_part(namespace_or_type_name_part_ast *node)
      {
        visit_node(node->identifier);
        visit_node(node->type_arguments);
      }

      virtual void visit_namespace_or_type_name_safe(namespace_or_type_name_safe_ast *node)
      {
        visit_node(node->qualified_alias_label);
        if (node->name_part_sequence)
          {
            const list_node<namespace_or_type_name_part_ast*> *__it = node->name_part_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_new_expression(new_expression_ast *node)
      {
        visit_node(node->array_creation_expression);
        visit_node(node->object_or_delegate_creation_expression);
      }

      virtual void visit_non_array_type(non_array_type_ast *node)
      {
        visit_node(node->builtin_class_type);
        visit_node(node->optionally_nullable_type);
      }

      virtual void visit_non_nullable_type(non_nullable_type_ast *node)
      {
        visit_node(node->type_name);
        visit_node(node->simple_type);
      }

      virtual void visit_null_coalescing_expression(null_coalescing_expression_ast *node)
      {
        if (node->expression_sequence)
          {
            const list_node<logical_or_expression_ast*> *__it = node->expression_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_numeric_type(numeric_type_ast *node)
      {
        visit_node(node->int_type);
        visit_node(node->float_type);
      }

      virtual void visit_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast *node)
      {
        visit_node(node->type);
        visit_node(node->argument_list_or_expression);
      }

      virtual void visit_optional_argument_list(optional_argument_list_ast *node)
      {
        if (node->argument_sequence)
          {
            const list_node<argument_ast*> *__it = node->argument_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_optional_attribute_sections(optional_attribute_sections_ast *node)
      {
        if (node->attribute_sequence)
          {
            const list_node<attribute_section_ast*> *__it = node->attribute_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_optional_modifiers(optional_modifiers_ast *)
    {}

      virtual void visit_optionally_nullable_type(optionally_nullable_type_ast *node)
      {
        visit_node(node->non_nullable_type);
      }

      virtual void visit_overloadable_binary_only_operator(overloadable_binary_only_operator_ast *)
      {}

      virtual void visit_overloadable_unary_only_operator(overloadable_unary_only_operator_ast *)
      {}

      virtual void visit_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast *)
      {}

      virtual void visit_parameter_array(parameter_array_ast *node)
      {
        visit_node(node->type);
        visit_node(node->variable_name);
      }

      virtual void visit_parameter_modifier(parameter_modifier_ast *)
      {}

      virtual void visit_pointer_type(pointer_type_ast *node)
      {
        visit_node(node->regular_type);
        if (node->unmanaged_type_suffix_sequence)
          {
            const list_node<unmanaged_type_suffix_ast*> *__it = node->unmanaged_type_suffix_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_positional_argument(positional_argument_ast *node)
      {
        visit_node(node->attribute_argument_expression);
      }

      virtual void visit_predefined_type(predefined_type_ast *)
      {}

      virtual void visit_primary_atom(primary_atom_ast *node)
      {
        visit_node(node->literal);
        visit_node(node->expression);
        visit_node(node->simple_name_or_member_access);
        visit_node(node->base_access);
        visit_node(node->new_expression);
        visit_node(node->typeof_expression);
        visit_node(node->type);
        visit_node(node->anonymous_method_expression);
        visit_node(node->unmanaged_type);
      }

      virtual void visit_primary_expression(primary_expression_ast *node)
      {
        visit_node(node->primary_atom);
        if (node->primary_suffix_sequence)
          {
            const list_node<primary_suffix_ast*> *__it = node->primary_suffix_sequence->to_front(), *__end = __it;
            do
              {
                visit_node(__it->element);
                __it = __it->next;
              }
            while (__it != __end);
          }
      }

      virtual void visit_primary_or_secondary_constraint(primary_or_secondary_constraint_ast *node)
      {
        visit_node(node->class_type_or_secondary_constraint);
      }

      virtual void visit_primary_suffix(primary_suffix_ast *node)
      {
        visit_node(node->member_name);
        visit_node(node->type_arguments);
        visit_node(node->arguments);
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

      virtual void visit_property_declaration(property_declaration_ast *node)
      {
        visit_node(node->attributes);
        visit_node(node->modifiers);
        visit_node(node->type);
        visit_node(node->property_name);
        visit_node(node->accessor_declarations);
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

      virtual void visit_rank_specifier(rank_specifier_ast *)
    {}

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
      }

      virtual void visit_relational_expression_rest(relational_expression_rest_ast *node)
      {
        visit_node(node->expression);
        visit_node(node->type);
      }

      virtual void visit_resource_acquisition(resource_acquisition_ast *node)
      {
        visit_node(node->local_variable_declaration);
        visit_node(node->expression);
      }

      virtual void visit_return_statement(return_statement_ast *node)
                               {
                                 visit_node(node->return_expression);
                               }

                               virtual void visit_return_type(return_type_ast *node)
                                                        {
                                                          visit_node(node->regular_type);
                                                        }

                                                        virtual void visit_secondary_constraints(secondary_constraints_ast *node)
                                                        {
                                                          if (node->interface_type_or_type_parameter_sequence)
                                                            {
                                                              const list_node<type_name_ast*> *__it = node->interface_type_or_type_parameter_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
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

                                                        virtual void visit_simple_name_or_member_access(simple_name_or_member_access_ast *node)
                                                        {
                                                          visit_node(node->qualified_alias_label);
                                                          visit_node(node->member_name);
                                                          visit_node(node->type_arguments);
                                                          visit_node(node->predefined_type);
                                                        }

                                                        virtual void visit_simple_type(simple_type_ast *node)
                                                        {
                                                          visit_node(node->numeric_type);
                                                        }

                                                        virtual void visit_specific_catch_clause(specific_catch_clause_ast *node)
                                                        {
                                                          visit_node(node->exception_type);
                                                          visit_node(node->exception_name);
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_stackalloc_initializer(stackalloc_initializer_ast *node)
                                                        {
                                                          visit_node(node->expression);
                                                        }

                                                        virtual void visit_statement_expression(statement_expression_ast *node)
                                                        {
                                                          visit_node(node->expression);
                                                        }

                                                        virtual void visit_struct_body(struct_body_ast *node)
                                                        {
                                                          if (node->member_declaration_sequence)
                                                            {
                                                              const list_node<struct_member_declaration_ast*> *__it = node->member_declaration_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
                                                        }

                                                        virtual void visit_struct_declaration(struct_declaration_ast *node)
                                                        {
                                                          visit_node(node->attributes);
                                                          visit_node(node->modifiers);
                                                          visit_node(node->struct_name);
                                                          visit_node(node->type_parameters);
                                                          visit_node(node->struct_interfaces);
                                                          if (node->type_parameter_constraints_sequence)
                                                            {
                                                              const list_node<type_parameter_constraints_clause_ast*> *__it = node->type_parameter_constraints_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_struct_interfaces(struct_interfaces_ast *node)
                                                        {
                                                          if (node->interface_type_sequence)
                                                            {
                                                              const list_node<type_name_ast*> *__it = node->interface_type_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
                                                        }

                                                        virtual void visit_struct_member_declaration(struct_member_declaration_ast *node)
                                                        {
                                                          visit_node(node->declaration);
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

                                                        virtual void visit_throw_statement(throw_statement_ast *node)
                                                        {
                                                          visit_node(node->exception);
                                                        }

                                                        virtual void visit_try_statement(try_statement_ast *node)
                                                        {
                                                          visit_node(node->try_body);
                                                          visit_node(node->catch_clauses);
                                                          visit_node(node->finally_body);
                                                        }

                                                        virtual void visit_type(type_ast *node)
                                                        {
                                                          visit_node(node->unmanaged_type);
                                                        }

                                                        virtual void visit_type_arguments(type_arguments_ast *node)
                                                        {
                                                          if (node->type_argument_sequence)
                                                            {
                                                              const list_node<type_ast*> *__it = node->type_argument_sequence->to_front(), *__end = __it;
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
                                                          visit_node(node->rest);
                                                        }

                                                        virtual void visit_type_declaration_rest(type_declaration_rest_ast *node)
                                                        {
                                                          visit_node(node->class_declaration);
                                                          visit_node(node->struct_declaration);
                                                          visit_node(node->interface_declaration);
                                                          visit_node(node->enum_declaration);
                                                          visit_node(node->delegate_declaration);
                                                        }

                                                        virtual void visit_type_name(type_name_ast *)
                                                        {}

                                                        virtual void visit_type_name_safe(type_name_safe_ast *)
                                                        {}

                                                        virtual void visit_type_parameter(type_parameter_ast *node)
                                                        {
                                                          visit_node(node->attributes);
                                                          visit_node(node->parameter_name);
                                                        }

                                                        virtual void visit_type_parameter_constraints(type_parameter_constraints_ast *node)
                                                        {
                                                          visit_node(node->primary_or_secondary_constraint);
                                                          visit_node(node->secondary_constraints);
                                                          visit_node(node->constructor_constraint);
                                                        }

                                                        virtual void visit_type_parameter_constraints_clause(type_parameter_constraints_clause_ast *node)
                                                        {
                                                          visit_node(node->type_parameter);
                                                          visit_node(node->constraints);
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

                                                        virtual void visit_typeof_expression(typeof_expression_ast *node)
                                                        {
                                                          visit_node(node->unbound_type_name);
                                                          visit_node(node->other_type);
                                                        }

                                                        virtual void visit_unary_expression(unary_expression_ast *node)
                                                        {
                                                          visit_node(node->unary_expression);
                                                          visit_node(node->cast_expression);
                                                          visit_node(node->primary_expression);
                                                        }

                                                        virtual void visit_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast *node)
                                                        {
                                                          visit_node(node->attributes);
                                                          visit_node(node->modifiers);
                                                          visit_node(node->return_type);
                                                          visit_node(node->source1_type);
                                                          visit_node(node->source1_name);
                                                          visit_node(node->source2_type);
                                                          visit_node(node->source2_name);
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_unbound_type_name(unbound_type_name_ast *node)
                                                        {
                                                          visit_node(node->qualified_alias_label);
                                                          if (node->name_part_sequence)
                                                            {
                                                              const list_node<unbound_type_name_part_ast*> *__it = node->name_part_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
                                                        }

                                                        virtual void visit_unbound_type_name_part(unbound_type_name_part_ast *node)
                                                        {
                                                          visit_node(node->identifier);
                                                          visit_node(node->generic_dimension_specifier);
                                                        }

                                                        virtual void visit_unchecked_statement(unchecked_statement_ast *node)
                                                        {
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_unmanaged_type(unmanaged_type_ast *node)
                                                        {
                                                          visit_node(node->regular_type);
                                                          if (node->unmanaged_type_suffix_sequence)
                                                            {
                                                              const list_node<unmanaged_type_suffix_ast*> *__it = node->unmanaged_type_suffix_sequence->to_front(), *__end = __it;
                                                              do
                                                                {
                                                                  visit_node(__it->element);
                                                                  __it = __it->next;
                                                                }
                                                              while (__it != __end);
                                                            }
                                                        }

                                                        virtual void visit_unmanaged_type_suffix(unmanaged_type_suffix_ast *node)
                                                        {
                                                          visit_node(node->rank_specifier);
                                                        }

                                                        virtual void visit_unsafe_statement(unsafe_statement_ast *node)
                                                        {
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_using_directive(using_directive_ast *node)
                                                        {
                                                          visit_node(node->alias);
                                                          visit_node(node->namespace_or_type_name);
                                                          visit_node(node->namespace_name);
                                                        }

                                                        virtual void visit_using_statement(using_statement_ast *node)
                                                        {
                                                          visit_node(node->resource_acquisition);
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_variable_declaration_data(variable_declaration_data_ast *node)
                                                        {
                                                          visit_node(node->attributes);
                                                          visit_node(node->modifiers);
                                                          visit_node(node->type);
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

                                                        virtual void visit_variable_declarator(variable_declarator_ast *node)
                                                        {
                                                          visit_node(node->variable_name);
                                                          visit_node(node->array_size);
                                                          visit_node(node->variable_initializer);
                                                        }

                                                        virtual void visit_variable_initializer(variable_initializer_ast *node)
                                                        {
                                                          visit_node(node->expression);
                                                          visit_node(node->array_initializer);
                                                          visit_node(node->stackalloc_initializer);
                                                        }

                                                        virtual void visit_while_statement(while_statement_ast *node)
                                                        {
                                                          visit_node(node->condition);
                                                          visit_node(node->body);
                                                        }

                                                        virtual void visit_yield_statement(yield_statement_ast *node)
                                                        {
                                                          visit_node(node->return_expression);
                                                        }

                                                      };

} // end of namespace csharp

#endif


