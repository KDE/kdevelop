// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp_default_visitor.h"

namespace csharp
  {

  void default_visitor::visit_accessor_declarations(accessor_declarations_ast *node)
  {
    visit_node(node->accessor1_attributes);
    visit_node(node->accessor1_modifier);
    visit_node(node->accessor1_body);
    visit_node(node->accessor2_attributes);
    visit_node(node->accessor2_modifier);
    visit_node(node->accessor2_body);
  }

  void default_visitor::visit_accessor_modifier(accessor_modifier_ast *)
  {}

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

  void default_visitor::visit_anonymous_method_expression(anonymous_method_expression_ast *node)
  {
    visit_node(node->anonymous_method_signature);
    visit_node(node->body);
  }

  void default_visitor::visit_anonymous_method_parameter(anonymous_method_parameter_ast *node)
  {
    visit_node(node->modifier);
    visit_node(node->type);
    visit_node(node->variable_name);
  }

  void default_visitor::visit_anonymous_method_signature(anonymous_method_signature_ast *node)
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

  void default_visitor::visit_argument(argument_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_array_creation_expression_rest(array_creation_expression_rest_ast *node)
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

  void default_visitor::visit_array_initializer(array_initializer_ast *node)
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

  void default_visitor::visit_array_type(array_type_ast *node)
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

  void default_visitor::visit_attribute(attribute_ast *node)
  {
    visit_node(node->name);
    visit_node(node->arguments);
  }

  void default_visitor::visit_attribute_arguments(attribute_arguments_ast *node)
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

  void default_visitor::visit_attribute_section(attribute_section_ast *node)
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

  void default_visitor::visit_attribute_target(attribute_target_ast *node)
  {
    visit_node(node->identifier);
    visit_node(node->keyword);
  }

  void default_visitor::visit_base_access(base_access_ast *node)
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
    visit_node(node->labeled_statement);
    visit_node(node->local_constant_declaration_statement);
    visit_node(node->local_variable_declaration_statement);
    visit_node(node->statement);
  }

  void default_visitor::visit_boolean_expression(boolean_expression_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_break_statement(break_statement_ast *)
  {}

  void default_visitor::visit_builtin_class_type(builtin_class_type_ast *)
  {}

  void default_visitor::visit_cast_expression(cast_expression_ast *node)
  {
    visit_node(node->type);
    visit_node(node->casted_expression);
  }

  void default_visitor::visit_catch_clauses(catch_clauses_ast *node)
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

  void default_visitor::visit_checked_statement(checked_statement_ast *node)
  {
    visit_node(node->body);
  }

  void default_visitor::visit_class_base(class_base_ast *node)
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

  void default_visitor::visit_class_body(class_body_ast *node)
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

  void default_visitor::visit_class_declaration(class_declaration_ast *node)
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

  void default_visitor::visit_class_member_declaration(class_member_declaration_ast *node)
  {
    visit_node(node->finalizer_declaration);
    visit_node(node->other_declaration);
  }

  void default_visitor::visit_class_or_struct_member_declaration(class_or_struct_member_declaration_ast *node)
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

  void default_visitor::visit_class_type(class_type_ast *node)
  {
    visit_node(node->type_name);
    visit_node(node->builtin_class_type);
  }

  void default_visitor::visit_compilation_unit(compilation_unit_ast *node)
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

  void default_visitor::visit_conditional_expression(conditional_expression_ast *node)
  {
    visit_node(node->null_coalescing_expression);
    visit_node(node->if_expression);
    visit_node(node->else_expression);
  }

  void default_visitor::visit_constant_declaration(constant_declaration_ast *node)
  {
    visit_node(node->data);
  }

  void default_visitor::visit_constant_declaration_data(constant_declaration_data_ast *node)
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

  void default_visitor::visit_constant_declarator(constant_declarator_ast *node)
  {
    visit_node(node->constant_name);
    visit_node(node->expression);
  }

  void default_visitor::visit_constant_expression(constant_expression_ast *node)
  {
    visit_node(node->expression);
  }

  void default_visitor::visit_constructor_constraint(constructor_constraint_ast *)
  {}

  void default_visitor::visit_constructor_declaration(constructor_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->class_name);
    visit_node(node->formal_parameters);
    visit_node(node->constructor_initializer);
    visit_node(node->body);
  }

  void default_visitor::visit_constructor_initializer(constructor_initializer_ast *node)
  {
    visit_node(node->arguments);
  }

  void default_visitor::visit_continue_statement(continue_statement_ast *)
  {}

  void default_visitor::visit_conversion_operator_declaration(conversion_operator_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->target_type);
    visit_node(node->source_type);
    visit_node(node->source_name);
    visit_node(node->body);
  }

  void default_visitor::visit_delegate_declaration(delegate_declaration_ast *node)
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

  void default_visitor::visit_do_while_statement(do_while_statement_ast *node)
  {
    visit_node(node->body);
    visit_node(node->condition);
  }

  void default_visitor::visit_embedded_statement(embedded_statement_ast *node)
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

  void default_visitor::visit_enum_base(enum_base_ast *node)
  {
    visit_node(node->integral_type);
  }

  void default_visitor::visit_enum_body(enum_body_ast *node)
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

  void default_visitor::visit_enum_declaration(enum_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->enum_name);
    visit_node(node->enum_base);
    visit_node(node->body);
  }

  void default_visitor::visit_enum_member_declaration(enum_member_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->member_name);
    visit_node(node->value);
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

  void default_visitor::visit_event_accessor_declaration(event_accessor_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->body);
  }

  void default_visitor::visit_event_accessor_declarations(event_accessor_declarations_ast *node)
  {
    visit_node(node->add_accessor_declaration);
    visit_node(node->remove_accessor_declaration);
  }

  void default_visitor::visit_event_declaration(event_declaration_ast *node)
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

  void default_visitor::visit_expression(expression_ast *node)
  {
    visit_node(node->conditional_expression);
    visit_node(node->assignment_expression);
  }

  void default_visitor::visit_extern_alias_directive(extern_alias_directive_ast *node)
  {
    visit_node(node->identifier);
  }

  void default_visitor::visit_finalizer_declaration(finalizer_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->class_name);
    visit_node(node->finalizer_body);
  }

  void default_visitor::visit_fixed_pointer_declarator(fixed_pointer_declarator_ast *node)
  {
    visit_node(node->pointer_name);
    visit_node(node->initializer);
  }

  void default_visitor::visit_fixed_statement(fixed_statement_ast *node)
  {
    visit_node(node->pointer_type);
    visit_node(node->fixed_pointer_declarator);
    visit_node(node->body);
  }

  void default_visitor::visit_floating_point_type(floating_point_type_ast *)
  {}

  void default_visitor::visit_for_control(for_control_ast *node)
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

  void default_visitor::visit_for_statement(for_statement_ast *node)
  {
    visit_node(node->for_control);
    visit_node(node->for_body);
  }

  void default_visitor::visit_foreach_statement(foreach_statement_ast *node)
  {
    visit_node(node->variable_type);
    visit_node(node->variable_name);
    visit_node(node->collection);
    visit_node(node->body);
  }

  void default_visitor::visit_formal_parameter(formal_parameter_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->params_type);
    visit_node(node->variable_name);
    visit_node(node->modifier);
    visit_node(node->type);
  }

  void default_visitor::visit_formal_parameter_list(formal_parameter_list_ast *node)
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

  void default_visitor::visit_general_catch_clause(general_catch_clause_ast *node)
  {
    visit_node(node->body);
  }

  void default_visitor::visit_generic_dimension_specifier(generic_dimension_specifier_ast *)
  {}

  void default_visitor::visit_global_attribute_section(global_attribute_section_ast *node)
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

  void default_visitor::visit_goto_statement(goto_statement_ast *node)
  {
    visit_node(node->label);
    visit_node(node->constant_expression);
  }

  void default_visitor::visit_identifier(identifier_ast *)
  {}

  void default_visitor::visit_if_statement(if_statement_ast *node)
  {
    visit_node(node->condition);
    visit_node(node->if_body);
    visit_node(node->else_body);
  }

  void default_visitor::visit_indexer_declaration(indexer_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->type);
    visit_node(node->interface_type);
    visit_node(node->formal_parameters);
    visit_node(node->accessor_declarations);
  }

  void default_visitor::visit_integral_type(integral_type_ast *)
  {}

  void default_visitor::visit_interface_accessors(interface_accessors_ast *node)
  {
    visit_node(node->accessor1_attributes);
    visit_node(node->accessor2_attributes);
  }

  void default_visitor::visit_interface_base(interface_base_ast *node)
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

  void default_visitor::visit_interface_body(interface_body_ast *node)
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

  void default_visitor::visit_interface_declaration(interface_declaration_ast *node)
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

  void default_visitor::visit_interface_event_declaration(interface_event_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->event_type);
    visit_node(node->event_name);
  }

  void default_visitor::visit_interface_indexer_declaration(interface_indexer_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->type);
    visit_node(node->formal_parameters);
    visit_node(node->interface_accessors);
  }

  void default_visitor::visit_interface_member_declaration(interface_member_declaration_ast *node)
  {
    visit_node(node->event_declaration);
    visit_node(node->indexer_declaration);
    visit_node(node->interface_property_declaration);
    visit_node(node->interface_method_declaration);
  }

  void default_visitor::visit_interface_method_declaration(interface_method_declaration_ast *node)
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

  void default_visitor::visit_interface_property_declaration(interface_property_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->type);
    visit_node(node->property_name);
    visit_node(node->interface_accessors);
  }

  void default_visitor::visit_keyword(keyword_ast *)
  {}

  void default_visitor::visit_labeled_statement(labeled_statement_ast *node)
  {
    visit_node(node->label);
  }

  void default_visitor::visit_literal(literal_ast *)
  {}

  void default_visitor::visit_local_constant_declaration(local_constant_declaration_ast *node)
  {
    visit_node(node->data);
  }

  void default_visitor::visit_local_variable_declaration(local_variable_declaration_ast *node)
  {
    visit_node(node->data);
  }

  void default_visitor::visit_local_variable_declaration_statement(local_variable_declaration_statement_ast *node)
  {
    visit_node(node->declaration);
  }

  void default_visitor::visit_lock_statement(lock_statement_ast *node)
  {
    visit_node(node->lock_expression);
    visit_node(node->body);
  }

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

  void default_visitor::visit_managed_type(managed_type_ast *node)
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

  void default_visitor::visit_method_declaration(method_declaration_ast *node)
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

  void default_visitor::visit_named_argument(named_argument_ast *node)
  {
    visit_node(node->argument_name);
    visit_node(node->attribute_argument_expression);
  }

  void default_visitor::visit_namespace_body(namespace_body_ast *node)
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

  void default_visitor::visit_namespace_declaration(namespace_declaration_ast *node)
  {
    visit_node(node->name);
    visit_node(node->body);
  }

  void default_visitor::visit_namespace_member_declaration(namespace_member_declaration_ast *node)
  {
    visit_node(node->namespace_declaration);
    visit_node(node->type_declaration);
  }

  void default_visitor::visit_namespace_name(namespace_name_ast *)
  {}

  void default_visitor::visit_namespace_or_type_name(namespace_or_type_name_ast *node)
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

  void default_visitor::visit_namespace_or_type_name_part(namespace_or_type_name_part_ast *node)
  {
    visit_node(node->identifier);
    visit_node(node->type_arguments);
  }

  void default_visitor::visit_namespace_or_type_name_safe(namespace_or_type_name_safe_ast *node)
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

  void default_visitor::visit_new_expression(new_expression_ast *node)
  {
    visit_node(node->array_creation_expression);
    visit_node(node->object_or_delegate_creation_expression);
  }

  void default_visitor::visit_non_array_type(non_array_type_ast *node)
  {
    visit_node(node->builtin_class_type);
    visit_node(node->optionally_nullable_type);
  }

  void default_visitor::visit_non_nullable_type(non_nullable_type_ast *node)
  {
    visit_node(node->type_name);
    visit_node(node->simple_type);
  }

  void default_visitor::visit_null_coalescing_expression(null_coalescing_expression_ast *node)
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

  void default_visitor::visit_numeric_type(numeric_type_ast *node)
  {
    visit_node(node->int_type);
    visit_node(node->float_type);
  }

  void default_visitor::visit_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast *node)
  {
    visit_node(node->type);
    visit_node(node->argument_list_or_expression);
  }

  void default_visitor::visit_optional_argument_list(optional_argument_list_ast *node)
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

  void default_visitor::visit_optional_attribute_sections(optional_attribute_sections_ast *node)
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

  void default_visitor::visit_optional_modifiers(optional_modifiers_ast *)
{}

  void default_visitor::visit_optional_parameter_modifier(optional_parameter_modifier_ast *)
  {}

  void default_visitor::visit_optionally_nullable_type(optionally_nullable_type_ast *node)
  {
    visit_node(node->non_nullable_type);
  }

  void default_visitor::visit_overloadable_binary_only_operator(overloadable_binary_only_operator_ast *)
  {}

  void default_visitor::visit_overloadable_unary_only_operator(overloadable_unary_only_operator_ast *)
  {}

  void default_visitor::visit_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast *)
  {}

  void default_visitor::visit_pointer_type(pointer_type_ast *node)
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

  void default_visitor::visit_positional_argument(positional_argument_ast *node)
  {
    visit_node(node->attribute_argument_expression);
  }

  void default_visitor::visit_predefined_type(predefined_type_ast *)
  {}

  void default_visitor::visit_primary_atom(primary_atom_ast *node)
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

  void default_visitor::visit_primary_expression(primary_expression_ast *node)
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

  void default_visitor::visit_primary_or_secondary_constraint(primary_or_secondary_constraint_ast *node)
  {
    visit_node(node->class_type_or_secondary_constraint);
  }

  void default_visitor::visit_primary_suffix(primary_suffix_ast *node)
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

  void default_visitor::visit_property_declaration(property_declaration_ast *node)
  {
    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->type);
    visit_node(node->property_name);
    visit_node(node->accessor_declarations);
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

  void default_visitor::visit_rank_specifier(rank_specifier_ast *)
{}

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
  }

  void default_visitor::visit_relational_expression_rest(relational_expression_rest_ast *node)
  {
    visit_node(node->expression);
    visit_node(node->type);
  }

  void default_visitor::visit_resource_acquisition(resource_acquisition_ast *node)
  {
    visit_node(node->local_variable_declaration);
    visit_node(node->expression);
  }

  void default_visitor::visit_return_statement(return_statement_ast *node)
                                    {
                                      visit_node(node->return_expression);
                                    }

                                    void default_visitor::visit_return_type(return_type_ast *node)
                                                                      {
                                                                        visit_node(node->regular_type);
                                                                      }

                                                                      void default_visitor::visit_secondary_constraint(secondary_constraint_ast *node)
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

                                                                      void default_visitor::visit_simple_name_or_member_access(simple_name_or_member_access_ast *node)
                                                                      {
                                                                        visit_node(node->qualified_alias_label);
                                                                        visit_node(node->member_name);
                                                                        visit_node(node->type_arguments);
                                                                        visit_node(node->predefined_type);
                                                                      }

                                                                      void default_visitor::visit_simple_type(simple_type_ast *node)
                                                                      {
                                                                        visit_node(node->numeric_type);
                                                                      }

                                                                      void default_visitor::visit_specific_catch_clause(specific_catch_clause_ast *node)
                                                                      {
                                                                        visit_node(node->exception_type);
                                                                        visit_node(node->exception_name);
                                                                        visit_node(node->body);
                                                                      }

                                                                      void default_visitor::visit_stackalloc_initializer(stackalloc_initializer_ast *node)
                                                                      {
                                                                        visit_node(node->expression);
                                                                      }

                                                                      void default_visitor::visit_statement_expression(statement_expression_ast *node)
                                                                      {
                                                                        visit_node(node->expression);
                                                                      }

                                                                      void default_visitor::visit_struct_body(struct_body_ast *node)
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

                                                                      void default_visitor::visit_struct_declaration(struct_declaration_ast *node)
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

                                                                      void default_visitor::visit_struct_interfaces(struct_interfaces_ast *node)
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

                                                                      void default_visitor::visit_struct_member_declaration(struct_member_declaration_ast *node)
                                                                      {
                                                                        visit_node(node->declaration);
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

                                                                      void default_visitor::visit_throw_statement(throw_statement_ast *node)
                                                                      {
                                                                        visit_node(node->exception);
                                                                      }

                                                                      void default_visitor::visit_try_statement(try_statement_ast *node)
                                                                      {
                                                                        visit_node(node->try_body);
                                                                        visit_node(node->catch_clauses);
                                                                        visit_node(node->finally_body);
                                                                      }

                                                                      void default_visitor::visit_type(type_ast *node)
                                                                      {
                                                                        visit_node(node->unmanaged_type);
                                                                      }

                                                                      void default_visitor::visit_type_arguments(type_arguments_ast *node)
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

                                                                      void default_visitor::visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *)
                                                                    {}

                                                                      void default_visitor::visit_type_declaration(type_declaration_ast *node)
                                                                      {
                                                                        visit_node(node->rest);
                                                                      }

                                                                      void default_visitor::visit_type_declaration_rest(type_declaration_rest_ast *node)
                                                                      {
                                                                        visit_node(node->class_declaration);
                                                                        visit_node(node->struct_declaration);
                                                                        visit_node(node->interface_declaration);
                                                                        visit_node(node->enum_declaration);
                                                                        visit_node(node->delegate_declaration);
                                                                      }

                                                                      void default_visitor::visit_type_name(type_name_ast *)
                                                                      {}

                                                                      void default_visitor::visit_type_name_safe(type_name_safe_ast *)
                                                                      {}

                                                                      void default_visitor::visit_type_parameter(type_parameter_ast *node)
                                                                      {
                                                                        visit_node(node->attributes);
                                                                        visit_node(node->parameter_name);
                                                                      }

                                                                      void default_visitor::visit_type_parameter_constraints(type_parameter_constraints_ast *node)
                                                                      {
                                                                        visit_node(node->primary_or_secondary_constraint);
                                                                        if (node->secondary_constraint_sequence)
                                                                          {
                                                                            const list_node<secondary_constraint_ast*> *__it = node->secondary_constraint_sequence->to_front(), *__end = __it;
                                                                            do
                                                                              {
                                                                                visit_node(__it->element);
                                                                                __it = __it->next;
                                                                              }
                                                                            while (__it != __end);
                                                                          }
                                                                        visit_node(node->constructor_constraint);
                                                                      }

                                                                      void default_visitor::visit_type_parameter_constraints_clause(type_parameter_constraints_clause_ast *node)
                                                                      {
                                                                        visit_node(node->type_parameter);
                                                                        visit_node(node->constraints);
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

                                                                      void default_visitor::visit_typeof_expression(typeof_expression_ast *node)
                                                                      {
                                                                        visit_node(node->unbound_type_name);
                                                                        visit_node(node->other_type);
                                                                      }

                                                                      void default_visitor::visit_unary_expression(unary_expression_ast *node)
                                                                      {
                                                                        visit_node(node->unary_expression);
                                                                        visit_node(node->cast_expression);
                                                                        visit_node(node->primary_expression);
                                                                      }

                                                                      void default_visitor::visit_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast *node)
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

                                                                      void default_visitor::visit_unbound_type_name(unbound_type_name_ast *node)
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

                                                                      void default_visitor::visit_unbound_type_name_part(unbound_type_name_part_ast *node)
                                                                      {
                                                                        visit_node(node->identifier);
                                                                        visit_node(node->generic_dimension_specifier);
                                                                      }

                                                                      void default_visitor::visit_unchecked_statement(unchecked_statement_ast *node)
                                                                      {
                                                                        visit_node(node->body);
                                                                      }

                                                                      void default_visitor::visit_unmanaged_type(unmanaged_type_ast *node)
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

                                                                      void default_visitor::visit_unmanaged_type_suffix(unmanaged_type_suffix_ast *node)
                                                                      {
                                                                        visit_node(node->rank_specifier);
                                                                      }

                                                                      void default_visitor::visit_unsafe_statement(unsafe_statement_ast *node)
                                                                      {
                                                                        visit_node(node->body);
                                                                      }

                                                                      void default_visitor::visit_using_alias_directive_data(using_alias_directive_data_ast *node)
                                                                      {
                                                                        visit_node(node->alias);
                                                                        visit_node(node->namespace_or_type_name);
                                                                      }

                                                                      void default_visitor::visit_using_directive(using_directive_ast *node)
                                                                      {
                                                                        visit_node(node->using_alias_directive);
                                                                        visit_node(node->using_namespace_directive);
                                                                      }

                                                                      void default_visitor::visit_using_namespace_directive_data(using_namespace_directive_data_ast *node)
                                                                      {
                                                                        visit_node(node->namespace_name);
                                                                      }

                                                                      void default_visitor::visit_using_statement(using_statement_ast *node)
                                                                      {
                                                                        visit_node(node->resource_acquisition);
                                                                        visit_node(node->body);
                                                                      }

                                                                      void default_visitor::visit_variable_declaration_data(variable_declaration_data_ast *node)
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

                                                                      void default_visitor::visit_variable_declarator(variable_declarator_ast *node)
                                                                      {
                                                                        visit_node(node->variable_name);
                                                                        visit_node(node->array_size);
                                                                        visit_node(node->variable_initializer);
                                                                      }

                                                                      void default_visitor::visit_variable_initializer(variable_initializer_ast *node)
                                                                      {
                                                                        visit_node(node->expression);
                                                                        visit_node(node->array_initializer);
                                                                        visit_node(node->stackalloc_initializer);
                                                                      }

                                                                      void default_visitor::visit_while_statement(while_statement_ast *node)
                                                                      {
                                                                        visit_node(node->condition);
                                                                        visit_node(node->body);
                                                                      }

                                                                      void default_visitor::visit_yield_statement(yield_statement_ast *node)
                                                                      {
                                                                        visit_node(node->return_expression);
                                                                      }


                                                                    } // end of namespace csharp


