// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef java_VISITOR_H_INCLUDED
#define java_VISITOR_H_INCLUDED

#include "java_ast.h"

namespace java
  {

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
                               virtual void visit_variable_declaration_statement(variable_declaration_statement_ast *)
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

} // end of namespace java

#endif


