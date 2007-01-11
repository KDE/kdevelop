// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "java_default_visitor.h"

namespace java
  {

  void default_visitor::visit_additive_expression(additive_expression_ast *node)
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

  void default_visitor::visit_additive_expression_rest(additive_expression_rest_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_annotation(annotation_ast *node)
  {
    visit_node(node->type_name);
    visit_node(node->args);
  }

  void default_visitor::visit_annotation_arguments(annotation_arguments_ast *node)
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

  void default_visitor::visit_annotation_element_array_initializer(annotation_element_array_initializer_ast *node)
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

  void default_visitor::visit_annotation_element_array_value(annotation_element_array_value_ast *node)
  {
    visit_node(node->cond_expression);
    visit_node(node->annotation);
  }

  void default_visitor::visit_annotation_element_value(annotation_element_value_ast *node)
  {
    visit_node(node->cond_expression);
    visit_node(node->annotation);
    visit_node(node->element_array_initializer);
  }

  void default_visitor::visit_annotation_element_value_pair(annotation_element_value_pair_ast *node)
  {
    visit_node(node->element_name);
    visit_node(node->element_value);
  }

  void default_visitor::visit_annotation_method_declaration(annotation_method_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->return_type);
    visit_node(node->annotation_name);
    visit_node(node->annotation_element_value);
  }

  void default_visitor::visit_annotation_type_body(annotation_type_body_ast *node)
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

  void default_visitor::visit_annotation_type_declaration(annotation_type_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->annotation_type_name);
    visit_node(node->body);
  }

  void default_visitor::visit_annotation_type_field(annotation_type_field_ast *node)
  {
    visit_node(node->class_declaration);
    visit_node(node->enum_declaration);
    visit_node(node->interface_declaration);
    visit_node(node->annotation_type_declaration);
    visit_node(node->method_declaration);
    visit_node(node->constant_declaration);
  }

  void default_visitor::visit_array_access(array_access_ast *node)
  {
    visit_node(node->array_index_expression);
  }

  void default_visitor::visit_array_creator_rest(array_creator_rest_ast *node)
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

  void default_visitor::visit_array_type_dot_class(array_type_dot_class_ast *node)
  {
    visit_node(node->qualified_identifier);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_assert_statement(assert_statement_ast *node)
  {
    visit_node(node->condition);
    visit_node(node->message);
  }

  void default_visitor::visit_bit_and_expression(bit_and_expression_ast *node)
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

  void default_visitor::visit_bit_or_expression(bit_or_expression_ast *node)
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

  void default_visitor::visit_bit_xor_expression(bit_xor_expression_ast *node)
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

  void default_visitor::visit_block(block_ast *node)
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

  void default_visitor::visit_block_statement(block_statement_ast *node)
  {
    visit_node(node->variable_declaration_statement);
    visit_node(node->statement);
    visit_node(node->class_declaration);
    visit_node(node->enum_declaration);
    visit_node(node->interface_declaration);
    visit_node(node->annotation_type_declaration);
  }

  void default_visitor::visit_break_statement(break_statement_ast *node)
  {
    visit_node(node->label);
  }

  void default_visitor::visit_builtin_type(builtin_type_ast *)
  {}

  void default_visitor::visit_builtin_type_dot_class(builtin_type_dot_class_ast *node)
  {
    visit_node(node->builtin_type);
  }

  void default_visitor::visit_cast_expression(cast_expression_ast *node)
  {
    visit_node(node->builtin_type);
    visit_node(node->builtin_casted_expression);
    visit_node(node->class_type);
    visit_node(node->class_casted_expression);
  }

  void default_visitor::visit_catch_clause(catch_clause_ast *node)
  {
    visit_node(node->exception_declaration);
    visit_node(node->body);
  }

  void default_visitor::visit_class_access_data(class_access_data_ast *)
  {}

  void default_visitor::visit_class_body(class_body_ast *node)
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

  void default_visitor::visit_class_declaration(class_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->class_name);
    visit_node(node->type_parameters);
    visit_node(node->extends);
    visit_node(node->implements);
    visit_node(node->body);
  }

  void default_visitor::visit_class_extends_clause(class_extends_clause_ast *node)
  {
    visit_node(node->type);
  }

  void default_visitor::visit_class_field(class_field_ast *node)
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

  void default_visitor::visit_class_or_interface_type_name(class_or_interface_type_name_ast *node)
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

  void default_visitor::visit_class_or_interface_type_name_part(class_or_interface_type_name_part_ast *node)
  {
    visit_node(node->identifier);
    visit_node(node->type_arguments);
  }

  void default_visitor::visit_class_type(class_type_ast *node)
  {
    visit_node(node->type);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_compilation_unit(compilation_unit_ast *node)
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

  void default_visitor::visit_conditional_expression(conditional_expression_ast *node)
  {
    visit_node(node->logical_or_expression);
    visit_node(node->if_expression);
    visit_node(node->else_expression);
  }

  void default_visitor::visit_constructor_declaration(constructor_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->type_parameters);
    visit_node(node->class_name);
    visit_node(node->parameters);
    visit_node(node->throws_clause);
    visit_node(node->body);
  }

  void default_visitor::visit_continue_statement(continue_statement_ast *node)
  {
    visit_node(node->label);
  }

  void default_visitor::visit_do_while_statement(do_while_statement_ast *node)
  {
    visit_node(node->body);
    visit_node(node->condition);
  }

  void default_visitor::visit_embedded_statement(embedded_statement_ast *node)
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

  void default_visitor::visit_enum_body(enum_body_ast *node)
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

  void default_visitor::visit_enum_constant(enum_constant_ast *node)
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

  void default_visitor::visit_enum_constant_body(enum_constant_body_ast *node)
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

  void default_visitor::visit_enum_constant_field(enum_constant_field_ast *node)
  {
    visit_node(node->class_declaration);
    visit_node(node->enum_declaration);
    visit_node(node->interface_declaration);
    visit_node(node->annotation_type_declaration);
    visit_node(node->method_declaration);
    visit_node(node->variable_declaration);
    visit_node(node->instance_initializer_block);
  }

  void default_visitor::visit_enum_declaration(enum_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->enum_name);
    visit_node(node->implements);
    visit_node(node->body);
  }

  void default_visitor::visit_equality_expression(equality_expression_ast *node)
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

  void default_visitor::visit_equality_expression_rest(equality_expression_rest_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_expression(expression_ast *node)
  {
    visit_node(node->conditional_expression);
    visit_node(node->assignment_expression);
  }

  void default_visitor::visit_for_clause_traditional_rest(for_clause_traditional_rest_ast *node)
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

  void default_visitor::visit_for_control(for_control_ast *node)
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

  void default_visitor::visit_for_statement(for_statement_ast *node)
  {
    visit_node(node->for_control);
    visit_node(node->for_body);
  }

  void default_visitor::visit_foreach_declaration_data(foreach_declaration_data_ast *node)
  {
    visit_node(node->foreach_parameter);
    visit_node(node->iterable_expression);
  }

  void default_visitor::visit_identifier(identifier_ast *)
  {}

  void default_visitor::visit_if_statement(if_statement_ast *node)
  {
    visit_node(node->condition);
    visit_node(node->if_body);
    visit_node(node->else_body);
  }

  void default_visitor::visit_implements_clause(implements_clause_ast *node)
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

  void default_visitor::visit_import_declaration(import_declaration_ast *node)
  {
    visit_node(node->identifier_name);
  }

  void default_visitor::visit_interface_body(interface_body_ast *node)
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

  void default_visitor::visit_interface_declaration(interface_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->interface_name);
    visit_node(node->type_parameters);
    visit_node(node->extends);
    visit_node(node->body);
  }

  void default_visitor::visit_interface_extends_clause(interface_extends_clause_ast *node)
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

  void default_visitor::visit_interface_field(interface_field_ast *node)
  {
    visit_node(node->class_declaration);
    visit_node(node->enum_declaration);
    visit_node(node->interface_declaration);
    visit_node(node->annotation_type_declaration);
    visit_node(node->interface_method_declaration);
    visit_node(node->variable_declaration);
  }

  void default_visitor::visit_interface_method_declaration(interface_method_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->type_parameters);
    visit_node(node->return_type);
    visit_node(node->method_name);
    visit_node(node->parameters);
    visit_node(node->declarator_brackets);
    visit_node(node->throws_clause);
  }

  void default_visitor::visit_labeled_statement(labeled_statement_ast *node)
  {
    visit_node(node->label);
    visit_node(node->statement);
  }

  void default_visitor::visit_literal(literal_ast *)
  {}

  void default_visitor::visit_logical_and_expression(logical_and_expression_ast *node)
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

  void default_visitor::visit_logical_or_expression(logical_or_expression_ast *node)
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

  void default_visitor::visit_mandatory_array_builtin_type(mandatory_array_builtin_type_ast *node)
  {
    visit_node(node->type);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_mandatory_declarator_brackets(mandatory_declarator_brackets_ast *)
  {}

  void default_visitor::visit_method_call_data(method_call_data_ast *node)
  {
    visit_node(node->type_arguments);
    visit_node(node->method_name);
    visit_node(node->arguments);
  }

  void default_visitor::visit_method_declaration(method_declaration_ast *node)
  {
    visit_node(node->modifiers);
    visit_node(node->type_parameters);
    visit_node(node->return_type);
    visit_node(node->method_name);
    visit_node(node->parameters);
    visit_node(node->declarator_brackets);
    visit_node(node->throws_clause);
  }

  void default_visitor::visit_multiplicative_expression(multiplicative_expression_ast *node)
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

  void default_visitor::visit_multiplicative_expression_rest(multiplicative_expression_rest_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_new_expression(new_expression_ast *node)
  {
    visit_node(node->type_arguments);
    visit_node(node->type);
    visit_node(node->class_constructor_arguments);
    visit_node(node->class_body);
    visit_node(node->array_creator_rest);
  }

  void default_visitor::visit_non_array_type(non_array_type_ast *node)
  {
    visit_node(node->class_or_interface_type);
    visit_node(node->builtin_type);
  }

  void default_visitor::visit_non_wildcard_type_arguments(non_wildcard_type_arguments_ast *node)
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

  void default_visitor::visit_optional_argument_list(optional_argument_list_ast *node)
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

  void default_visitor::visit_optional_array_builtin_type(optional_array_builtin_type_ast *node)
  {
    visit_node(node->type);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_optional_declarator_brackets(optional_declarator_brackets_ast *)
  {}

  void default_visitor::visit_optional_modifiers(optional_modifiers_ast *node)
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

  void default_visitor::visit_optional_parameter_declaration_list(optional_parameter_declaration_list_ast *node)
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

  void default_visitor::visit_optional_parameter_modifiers(optional_parameter_modifiers_ast *node)
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

  void default_visitor::visit_package_declaration(package_declaration_ast *node)
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

  void default_visitor::visit_parameter_declaration(parameter_declaration_ast *node)
  {
    visit_node(node->parameter_modifiers);
    visit_node(node->type);
    visit_node(node->variable_name);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast *node)
  {
    visit_node(node->parameter_modifiers);
    visit_node(node->type);
    visit_node(node->variable_name);
    visit_node(node->declarator_brackets);
  }

  void default_visitor::visit_postfix_operator(postfix_operator_ast *)
  {}

  void default_visitor::visit_primary_atom(primary_atom_ast *node)
  {
    visit_node(node->literal);
    visit_node(node->new_expression);
    visit_node(node->parenthesis_expression);
    visit_node(node->builtin_type_dot_class);
    visit_node(node->this_call);
    visit_node(node->this_access);
    visit_node(node->super_access);
    visit_node(node->method_call);
    visit_node(node->array_type_dot_class);
    visit_node(node->simple_name_access);
  }

  void default_visitor::visit_primary_expression(primary_expression_ast *node)
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

  void default_visitor::visit_primary_selector(primary_selector_ast *node)
  {
    visit_node(node->class_access);
    visit_node(node->this_access);
    visit_node(node->new_expression);
    visit_node(node->simple_name_access);
    visit_node(node->super_access);
    visit_node(node->method_call);
    visit_node(node->array_access);
  }

  void default_visitor::visit_qualified_identifier(qualified_identifier_ast *node)
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

  void default_visitor::visit_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast *node)
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

  void default_visitor::visit_relational_expression(relational_expression_ast *node)
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

  void default_visitor::visit_relational_expression_rest(relational_expression_rest_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_return_statement(return_statement_ast *node)
                                    {
                                      visit_node(node->return_expression);
                                    }

                                    void default_visitor::visit_shift_expression(shift_expression_ast *node)
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

                                    void default_visitor::visit_shift_expression_rest(shift_expression_rest_ast *node)
                                    {
                                      visit_node(node->expression);
                                    }

                                    void default_visitor::visit_simple_name_access_data(simple_name_access_data_ast *node)
                                    {
                                      visit_node(node->name);
                                    }

                                    void default_visitor::visit_statement_expression(statement_expression_ast *node)
                                    {
                                      visit_node(node->expression);
                                    }

                                    void default_visitor::visit_super_access_data(super_access_data_ast *node)
                                    {
                                      visit_node(node->type_arguments);
                                      visit_node(node->super_suffix);
                                    }

                                    void default_visitor::visit_super_suffix(super_suffix_ast *node)
                                    {
                                      visit_node(node->constructor_arguments);
                                      visit_node(node->simple_name_access);
                                      visit_node(node->method_call);
                                    }

                                    void default_visitor::visit_switch_label(switch_label_ast *node)
                                    {
                                      visit_node(node->case_expression);
                                    }

                                    void default_visitor::visit_switch_section(switch_section_ast *node)
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

                                    void default_visitor::visit_switch_statement(switch_statement_ast *node)
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

                                    void default_visitor::visit_synchronized_statement(synchronized_statement_ast *node)
                                    {
                                      visit_node(node->locked_type);
                                      visit_node(node->synchronized_body);
                                    }

                                    void default_visitor::visit_this_access_data(this_access_data_ast *)
                                    {}

                                    void default_visitor::visit_this_call_data(this_call_data_ast *node)
                                    {
                                      visit_node(node->type_arguments);
                                      visit_node(node->arguments);
                                    }

                                    void default_visitor::visit_throw_statement(throw_statement_ast *node)
                                    {
                                      visit_node(node->exception);
                                    }

                                    void default_visitor::visit_throws_clause(throws_clause_ast *node)
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

                                    void default_visitor::visit_try_statement(try_statement_ast *node)
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

                                    void default_visitor::visit_type(type_ast *node)
                                    {
                                      visit_node(node->class_type);
                                      visit_node(node->builtin_type);
                                    }

                                    void default_visitor::visit_type_argument(type_argument_ast *node)
                                    {
                                      visit_node(node->type_argument_type);
                                      visit_node(node->wildcard_type);
                                    }

                                    void default_visitor::visit_type_argument_type(type_argument_type_ast *node)
                                    {
                                      visit_node(node->class_type);
                                      visit_node(node->mandatory_array_builtin_type);
                                    }

                                    void default_visitor::visit_type_arguments(type_arguments_ast *node)
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

                                    void default_visitor::visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *)
                                  {}

                                    void default_visitor::visit_type_declaration(type_declaration_ast *node)
                                    {
                                      visit_node(node->class_declaration);
                                      visit_node(node->enum_declaration);
                                      visit_node(node->interface_declaration);
                                      visit_node(node->annotation_type_declaration);
                                    }

                                    void default_visitor::visit_type_parameter(type_parameter_ast *node)
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

                                    void default_visitor::visit_type_parameters(type_parameters_ast *node)
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

                                    void default_visitor::visit_unary_expression(unary_expression_ast *node)
                                    {
                                      visit_node(node->unary_expression);
                                      visit_node(node->unary_expression_not_plusminus);
                                    }

                                    void default_visitor::visit_unary_expression_not_plusminus(unary_expression_not_plusminus_ast *node)
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

                                    void default_visitor::visit_variable_array_initializer(variable_array_initializer_ast *node)
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

                                    void default_visitor::visit_variable_declaration(variable_declaration_ast *node)
                                    {
                                      visit_node(node->data);
                                    }

                                    void default_visitor::visit_variable_declaration_data(variable_declaration_data_ast *node)
                                    {
                                      visit_node(node->modifiers);
                                      visit_node(node->type);
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
                                    }

                                    void default_visitor::visit_variable_declaration_rest(variable_declaration_rest_ast *node)
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

                                    void default_visitor::visit_variable_declaration_split_data(variable_declaration_split_data_ast *node)
                                    {
                                      visit_node(node->data);
                                    }

                                    void default_visitor::visit_variable_declaration_statement(variable_declaration_statement_ast *node)
                                    {
                                      visit_node(node->variable_declaration);
                                    }

                                    void default_visitor::visit_variable_declarator(variable_declarator_ast *node)
                                    {
                                      visit_node(node->variable_name);
                                      visit_node(node->declarator_brackets);
                                      visit_node(node->initializer);
                                    }

                                    void default_visitor::visit_variable_initializer(variable_initializer_ast *node)
                                    {
                                      visit_node(node->expression);
                                      visit_node(node->array_initializer);
                                    }

                                    void default_visitor::visit_while_statement(while_statement_ast *node)
                                    {
                                      visit_node(node->condition);
                                      visit_node(node->body);
                                    }

                                    void default_visitor::visit_wildcard_type(wildcard_type_ast *node)
                                    {
                                      visit_node(node->bounds);
                                    }

                                    void default_visitor::visit_wildcard_type_bounds(wildcard_type_bounds_ast *node)
                                    {
                                      visit_node(node->type);
                                    }


                                  } // end of namespace java


