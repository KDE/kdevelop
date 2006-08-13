// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_SERIALIZATION_H_INCLUDED
#define csharp_SERIALIZATION_H_INCLUDED

#include "csharp_default_visitor.h"

#include <iostream>
#include <fstream>

namespace csharp
  {

  class serialize: public default_visitor
    {
    public:
      static void read(kdev_pg_memory_pool *p,
                       ast_node *node, std::ifstream *i)
      {
        serialize(p, node, i);
      }

      static void write(ast_node *node, std::ofstream *o)
      {
        serialize(node, o);
      }

    private:
      serialize(kdev_pg_memory_pool *p,
                ast_node *node, std::ifstream *i) : in(i), out(0)
      {
        memory_pool = p;
        if ( !node )
          node = create<compilation_unit_ast>();
        visit_node( node );
      }

      serialize(ast_node *node, std::ofstream *o) : in(0), out(o)
      {
        visit_node( node );
      }

      std::ifstream *in;
      std::ofstream *out;

      // memory pool
      typedef kdev_pg_memory_pool memory_pool_type;
      kdev_pg_memory_pool *memory_pool;
      template <class T>
      inline T *create()
      {
        T *node = new (memory_pool->allocate(sizeof(T))) T();
        node->kind = T::KIND;
        return node;
      }
      template <class T, class E>
      void handle_list_node(const list_node<T> *t, E *e)
      {
        if (in)
          {
            bool b;
            in->read(reinterpret_cast<char*>(&b), sizeof(bool));
            if (b)
              {
                int count;
                in->read(reinterpret_cast<char*>(&count), sizeof(int));
                for ( int i = 0; i < count; ++i )
                  {
                    e = create<E>();
                    t = snoc(t, e, memory_pool);
                  }
              }
          }
        else if (out)
          {
            if (t)
              {
                bool b = true;
                out->write(reinterpret_cast<char*>(&b), sizeof(bool));
                int c = t->count();
                out->write(reinterpret_cast<char*>(&c), sizeof(int));
              }
            else
              {
                bool b = false;
                out->write(reinterpret_cast<char*>(&b), sizeof(bool));
              }

          }
      }

      template <class T>
      void handle_ast_node(T *t)
      {
        if (in)
          {
            bool b;
            in->read(reinterpret_cast<char*>(&b), sizeof(bool));
            if (b)
              {
                t = create<T>();
                in->read(reinterpret_cast<char*>(&t->start_token),
                         sizeof(std::size_t));
                in->read(reinterpret_cast<char*>(&t->end_token),
                         sizeof(std::size_t));
              }
          }
        else if (out)
          {
            if (t)
              {
                bool b = true;
                out->write(reinterpret_cast<char*>(&b), sizeof(bool));
                out->write(reinterpret_cast<char*>(&t->start_token),
                           sizeof(std::size_t));
                out->write(reinterpret_cast<char*>(&t->end_token),
                           sizeof(std::size_t));
              }
            else
              {
                bool b = false;
                out->write(reinterpret_cast<char*>(&b), sizeof(bool));
              }

          }
      }

      template <class T>
      void handle_variable(T *t)
      {
        if (in)
          {
            in->read( reinterpret_cast<char*>(t), sizeof(T));
          }
        else if (out)
          {
            out->write( reinterpret_cast<char*>(t), sizeof(T));
          }
      }

      virtual void visit_accessor_declarations(accessor_declarations_ast *node)
      {
        handle_variable(&node->accessor1_type);
        handle_variable(&node->accessor2_type);
        handle_ast_node(node->accessor1_attributes);
        handle_ast_node(node->accessor1_modifier);
        handle_ast_node(node->accessor1_body);
        handle_ast_node(node->accessor2_attributes);
        handle_ast_node(node->accessor2_modifier);
        handle_ast_node(node->accessor2_body);
        default_visitor::visit_accessor_declarations(node);
      }

      virtual void visit_accessor_modifier(accessor_modifier_ast *node)
      {
        handle_variable(&node->access_policy);
        default_visitor::visit_accessor_modifier(node);
      }

      virtual void visit_additive_expression(additive_expression_ast *node)
      {
        handle_ast_node(node->expression);
        {
          additive_expression_rest_ast *e = 0;
          handle_list_node(node->additional_expression_sequence, e);
        }
        default_visitor::visit_additive_expression(node);
      }

      virtual void visit_additive_expression_rest(additive_expression_rest_ast *node)
      {
        handle_variable(&node->additive_operator);
        handle_ast_node(node->expression);
        default_visitor::visit_additive_expression_rest(node);
      }

      virtual void visit_anonymous_method_expression(anonymous_method_expression_ast *node)
      {
        handle_ast_node(node->anonymous_method_signature);
        handle_ast_node(node->body);
        default_visitor::visit_anonymous_method_expression(node);
      }

      virtual void visit_anonymous_method_parameter(anonymous_method_parameter_ast *node)
      {
        handle_ast_node(node->modifier);
        handle_ast_node(node->type);
        handle_ast_node(node->variable_name);
        default_visitor::visit_anonymous_method_parameter(node);
      }

      virtual void visit_anonymous_method_signature(anonymous_method_signature_ast *node)
      {
        {
          anonymous_method_parameter_ast *e = 0;
          handle_list_node(node->anonymous_method_parameter_sequence, e);
        }
        default_visitor::visit_anonymous_method_signature(node);
      }

      virtual void visit_argument(argument_ast *node)
      {
        handle_variable(&node->argument_type);
        handle_ast_node(node->expression);
        default_visitor::visit_argument(node);
      }

      virtual void visit_array_creation_expression_rest(array_creation_expression_rest_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->array_initializer);
        {
          expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        {
          rank_specifier_ast *e = 0;
          handle_list_node(node->rank_specifier_sequence, e);
        }
        default_visitor::visit_array_creation_expression_rest(node);
      }

      virtual void visit_array_initializer(array_initializer_ast *node)
      {
        {
          variable_initializer_ast *e = 0;
          handle_list_node(node->variable_initializer_sequence, e);
        }
        default_visitor::visit_array_initializer(node);
      }

      virtual void visit_array_type(array_type_ast *node)
      {
        handle_ast_node(node->non_array_type);
        {
          rank_specifier_ast *e = 0;
          handle_list_node(node->rank_specifier_sequence, e);
        }
        default_visitor::visit_array_type(node);
      }

      virtual void visit_attribute(attribute_ast *node)
      {
        handle_ast_node(node->name);
        handle_ast_node(node->arguments);
        default_visitor::visit_attribute(node);
      }

      virtual void visit_attribute_arguments(attribute_arguments_ast *node)
      {
        {
          named_argument_ast *e = 0;
          handle_list_node(node->named_argument_sequence, e);
        }
        {
          positional_argument_ast *e = 0;
          handle_list_node(node->positional_argument_sequence, e);
        }
        default_visitor::visit_attribute_arguments(node);
      }

      virtual void visit_attribute_section(attribute_section_ast *node)
      {
        handle_ast_node(node->target);
        {
          attribute_ast *e = 0;
          handle_list_node(node->attribute_sequence, e);
        }
        default_visitor::visit_attribute_section(node);
      }

      virtual void visit_attribute_target(attribute_target_ast *node)
      {
        handle_ast_node(node->identifier);
        handle_ast_node(node->keyword);
        default_visitor::visit_attribute_target(node);
      }

      virtual void visit_base_access(base_access_ast *node)
      {
        handle_variable(&node->access_type);
        handle_ast_node(node->identifier);
        handle_ast_node(node->type_arguments);
        {
          expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_base_access(node);
      }

      virtual void visit_bit_and_expression(bit_and_expression_ast *node)
      {
        {
          equality_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_bit_and_expression(node);
      }

      virtual void visit_bit_or_expression(bit_or_expression_ast *node)
      {
        {
          bit_xor_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_bit_or_expression(node);
      }

      virtual void visit_bit_xor_expression(bit_xor_expression_ast *node)
      {
        {
          bit_and_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_bit_xor_expression(node);
      }

      virtual void visit_block(block_ast *node)
      {
        {
          block_statement_ast *e = 0;
          handle_list_node(node->statement_sequence, e);
        }
        default_visitor::visit_block(node);
      }

      virtual void visit_block_statement(block_statement_ast *node)
      {
        handle_ast_node(node->labeled_statement);
        handle_ast_node(node->local_constant_declaration_statement);
        handle_ast_node(node->local_variable_declaration_statement);
        handle_ast_node(node->statement);
        default_visitor::visit_block_statement(node);
      }

      virtual void visit_boolean_expression(boolean_expression_ast *node)
      {
        handle_ast_node(node->expression);
        default_visitor::visit_boolean_expression(node);
      }

      virtual void visit_break_statement(break_statement_ast *node)
      {
        default_visitor::visit_break_statement(node);
      }

      virtual void visit_builtin_class_type(builtin_class_type_ast *node)
      {
        handle_variable(&node->type);
        default_visitor::visit_builtin_class_type(node);
      }

      virtual void visit_cast_expression(cast_expression_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->casted_expression);
        default_visitor::visit_cast_expression(node);
      }

      virtual void visit_catch_clauses(catch_clauses_ast *node)
      {
        handle_ast_node(node->general_catch_clause);
        {
          specific_catch_clause_ast *e = 0;
          handle_list_node(node->specific_catch_clause_sequence, e);
        }
        default_visitor::visit_catch_clauses(node);
      }

      virtual void visit_checked_statement(checked_statement_ast *node)
      {
        handle_ast_node(node->body);
        default_visitor::visit_checked_statement(node);
      }

      virtual void visit_class_base(class_base_ast *node)
      {
        handle_ast_node(node->builtin_class_type);
        {
          type_name_ast *e = 0;
          handle_list_node(node->interface_type_sequence, e);
        }
        {
          type_name_ast *e = 0;
          handle_list_node(node->base_type_sequence, e);
        }
        default_visitor::visit_class_base(node);
      }

      virtual void visit_class_body(class_body_ast *node)
      {
        {
          class_member_declaration_ast *e = 0;
          handle_list_node(node->member_declaration_sequence, e);
        }
        default_visitor::visit_class_body(node);
      }

      virtual void visit_class_declaration(class_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_variable(&node->partial);
        handle_ast_node(node->class_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->class_base);
        {
          type_parameter_constraints_clause_ast *e = 0;
          handle_list_node(node->type_parameter_constraints_sequence, e);
        }
        handle_ast_node(node->body);
        default_visitor::visit_class_declaration(node);
      }

      virtual void visit_class_member_declaration(class_member_declaration_ast *node)
      {
        handle_ast_node(node->finalizer_declaration);
        handle_ast_node(node->other_declaration);
        default_visitor::visit_class_member_declaration(node);
      }

      virtual void visit_class_or_struct_member_declaration(class_or_struct_member_declaration_ast *node)
      {
        handle_ast_node(node->constant_declaration);
        handle_ast_node(node->event_declaration);
        handle_ast_node(node->conversion_operator_declaration);
        handle_ast_node(node->constructor_declaration);
        handle_ast_node(node->type_declaration_rest);
        handle_ast_node(node->unary_or_binary_operator_declaration);
        handle_ast_node(node->indexer_declaration);
        handle_ast_node(node->field_declaration);
        handle_ast_node(node->property_declaration);
        handle_ast_node(node->method_declaration);
        default_visitor::visit_class_or_struct_member_declaration(node);
      }

      virtual void visit_class_type(class_type_ast *node)
      {
        handle_ast_node(node->type_name);
        handle_ast_node(node->builtin_class_type);
        default_visitor::visit_class_type(node);
      }

      virtual void visit_compilation_unit(compilation_unit_ast *node)
      {
        {
          extern_alias_directive_ast *e = 0;
          handle_list_node(node->extern_alias_sequence, e);
        }
        {
          using_directive_ast *e = 0;
          handle_list_node(node->using_sequence, e);
        }
        {
          global_attribute_section_ast *e = 0;
          handle_list_node(node->global_attribute_sequence, e);
        }
        {
          namespace_member_declaration_ast *e = 0;
          handle_list_node(node->namespace_sequence, e);
        }
        default_visitor::visit_compilation_unit(node);
      }

      virtual void visit_conditional_expression(conditional_expression_ast *node)
      {
        handle_ast_node(node->null_coalescing_expression);
        handle_ast_node(node->if_expression);
        handle_ast_node(node->else_expression);
        default_visitor::visit_conditional_expression(node);
      }

      virtual void visit_constant_declaration(constant_declaration_ast *node)
      {
        handle_ast_node(node->data);
        default_visitor::visit_constant_declaration(node);
      }

      virtual void visit_constant_declaration_data(constant_declaration_data_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type);
        {
          constant_declarator_ast *e = 0;
          handle_list_node(node->constant_declarator_sequence, e);
        }
        default_visitor::visit_constant_declaration_data(node);
      }

      virtual void visit_constant_declarator(constant_declarator_ast *node)
      {
        handle_ast_node(node->constant_name);
        handle_ast_node(node->expression);
        default_visitor::visit_constant_declarator(node);
      }

      virtual void visit_constant_expression(constant_expression_ast *node)
      {
        handle_ast_node(node->expression);
        default_visitor::visit_constant_expression(node);
      }

      virtual void visit_constructor_constraint(constructor_constraint_ast *node)
      {
        default_visitor::visit_constructor_constraint(node);
      }

      virtual void visit_constructor_declaration(constructor_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->class_name);
        handle_ast_node(node->formal_parameters);
        handle_ast_node(node->constructor_initializer);
        handle_ast_node(node->body);
        default_visitor::visit_constructor_declaration(node);
      }

      virtual void visit_constructor_initializer(constructor_initializer_ast *node)
      {
        handle_variable(&node->initializer_type);
        handle_ast_node(node->arguments);
        default_visitor::visit_constructor_initializer(node);
      }

      virtual void visit_continue_statement(continue_statement_ast *node)
      {
        default_visitor::visit_continue_statement(node);
      }

      virtual void visit_conversion_operator_declaration(conversion_operator_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_variable(&node->conversion);
        handle_ast_node(node->target_type);
        handle_ast_node(node->source_type);
        handle_ast_node(node->source_name);
        handle_ast_node(node->body);
        default_visitor::visit_conversion_operator_declaration(node);
      }

      virtual void visit_delegate_declaration(delegate_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->return_type);
        handle_ast_node(node->delegate_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->formal_parameters);
        {
          type_parameter_constraints_clause_ast *e = 0;
          handle_list_node(node->type_parameter_constraints_sequence, e);
        }
        default_visitor::visit_delegate_declaration(node);
      }

      virtual void visit_do_while_statement(do_while_statement_ast *node)
      {
        handle_ast_node(node->body);
        handle_ast_node(node->condition);
        default_visitor::visit_do_while_statement(node);
      }

      virtual void visit_embedded_statement(embedded_statement_ast *node)
      {
        handle_ast_node(node->block);
        handle_ast_node(node->if_statement);
        handle_ast_node(node->switch_statement);
        handle_ast_node(node->while_statement);
        handle_ast_node(node->do_while_statement);
        handle_ast_node(node->for_statement);
        handle_ast_node(node->foreach_statement);
        handle_ast_node(node->break_statement);
        handle_ast_node(node->continue_statement);
        handle_ast_node(node->goto_statement);
        handle_ast_node(node->return_statement);
        handle_ast_node(node->throw_statement);
        handle_ast_node(node->try_statement);
        handle_ast_node(node->lock_statement);
        handle_ast_node(node->using_statement);
        handle_ast_node(node->checked_statement);
        handle_ast_node(node->unchecked_statement);
        handle_ast_node(node->yield_statement);
        handle_ast_node(node->expression_statement);
        handle_ast_node(node->unsafe_statement);
        handle_ast_node(node->fixed_statement);
        default_visitor::visit_embedded_statement(node);
      }

      virtual void visit_enum_base(enum_base_ast *node)
      {
        handle_ast_node(node->integral_type);
        default_visitor::visit_enum_base(node);
      }

      virtual void visit_enum_body(enum_body_ast *node)
      {
        {
          enum_member_declaration_ast *e = 0;
          handle_list_node(node->member_declaration_sequence, e);
        }
        default_visitor::visit_enum_body(node);
      }

      virtual void visit_enum_declaration(enum_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->enum_name);
        handle_ast_node(node->enum_base);
        handle_ast_node(node->body);
        default_visitor::visit_enum_declaration(node);
      }

      virtual void visit_enum_member_declaration(enum_member_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->member_name);
        handle_ast_node(node->value);
        default_visitor::visit_enum_member_declaration(node);
      }

      virtual void visit_equality_expression(equality_expression_ast *node)
      {
        handle_ast_node(node->expression);
        {
          equality_expression_rest_ast *e = 0;
          handle_list_node(node->additional_expression_sequence, e);
        }
        default_visitor::visit_equality_expression(node);
      }

      virtual void visit_equality_expression_rest(equality_expression_rest_ast *node)
      {
        handle_variable(&node->equality_operator);
        handle_ast_node(node->expression);
        default_visitor::visit_equality_expression_rest(node);
      }

      virtual void visit_event_accessor_declaration(event_accessor_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->body);
        default_visitor::visit_event_accessor_declaration(node);
      }

      virtual void visit_event_accessor_declarations(event_accessor_declarations_ast *node)
      {
        handle_ast_node(node->add_accessor_declaration);
        handle_ast_node(node->remove_accessor_declaration);
        default_visitor::visit_event_accessor_declarations(node);
      }

      virtual void visit_event_declaration(event_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type);
        {
          variable_declarator_ast *e = 0;
          handle_list_node(node->variable_declarator_sequence, e);
        }
        handle_ast_node(node->event_name);
        handle_ast_node(node->event_accessor_declarations);
        default_visitor::visit_event_declaration(node);
      }

      virtual void visit_expression(expression_ast *node)
      {
        handle_variable(&node->assignment_operator);
        handle_ast_node(node->conditional_expression);
        handle_ast_node(node->assignment_expression);
        default_visitor::visit_expression(node);
      }

      virtual void visit_extern_alias_directive(extern_alias_directive_ast *node)
      {
        handle_ast_node(node->identifier);
        default_visitor::visit_extern_alias_directive(node);
      }

      virtual void visit_finalizer_declaration(finalizer_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->class_name);
        handle_ast_node(node->finalizer_body);
        default_visitor::visit_finalizer_declaration(node);
      }

      virtual void visit_fixed_pointer_declarator(fixed_pointer_declarator_ast *node)
      {
        handle_ast_node(node->pointer_name);
        handle_ast_node(node->initializer);
        default_visitor::visit_fixed_pointer_declarator(node);
      }

      virtual void visit_fixed_statement(fixed_statement_ast *node)
      {
        handle_ast_node(node->pointer_type);
        handle_ast_node(node->fixed_pointer_declarator);
        handle_ast_node(node->body);
        default_visitor::visit_fixed_statement(node);
      }

      virtual void visit_floating_point_type(floating_point_type_ast *node)
      {
        handle_variable(&node->type);
        default_visitor::visit_floating_point_type(node);
      }

      virtual void visit_for_control(for_control_ast *node)
      {
        handle_ast_node(node->local_variable_declaration);
        {
          statement_expression_ast *e = 0;
          handle_list_node(node->statement_expression_sequence, e);
        }
        handle_ast_node(node->for_condition);
        {
          statement_expression_ast *e = 0;
          handle_list_node(node->for_iterator_sequence, e);
        }
        default_visitor::visit_for_control(node);
      }

      virtual void visit_for_statement(for_statement_ast *node)
      {
        handle_ast_node(node->for_control);
        handle_ast_node(node->for_body);
        default_visitor::visit_for_statement(node);
      }

      virtual void visit_foreach_statement(foreach_statement_ast *node)
      {
        handle_ast_node(node->variable_type);
        handle_ast_node(node->variable_name);
        handle_ast_node(node->collection);
        handle_ast_node(node->body);
        default_visitor::visit_foreach_statement(node);
      }

      virtual void visit_formal_parameter(formal_parameter_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->params_type);
        handle_ast_node(node->variable_name);
        handle_ast_node(node->modifier);
        handle_ast_node(node->type);
        default_visitor::visit_formal_parameter(node);
      }

      virtual void visit_formal_parameter_list(formal_parameter_list_ast *node)
      {
        {
          formal_parameter_ast *e = 0;
          handle_list_node(node->formal_parameter_sequence, e);
        }
        default_visitor::visit_formal_parameter_list(node);
      }

      virtual void visit_general_catch_clause(general_catch_clause_ast *node)
      {
        handle_ast_node(node->body);
        default_visitor::visit_general_catch_clause(node);
      }

      virtual void visit_generic_dimension_specifier(generic_dimension_specifier_ast *node)
      {
        handle_variable(&node->comma_count);
        default_visitor::visit_generic_dimension_specifier(node);
      }

      virtual void visit_global_attribute_section(global_attribute_section_ast *node)
      {
        {
          attribute_ast *e = 0;
          handle_list_node(node->attribute_sequence, e);
        }
        default_visitor::visit_global_attribute_section(node);
      }

      virtual void visit_goto_statement(goto_statement_ast *node)
      {
        handle_variable(&node->goto_type);
        handle_ast_node(node->label);
        handle_ast_node(node->constant_expression);
        default_visitor::visit_goto_statement(node);
      }

      virtual void visit_identifier(identifier_ast *node)
      {
        handle_variable(&node->ident);
        default_visitor::visit_identifier(node);
      }

      virtual void visit_if_statement(if_statement_ast *node)
      {
        handle_ast_node(node->condition);
        handle_ast_node(node->if_body);
        handle_ast_node(node->else_body);
        default_visitor::visit_if_statement(node);
      }

      virtual void visit_indexer_declaration(indexer_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type);
        handle_ast_node(node->interface_type);
        handle_ast_node(node->formal_parameters);
        handle_ast_node(node->accessor_declarations);
        default_visitor::visit_indexer_declaration(node);
      }

      virtual void visit_integral_type(integral_type_ast *node)
      {
        handle_variable(&node->type);
        default_visitor::visit_integral_type(node);
      }

      virtual void visit_interface_accessors(interface_accessors_ast *node)
      {
        handle_variable(&node->accessor1_type);
        handle_variable(&node->accessor2_type);
        handle_ast_node(node->accessor1_attributes);
        handle_ast_node(node->accessor2_attributes);
        default_visitor::visit_interface_accessors(node);
      }

      virtual void visit_interface_base(interface_base_ast *node)
      {
        {
          type_name_ast *e = 0;
          handle_list_node(node->interface_type_sequence, e);
        }
        default_visitor::visit_interface_base(node);
      }

      virtual void visit_interface_body(interface_body_ast *node)
      {
        {
          interface_member_declaration_ast *e = 0;
          handle_list_node(node->member_declaration_sequence, e);
        }
        default_visitor::visit_interface_body(node);
      }

      virtual void visit_interface_declaration(interface_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_variable(&node->partial);
        handle_ast_node(node->interface_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->interface_base);
        {
          type_parameter_constraints_clause_ast *e = 0;
          handle_list_node(node->type_parameter_constraints_sequence, e);
        }
        handle_ast_node(node->body);
        default_visitor::visit_interface_declaration(node);
      }

      virtual void visit_interface_event_declaration(interface_event_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_variable(&node->decl_new);
        handle_ast_node(node->event_type);
        handle_ast_node(node->event_name);
        default_visitor::visit_interface_event_declaration(node);
      }

      virtual void visit_interface_indexer_declaration(interface_indexer_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_variable(&node->decl_new);
        handle_ast_node(node->type);
        handle_ast_node(node->formal_parameters);
        handle_ast_node(node->interface_accessors);
        default_visitor::visit_interface_indexer_declaration(node);
      }

      virtual void visit_interface_member_declaration(interface_member_declaration_ast *node)
      {
        handle_ast_node(node->event_declaration);
        handle_ast_node(node->indexer_declaration);
        handle_ast_node(node->interface_property_declaration);
        handle_ast_node(node->interface_method_declaration);
        default_visitor::visit_interface_member_declaration(node);
      }

      virtual void visit_interface_method_declaration(interface_method_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_variable(&node->decl_new);
        handle_ast_node(node->return_type);
        handle_ast_node(node->method_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->formal_parameters);
        {
          type_parameter_constraints_clause_ast *e = 0;
          handle_list_node(node->type_parameter_constraints_sequence, e);
        }
        default_visitor::visit_interface_method_declaration(node);
      }

      virtual void visit_interface_property_declaration(interface_property_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_variable(&node->decl_new);
        handle_ast_node(node->type);
        handle_ast_node(node->property_name);
        handle_ast_node(node->interface_accessors);
        default_visitor::visit_interface_property_declaration(node);
      }

      virtual void visit_keyword(keyword_ast *node)
      {
        handle_variable(&node->keyword);
        default_visitor::visit_keyword(node);
      }

      virtual void visit_labeled_statement(labeled_statement_ast *node)
      {
        handle_ast_node(node->label);
        default_visitor::visit_labeled_statement(node);
      }

      virtual void visit_literal(literal_ast *node)
      {
        handle_variable(&node->literal_type);
        handle_variable(&node->integer_literal);
        handle_variable(&node->floating_point_literal);
        handle_variable(&node->character_literal);
        handle_variable(&node->string_literal);
        default_visitor::visit_literal(node);
      }

      virtual void visit_local_constant_declaration(local_constant_declaration_ast *node)
      {
        handle_ast_node(node->data);
        default_visitor::visit_local_constant_declaration(node);
      }

      virtual void visit_local_variable_declaration(local_variable_declaration_ast *node)
      {
        handle_ast_node(node->data);
        default_visitor::visit_local_variable_declaration(node);
      }

      virtual void visit_local_variable_declaration_statement(local_variable_declaration_statement_ast *node)
      {
        handle_ast_node(node->declaration);
        default_visitor::visit_local_variable_declaration_statement(node);
      }

      virtual void visit_lock_statement(lock_statement_ast *node)
      {
        handle_ast_node(node->lock_expression);
        handle_ast_node(node->body);
        default_visitor::visit_lock_statement(node);
      }

      virtual void visit_logical_and_expression(logical_and_expression_ast *node)
      {
        {
          bit_or_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_logical_and_expression(node);
      }

      virtual void visit_logical_or_expression(logical_or_expression_ast *node)
      {
        {
          logical_and_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_logical_or_expression(node);
      }

      virtual void visit_managed_type(managed_type_ast *node)
      {
        handle_ast_node(node->non_array_type);
        {
          rank_specifier_ast *e = 0;
          handle_list_node(node->rank_specifier_sequence, e);
        }
        default_visitor::visit_managed_type(node);
      }

      virtual void visit_method_declaration(method_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->return_type);
        handle_ast_node(node->method_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->formal_parameters);
        {
          type_parameter_constraints_clause_ast *e = 0;
          handle_list_node(node->type_parameter_constraints_sequence, e);
        }
        handle_ast_node(node->method_body);
        default_visitor::visit_method_declaration(node);
      }

      virtual void visit_multiplicative_expression(multiplicative_expression_ast *node)
      {
        handle_ast_node(node->expression);
        {
          multiplicative_expression_rest_ast *e = 0;
          handle_list_node(node->additional_expression_sequence, e);
        }
        default_visitor::visit_multiplicative_expression(node);
      }

      virtual void visit_multiplicative_expression_rest(multiplicative_expression_rest_ast *node)
      {
        handle_variable(&node->multiplicative_operator);
        handle_ast_node(node->expression);
        default_visitor::visit_multiplicative_expression_rest(node);
      }

      virtual void visit_named_argument(named_argument_ast *node)
      {
        handle_ast_node(node->argument_name);
        handle_ast_node(node->attribute_argument_expression);
        default_visitor::visit_named_argument(node);
      }

      virtual void visit_namespace_body(namespace_body_ast *node)
      {
        {
          extern_alias_directive_ast *e = 0;
          handle_list_node(node->extern_alias_sequence, e);
        }
        {
          using_directive_ast *e = 0;
          handle_list_node(node->using_sequence, e);
        }
        {
          namespace_member_declaration_ast *e = 0;
          handle_list_node(node->namespace_sequence, e);
        }
        default_visitor::visit_namespace_body(node);
      }

      virtual void visit_namespace_declaration(namespace_declaration_ast *node)
      {
        handle_ast_node(node->name);
        handle_ast_node(node->body);
        default_visitor::visit_namespace_declaration(node);
      }

      virtual void visit_namespace_member_declaration(namespace_member_declaration_ast *node)
      {
        handle_ast_node(node->namespace_declaration);
        handle_ast_node(node->type_declaration);
        default_visitor::visit_namespace_member_declaration(node);
      }

      virtual void visit_namespace_name(namespace_name_ast *node)
      {
        default_visitor::visit_namespace_name(node);
      }

      virtual void visit_namespace_or_type_name(namespace_or_type_name_ast *node)
      {
        handle_ast_node(node->qualified_alias_label);
        {
          namespace_or_type_name_part_ast *e = 0;
          handle_list_node(node->name_part_sequence, e);
        }
        default_visitor::visit_namespace_or_type_name(node);
      }

      virtual void visit_namespace_or_type_name_part(namespace_or_type_name_part_ast *node)
      {
        handle_ast_node(node->identifier);
        handle_ast_node(node->type_arguments);
        default_visitor::visit_namespace_or_type_name_part(node);
      }

      virtual void visit_namespace_or_type_name_safe(namespace_or_type_name_safe_ast *node)
      {
        handle_ast_node(node->qualified_alias_label);
        {
          namespace_or_type_name_part_ast *e = 0;
          handle_list_node(node->name_part_sequence, e);
        }
        default_visitor::visit_namespace_or_type_name_safe(node);
      }

      virtual void visit_new_expression(new_expression_ast *node)
      {
        handle_ast_node(node->array_creation_expression);
        handle_ast_node(node->object_or_delegate_creation_expression);
        default_visitor::visit_new_expression(node);
      }

      virtual void visit_non_array_type(non_array_type_ast *node)
      {
        handle_ast_node(node->builtin_class_type);
        handle_ast_node(node->optionally_nullable_type);
        default_visitor::visit_non_array_type(node);
      }

      virtual void visit_non_nullable_type(non_nullable_type_ast *node)
      {
        handle_ast_node(node->type_name);
        handle_ast_node(node->simple_type);
        default_visitor::visit_non_nullable_type(node);
      }

      virtual void visit_null_coalescing_expression(null_coalescing_expression_ast *node)
      {
        {
          logical_or_expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_null_coalescing_expression(node);
      }

      virtual void visit_numeric_type(numeric_type_ast *node)
      {
        handle_variable(&node->type);
        handle_ast_node(node->int_type);
        handle_ast_node(node->float_type);
        default_visitor::visit_numeric_type(node);
      }

      virtual void visit_object_or_delegate_creation_expression_rest(object_or_delegate_creation_expression_rest_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->argument_list_or_expression);
        default_visitor::visit_object_or_delegate_creation_expression_rest(node);
      }

      virtual void visit_optional_argument_list(optional_argument_list_ast *node)
      {
        {
          argument_ast *e = 0;
          handle_list_node(node->argument_sequence, e);
        }
        default_visitor::visit_optional_argument_list(node);
      }

      virtual void visit_optional_attribute_sections(optional_attribute_sections_ast *node)
      {
        {
          attribute_section_ast *e = 0;
          handle_list_node(node->attribute_sequence, e);
        }
        default_visitor::visit_optional_attribute_sections(node);
      }

      virtual void visit_optional_modifiers(optional_modifiers_ast *node)
      {
        handle_variable(&node->modifiers);
        handle_variable(&node->access_policy);
        default_visitor::visit_optional_modifiers(node);
      }

      virtual void visit_optional_parameter_modifier(optional_parameter_modifier_ast *node)
      {
        handle_variable(&node->parameter_type);
        default_visitor::visit_optional_parameter_modifier(node);
      }

      virtual void visit_optionally_nullable_type(optionally_nullable_type_ast *node)
      {
        handle_variable(&node->nullable);
        handle_ast_node(node->non_nullable_type);
        default_visitor::visit_optionally_nullable_type(node);
      }

      virtual void visit_overloadable_binary_only_operator(overloadable_binary_only_operator_ast *node)
      {
        default_visitor::visit_overloadable_binary_only_operator(node);
      }

      virtual void visit_overloadable_unary_only_operator(overloadable_unary_only_operator_ast *node)
      {
        default_visitor::visit_overloadable_unary_only_operator(node);
      }

      virtual void visit_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast *node)
      {
        default_visitor::visit_overloadable_unary_or_binary_operator(node);
      }

      virtual void visit_pointer_type(pointer_type_ast *node)
      {
        handle_variable(&node->type);
        handle_variable(&node->star_count);
        handle_ast_node(node->regular_type);
        {
          unmanaged_type_suffix_ast *e = 0;
          handle_list_node(node->unmanaged_type_suffix_sequence, e);
        }
        default_visitor::visit_pointer_type(node);
      }

      virtual void visit_positional_argument(positional_argument_ast *node)
      {
        handle_ast_node(node->attribute_argument_expression);
        default_visitor::visit_positional_argument(node);
      }

      virtual void visit_predefined_type(predefined_type_ast *node)
      {
        handle_variable(&node->type);
        default_visitor::visit_predefined_type(node);
      }

      virtual void visit_primary_atom(primary_atom_ast *node)
      {
        handle_variable(&node->rule_type);
        handle_ast_node(node->literal);
        handle_ast_node(node->expression);
        handle_ast_node(node->simple_name_or_member_access);
        handle_ast_node(node->base_access);
        handle_ast_node(node->new_expression);
        handle_ast_node(node->typeof_expression);
        handle_ast_node(node->type);
        handle_ast_node(node->anonymous_method_expression);
        handle_ast_node(node->unmanaged_type);
        default_visitor::visit_primary_atom(node);
      }

      virtual void visit_primary_expression(primary_expression_ast *node)
      {
        handle_ast_node(node->primary_atom);
        {
          primary_suffix_ast *e = 0;
          handle_list_node(node->primary_suffix_sequence, e);
        }
        default_visitor::visit_primary_expression(node);
      }

      virtual void visit_primary_or_secondary_constraint(primary_or_secondary_constraint_ast *node)
      {
        handle_variable(&node->constraint_type);
        handle_ast_node(node->class_type_or_secondary_constraint);
        default_visitor::visit_primary_or_secondary_constraint(node);
      }

      virtual void visit_primary_suffix(primary_suffix_ast *node)
      {
        handle_variable(&node->suffix_type);
        handle_ast_node(node->member_name);
        handle_ast_node(node->type_arguments);
        handle_ast_node(node->arguments);
        {
          expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_primary_suffix(node);
      }

      virtual void visit_property_declaration(property_declaration_ast *node)
      {
        handle_ast_node(node->attributes);
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type);
        handle_ast_node(node->property_name);
        handle_ast_node(node->accessor_declarations);
        default_visitor::visit_property_declaration(node);
      }

      virtual void visit_qualified_identifier(qualified_identifier_ast *node)
      {
        {
          identifier_ast *e = 0;
          handle_list_node(node->name_sequence, e);
        }
        default_visitor::visit_qualified_identifier(node);
      }

      virtual void visit_rank_specifier(rank_specifier_ast *node)
      {
        handle_variable(&node->dimension_seperator_count);
        default_visitor::visit_rank_specifier(node);
      }

      virtual void visit_relational_expression(relational_expression_ast *node)
      {
        handle_ast_node(node->expression);
        {
          relational_expression_rest_ast *e = 0;
          handle_list_node(node->additional_expression_sequence, e);
        }
        default_visitor::visit_relational_expression(node);
      }

      virtual void visit_relational_expression_rest(relational_expression_rest_ast *node)
      {
        handle_variable(&node->relational_operator);
        handle_ast_node(node->expression);
        handle_ast_node(node->type);
        default_visitor::visit_relational_expression_rest(node);
      }

      virtual void visit_resource_acquisition(resource_acquisition_ast *node)
      {
        handle_ast_node(node->local_variable_declaration);
        handle_ast_node(node->expression);
        default_visitor::visit_resource_acquisition(node);
      }

      virtual void visit_return_statement(return_statement_ast *node)
                               {
                                 handle_ast_node(node->return_expression);
                                 default_visitor::visit_return_statement(node);
                               }

                               virtual void visit_return_type(return_type_ast *node)
                                                        {
                                                          handle_variable(&node->type);
                                                          handle_ast_node(node->regular_type);
                                                          default_visitor::visit_return_type(node);
                                                        }

                                                        virtual void visit_secondary_constraint(secondary_constraint_ast *node)
                                                        {
                                                          {
                                                            type_name_ast *e = 0;
                                                            handle_list_node(node->interface_type_or_type_parameter_sequence, e);
                                                          }
                                                          default_visitor::visit_secondary_constraint(node);
                                                        }

                                                        virtual void visit_shift_expression(shift_expression_ast *node)
                                                        {
                                                          handle_ast_node(node->expression);
                                                          {
                                                            shift_expression_rest_ast *e = 0;
                                                            handle_list_node(node->additional_expression_sequence, e);
                                                          }
                                                          default_visitor::visit_shift_expression(node);
                                                        }

                                                        virtual void visit_shift_expression_rest(shift_expression_rest_ast *node)
                                                        {
                                                          handle_variable(&node->shift_operator);
                                                          handle_ast_node(node->expression);
                                                          default_visitor::visit_shift_expression_rest(node);
                                                        }

                                                        virtual void visit_simple_name_or_member_access(simple_name_or_member_access_ast *node)
                                                        {
                                                          handle_ast_node(node->qualified_alias_label);
                                                          handle_ast_node(node->member_name);
                                                          handle_ast_node(node->type_arguments);
                                                          handle_ast_node(node->predefined_type);
                                                          default_visitor::visit_simple_name_or_member_access(node);
                                                        }

                                                        virtual void visit_simple_type(simple_type_ast *node)
                                                        {
                                                          handle_variable(&node->type);
                                                          handle_ast_node(node->numeric_type);
                                                          default_visitor::visit_simple_type(node);
                                                        }

                                                        virtual void visit_specific_catch_clause(specific_catch_clause_ast *node)
                                                        {
                                                          handle_ast_node(node->exception_type);
                                                          handle_ast_node(node->exception_name);
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_specific_catch_clause(node);
                                                        }

                                                        virtual void visit_stackalloc_initializer(stackalloc_initializer_ast *node)
                                                        {
                                                          handle_ast_node(node->expression);
                                                          default_visitor::visit_stackalloc_initializer(node);
                                                        }

                                                        virtual void visit_statement_expression(statement_expression_ast *node)
                                                        {
                                                          handle_ast_node(node->expression);
                                                          default_visitor::visit_statement_expression(node);
                                                        }

                                                        virtual void visit_struct_body(struct_body_ast *node)
                                                        {
                                                          {
                                                            struct_member_declaration_ast *e = 0;
                                                            handle_list_node(node->member_declaration_sequence, e);
                                                          }
                                                          default_visitor::visit_struct_body(node);
                                                        }

                                                        virtual void visit_struct_declaration(struct_declaration_ast *node)
                                                        {
                                                          handle_ast_node(node->attributes);
                                                          handle_ast_node(node->modifiers);
                                                          handle_variable(&node->partial);
                                                          handle_ast_node(node->struct_name);
                                                          handle_ast_node(node->type_parameters);
                                                          handle_ast_node(node->struct_interfaces);
                                                          {
                                                            type_parameter_constraints_clause_ast *e = 0;
                                                            handle_list_node(node->type_parameter_constraints_sequence, e);
                                                          }
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_struct_declaration(node);
                                                        }

                                                        virtual void visit_struct_interfaces(struct_interfaces_ast *node)
                                                        {
                                                          {
                                                            type_name_ast *e = 0;
                                                            handle_list_node(node->interface_type_sequence, e);
                                                          }
                                                          default_visitor::visit_struct_interfaces(node);
                                                        }

                                                        virtual void visit_struct_member_declaration(struct_member_declaration_ast *node)
                                                        {
                                                          handle_ast_node(node->declaration);
                                                          default_visitor::visit_struct_member_declaration(node);
                                                        }

                                                        virtual void visit_switch_label(switch_label_ast *node)
                                                        {
                                                          handle_variable(&node->branch_type);
                                                          handle_ast_node(node->case_expression);
                                                          default_visitor::visit_switch_label(node);
                                                        }

                                                        virtual void visit_switch_section(switch_section_ast *node)
                                                        {
                                                          {
                                                            switch_label_ast *e = 0;
                                                            handle_list_node(node->label_sequence, e);
                                                          }
                                                          {
                                                            block_statement_ast *e = 0;
                                                            handle_list_node(node->statement_sequence, e);
                                                          }
                                                          default_visitor::visit_switch_section(node);
                                                        }

                                                        virtual void visit_switch_statement(switch_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->switch_expression);
                                                          {
                                                            switch_section_ast *e = 0;
                                                            handle_list_node(node->switch_section_sequence, e);
                                                          }
                                                          default_visitor::visit_switch_statement(node);
                                                        }

                                                        virtual void visit_throw_statement(throw_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->exception);
                                                          default_visitor::visit_throw_statement(node);
                                                        }

                                                        virtual void visit_try_statement(try_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->try_body);
                                                          handle_ast_node(node->catch_clauses);
                                                          handle_ast_node(node->finally_body);
                                                          default_visitor::visit_try_statement(node);
                                                        }

                                                        virtual void visit_type(type_ast *node)
                                                        {
                                                          handle_ast_node(node->unmanaged_type);
                                                          default_visitor::visit_type(node);
                                                        }

                                                        virtual void visit_type_arguments(type_arguments_ast *node)
                                                        {
                                                          {
                                                            type_ast *e = 0;
                                                            handle_list_node(node->type_argument_sequence, e);
                                                          }
                                                          default_visitor::visit_type_arguments(node);
                                                        }

                                                        virtual void visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *node)
                                                        {
                                                          default_visitor::visit_type_arguments_or_parameters_end(node);
                                                        }

                                                        virtual void visit_type_declaration(type_declaration_ast *node)
                                                        {
                                                          handle_ast_node(node->rest);
                                                          default_visitor::visit_type_declaration(node);
                                                        }

                                                        virtual void visit_type_declaration_rest(type_declaration_rest_ast *node)
                                                        {
                                                          handle_ast_node(node->class_declaration);
                                                          handle_ast_node(node->struct_declaration);
                                                          handle_ast_node(node->interface_declaration);
                                                          handle_ast_node(node->enum_declaration);
                                                          handle_ast_node(node->delegate_declaration);
                                                          default_visitor::visit_type_declaration_rest(node);
                                                        }

                                                        virtual void visit_type_name(type_name_ast *node)
                                                        {
                                                          default_visitor::visit_type_name(node);
                                                        }

                                                        virtual void visit_type_name_safe(type_name_safe_ast *node)
                                                        {
                                                          default_visitor::visit_type_name_safe(node);
                                                        }

                                                        virtual void visit_type_parameter(type_parameter_ast *node)
                                                        {
                                                          handle_ast_node(node->attributes);
                                                          handle_ast_node(node->parameter_name);
                                                          default_visitor::visit_type_parameter(node);
                                                        }

                                                        virtual void visit_type_parameter_constraints(type_parameter_constraints_ast *node)
                                                        {
                                                          handle_ast_node(node->primary_or_secondary_constraint);
                                                          {
                                                            secondary_constraint_ast *e = 0;
                                                            handle_list_node(node->secondary_constraint_sequence, e);
                                                          }
                                                          handle_ast_node(node->constructor_constraint);
                                                          default_visitor::visit_type_parameter_constraints(node);
                                                        }

                                                        virtual void visit_type_parameter_constraints_clause(type_parameter_constraints_clause_ast *node)
                                                        {
                                                          handle_ast_node(node->type_parameter);
                                                          handle_ast_node(node->constraints);
                                                          default_visitor::visit_type_parameter_constraints_clause(node);
                                                        }

                                                        virtual void visit_type_parameters(type_parameters_ast *node)
                                                        {
                                                          {
                                                            type_parameter_ast *e = 0;
                                                            handle_list_node(node->type_parameter_sequence, e);
                                                          }
                                                          default_visitor::visit_type_parameters(node);
                                                        }

                                                        virtual void visit_typeof_expression(typeof_expression_ast *node)
                                                        {
                                                          handle_variable(&node->typeof_type);
                                                          handle_ast_node(node->unbound_type_name);
                                                          handle_ast_node(node->other_type);
                                                          default_visitor::visit_typeof_expression(node);
                                                        }

                                                        virtual void visit_unary_expression(unary_expression_ast *node)
                                                        {
                                                          handle_variable(&node->rule_type);
                                                          handle_ast_node(node->unary_expression);
                                                          handle_ast_node(node->cast_expression);
                                                          handle_ast_node(node->primary_expression);
                                                          default_visitor::visit_unary_expression(node);
                                                        }

                                                        virtual void visit_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast *node)
                                                        {
                                                          handle_ast_node(node->attributes);
                                                          handle_ast_node(node->modifiers);
                                                          handle_ast_node(node->return_type);
                                                          handle_variable(&node->overloadable_operator_type);
                                                          handle_variable(&node->unary_or_binary);
                                                          handle_ast_node(node->source1_type);
                                                          handle_ast_node(node->source1_name);
                                                          handle_ast_node(node->source2_type);
                                                          handle_ast_node(node->source2_name);
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_unary_or_binary_operator_declaration(node);
                                                        }

                                                        virtual void visit_unbound_type_name(unbound_type_name_ast *node)
                                                        {
                                                          handle_ast_node(node->qualified_alias_label);
                                                          {
                                                            unbound_type_name_part_ast *e = 0;
                                                            handle_list_node(node->name_part_sequence, e);
                                                          }
                                                          default_visitor::visit_unbound_type_name(node);
                                                        }

                                                        virtual void visit_unbound_type_name_part(unbound_type_name_part_ast *node)
                                                        {
                                                          handle_ast_node(node->identifier);
                                                          handle_ast_node(node->generic_dimension_specifier);
                                                          default_visitor::visit_unbound_type_name_part(node);
                                                        }

                                                        virtual void visit_unchecked_statement(unchecked_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_unchecked_statement(node);
                                                        }

                                                        virtual void visit_unmanaged_type(unmanaged_type_ast *node)
                                                        {
                                                          handle_variable(&node->type);
                                                          handle_ast_node(node->regular_type);
                                                          {
                                                            unmanaged_type_suffix_ast *e = 0;
                                                            handle_list_node(node->unmanaged_type_suffix_sequence, e);
                                                          }
                                                          default_visitor::visit_unmanaged_type(node);
                                                        }

                                                        virtual void visit_unmanaged_type_suffix(unmanaged_type_suffix_ast *node)
                                                        {
                                                          handle_variable(&node->type);
                                                          handle_ast_node(node->rank_specifier);
                                                          default_visitor::visit_unmanaged_type_suffix(node);
                                                        }

                                                        virtual void visit_unsafe_statement(unsafe_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_unsafe_statement(node);
                                                        }

                                                        virtual void visit_using_alias_directive_data(using_alias_directive_data_ast *node)
                                                        {
                                                          handle_ast_node(node->alias);
                                                          handle_ast_node(node->namespace_or_type_name);
                                                          default_visitor::visit_using_alias_directive_data(node);
                                                        }

                                                        virtual void visit_using_directive(using_directive_ast *node)
                                                        {
                                                          handle_ast_node(node->using_alias_directive);
                                                          handle_ast_node(node->using_namespace_directive);
                                                          default_visitor::visit_using_directive(node);
                                                        }

                                                        virtual void visit_using_namespace_directive_data(using_namespace_directive_data_ast *node)
                                                        {
                                                          handle_ast_node(node->namespace_name);
                                                          default_visitor::visit_using_namespace_directive_data(node);
                                                        }

                                                        virtual void visit_using_statement(using_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->resource_acquisition);
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_using_statement(node);
                                                        }

                                                        virtual void visit_variable_declaration_data(variable_declaration_data_ast *node)
                                                        {
                                                          handle_ast_node(node->attributes);
                                                          handle_ast_node(node->modifiers);
                                                          handle_ast_node(node->type);
                                                          {
                                                            variable_declarator_ast *e = 0;
                                                            handle_list_node(node->variable_declarator_sequence, e);
                                                          }
                                                          default_visitor::visit_variable_declaration_data(node);
                                                        }

                                                        virtual void visit_variable_declarator(variable_declarator_ast *node)
                                                        {
                                                          handle_ast_node(node->variable_name);
                                                          handle_ast_node(node->array_size);
                                                          handle_ast_node(node->variable_initializer);
                                                          default_visitor::visit_variable_declarator(node);
                                                        }

                                                        virtual void visit_variable_initializer(variable_initializer_ast *node)
                                                        {
                                                          handle_ast_node(node->expression);
                                                          handle_ast_node(node->array_initializer);
                                                          handle_ast_node(node->stackalloc_initializer);
                                                          default_visitor::visit_variable_initializer(node);
                                                        }

                                                        virtual void visit_while_statement(while_statement_ast *node)
                                                        {
                                                          handle_ast_node(node->condition);
                                                          handle_ast_node(node->body);
                                                          default_visitor::visit_while_statement(node);
                                                        }

                                                        virtual void visit_yield_statement(yield_statement_ast *node)
                                                        {
                                                          handle_variable(&node->yield_type);
                                                          handle_ast_node(node->return_expression);
                                                          default_visitor::visit_yield_statement(node);
                                                        }

                                                      };

} // end of namespace csharp

#endif


