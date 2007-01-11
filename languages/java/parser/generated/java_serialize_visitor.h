// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef java_SERIALIZATION_H_INCLUDED
#define java_SERIALIZATION_H_INCLUDED

#include "java_default_visitor.h"

#include <iostream>
#include <fstream>

namespace java
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

      virtual void visit_annotation(annotation_ast *node)
      {
        handle_variable(&node->has_parentheses);
        handle_ast_node(node->type_name);
        handle_ast_node(node->args);
        default_visitor::visit_annotation(node);
      }

      virtual void visit_annotation_arguments(annotation_arguments_ast *node)
      {
        {
          annotation_element_value_pair_ast *e = 0;
          handle_list_node(node->value_pair_sequence, e);
        }
        handle_ast_node(node->element_value);
        default_visitor::visit_annotation_arguments(node);
      }

      virtual void visit_annotation_element_array_initializer(annotation_element_array_initializer_ast *node)
      {
        {
          annotation_element_array_value_ast *e = 0;
          handle_list_node(node->element_value_sequence, e);
        }
        default_visitor::visit_annotation_element_array_initializer(node);
      }

      virtual void visit_annotation_element_array_value(annotation_element_array_value_ast *node)
      {
        handle_ast_node(node->cond_expression);
        handle_ast_node(node->annotation);
        default_visitor::visit_annotation_element_array_value(node);
      }

      virtual void visit_annotation_element_value(annotation_element_value_ast *node)
      {
        handle_ast_node(node->cond_expression);
        handle_ast_node(node->annotation);
        handle_ast_node(node->element_array_initializer);
        default_visitor::visit_annotation_element_value(node);
      }

      virtual void visit_annotation_element_value_pair(annotation_element_value_pair_ast *node)
      {
        handle_ast_node(node->element_name);
        handle_ast_node(node->element_value);
        default_visitor::visit_annotation_element_value_pair(node);
      }

      virtual void visit_annotation_method_declaration(annotation_method_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->return_type);
        handle_ast_node(node->annotation_name);
        handle_ast_node(node->annotation_element_value);
        default_visitor::visit_annotation_method_declaration(node);
      }

      virtual void visit_annotation_type_body(annotation_type_body_ast *node)
      {
        {
          annotation_type_field_ast *e = 0;
          handle_list_node(node->annotation_type_field_sequence, e);
        }
        default_visitor::visit_annotation_type_body(node);
      }

      virtual void visit_annotation_type_declaration(annotation_type_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->annotation_type_name);
        handle_ast_node(node->body);
        default_visitor::visit_annotation_type_declaration(node);
      }

      virtual void visit_annotation_type_field(annotation_type_field_ast *node)
      {
        handle_ast_node(node->class_declaration);
        handle_ast_node(node->enum_declaration);
        handle_ast_node(node->interface_declaration);
        handle_ast_node(node->annotation_type_declaration);
        handle_ast_node(node->method_declaration);
        handle_ast_node(node->constant_declaration);
        default_visitor::visit_annotation_type_field(node);
      }

      virtual void visit_array_access(array_access_ast *node)
      {
        handle_ast_node(node->array_index_expression);
        default_visitor::visit_array_access(node);
      }

      virtual void visit_array_creator_rest(array_creator_rest_ast *node)
      {
        handle_ast_node(node->mandatory_declarator_brackets);
        handle_ast_node(node->array_initializer);
        {
          expression_ast *e = 0;
          handle_list_node(node->index_expression_sequence, e);
        }
        handle_ast_node(node->optional_declarator_brackets);
        default_visitor::visit_array_creator_rest(node);
      }

      virtual void visit_array_type_dot_class(array_type_dot_class_ast *node)
      {
        handle_ast_node(node->qualified_identifier);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_array_type_dot_class(node);
      }

      virtual void visit_assert_statement(assert_statement_ast *node)
      {
        handle_ast_node(node->condition);
        handle_ast_node(node->message);
        default_visitor::visit_assert_statement(node);
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
        handle_ast_node(node->variable_declaration_statement);
        handle_ast_node(node->statement);
        handle_ast_node(node->class_declaration);
        handle_ast_node(node->enum_declaration);
        handle_ast_node(node->interface_declaration);
        handle_ast_node(node->annotation_type_declaration);
        default_visitor::visit_block_statement(node);
      }

      virtual void visit_break_statement(break_statement_ast *node)
      {
        handle_ast_node(node->label);
        default_visitor::visit_break_statement(node);
      }

      virtual void visit_builtin_type(builtin_type_ast *node)
      {
        handle_variable(&node->type);
        default_visitor::visit_builtin_type(node);
      }

      virtual void visit_builtin_type_dot_class(builtin_type_dot_class_ast *node)
      {
        handle_ast_node(node->builtin_type);
        default_visitor::visit_builtin_type_dot_class(node);
      }

      virtual void visit_cast_expression(cast_expression_ast *node)
      {
        handle_ast_node(node->builtin_type);
        handle_ast_node(node->builtin_casted_expression);
        handle_ast_node(node->class_type);
        handle_ast_node(node->class_casted_expression);
        default_visitor::visit_cast_expression(node);
      }

      virtual void visit_catch_clause(catch_clause_ast *node)
      {
        handle_ast_node(node->exception_declaration);
        handle_ast_node(node->body);
        default_visitor::visit_catch_clause(node);
      }

      virtual void visit_class_access_data(class_access_data_ast *node)
      {
        default_visitor::visit_class_access_data(node);
      }

      virtual void visit_class_body(class_body_ast *node)
      {
        {
          class_field_ast *e = 0;
          handle_list_node(node->declaration_sequence, e);
        }
        default_visitor::visit_class_body(node);
      }

      virtual void visit_class_declaration(class_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->class_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->extends);
        handle_ast_node(node->implements);
        handle_ast_node(node->body);
        default_visitor::visit_class_declaration(node);
      }

      virtual void visit_class_extends_clause(class_extends_clause_ast *node)
      {
        handle_ast_node(node->type);
        default_visitor::visit_class_extends_clause(node);
      }

      virtual void visit_class_field(class_field_ast *node)
      {
        handle_ast_node(node->class_declaration);
        handle_ast_node(node->enum_declaration);
        handle_ast_node(node->interface_declaration);
        handle_ast_node(node->annotation_type_declaration);
        handle_ast_node(node->constructor_declaration);
        handle_ast_node(node->method_declaration);
        handle_ast_node(node->variable_declaration);
        handle_ast_node(node->instance_initializer_block);
        handle_ast_node(node->static_initializer_block);
        default_visitor::visit_class_field(node);
      }

      virtual void visit_class_or_interface_type_name(class_or_interface_type_name_ast *node)
      {
        {
          class_or_interface_type_name_part_ast *e = 0;
          handle_list_node(node->part_sequence, e);
        }
        default_visitor::visit_class_or_interface_type_name(node);
      }

      virtual void visit_class_or_interface_type_name_part(class_or_interface_type_name_part_ast *node)
      {
        handle_ast_node(node->identifier);
        handle_ast_node(node->type_arguments);
        default_visitor::visit_class_or_interface_type_name_part(node);
      }

      virtual void visit_class_type(class_type_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_class_type(node);
      }

      virtual void visit_compilation_unit(compilation_unit_ast *node)
      {
        handle_ast_node(node->package_declaration);
        {
          import_declaration_ast *e = 0;
          handle_list_node(node->import_declaration_sequence, e);
        }
        {
          type_declaration_ast *e = 0;
          handle_list_node(node->type_declaration_sequence, e);
        }
        default_visitor::visit_compilation_unit(node);
      }

      virtual void visit_conditional_expression(conditional_expression_ast *node)
      {
        handle_ast_node(node->logical_or_expression);
        handle_ast_node(node->if_expression);
        handle_ast_node(node->else_expression);
        default_visitor::visit_conditional_expression(node);
      }

      virtual void visit_constructor_declaration(constructor_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->class_name);
        handle_ast_node(node->parameters);
        handle_ast_node(node->throws_clause);
        handle_ast_node(node->body);
        default_visitor::visit_constructor_declaration(node);
      }

      virtual void visit_continue_statement(continue_statement_ast *node)
      {
        handle_ast_node(node->label);
        default_visitor::visit_continue_statement(node);
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
        handle_ast_node(node->assert_statement);
        handle_ast_node(node->if_statement);
        handle_ast_node(node->for_statement);
        handle_ast_node(node->while_statement);
        handle_ast_node(node->do_while_statement);
        handle_ast_node(node->try_statement);
        handle_ast_node(node->switch_statement);
        handle_ast_node(node->synchronized_statement);
        handle_ast_node(node->return_statement);
        handle_ast_node(node->throw_statement);
        handle_ast_node(node->break_statement);
        handle_ast_node(node->continue_statement);
        handle_ast_node(node->labeled_statement);
        handle_ast_node(node->expression_statement);
        default_visitor::visit_embedded_statement(node);
      }

      virtual void visit_enum_body(enum_body_ast *node)
      {
        {
          enum_constant_ast *e = 0;
          handle_list_node(node->enum_constant_sequence, e);
        }
        {
          class_field_ast *e = 0;
          handle_list_node(node->class_field_sequence, e);
        }
        default_visitor::visit_enum_body(node);
      }

      virtual void visit_enum_constant(enum_constant_ast *node)
      {
        {
          annotation_ast *e = 0;
          handle_list_node(node->annotation_sequence, e);
        }
        handle_ast_node(node->identifier);
        handle_ast_node(node->arguments);
        handle_ast_node(node->body);
        default_visitor::visit_enum_constant(node);
      }

      virtual void visit_enum_constant_body(enum_constant_body_ast *node)
      {
        {
          enum_constant_field_ast *e = 0;
          handle_list_node(node->declaration_sequence, e);
        }
        default_visitor::visit_enum_constant_body(node);
      }

      virtual void visit_enum_constant_field(enum_constant_field_ast *node)
      {
        handle_ast_node(node->class_declaration);
        handle_ast_node(node->enum_declaration);
        handle_ast_node(node->interface_declaration);
        handle_ast_node(node->annotation_type_declaration);
        handle_ast_node(node->method_declaration);
        handle_ast_node(node->variable_declaration);
        handle_ast_node(node->instance_initializer_block);
        default_visitor::visit_enum_constant_field(node);
      }

      virtual void visit_enum_declaration(enum_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->enum_name);
        handle_ast_node(node->implements);
        handle_ast_node(node->body);
        default_visitor::visit_enum_declaration(node);
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

      virtual void visit_expression(expression_ast *node)
      {
        handle_variable(&node->assignment_operator);
        handle_ast_node(node->conditional_expression);
        handle_ast_node(node->assignment_expression);
        default_visitor::visit_expression(node);
      }

      virtual void visit_for_clause_traditional_rest(for_clause_traditional_rest_ast *node)
      {
        handle_ast_node(node->for_condition);
        {
          statement_expression_ast *e = 0;
          handle_list_node(node->for_update_expression_sequence, e);
        }
        default_visitor::visit_for_clause_traditional_rest(node);
      }

      virtual void visit_for_control(for_control_ast *node)
      {
        handle_ast_node(node->foreach_declaration);
        handle_ast_node(node->variable_declaration);
        handle_ast_node(node->traditional_for_rest);
        {
          statement_expression_ast *e = 0;
          handle_list_node(node->statement_expression_sequence, e);
        }
        default_visitor::visit_for_control(node);
      }

      virtual void visit_for_statement(for_statement_ast *node)
      {
        handle_ast_node(node->for_control);
        handle_ast_node(node->for_body);
        default_visitor::visit_for_statement(node);
      }

      virtual void visit_foreach_declaration_data(foreach_declaration_data_ast *node)
      {
        handle_ast_node(node->foreach_parameter);
        handle_ast_node(node->iterable_expression);
        default_visitor::visit_foreach_declaration_data(node);
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

      virtual void visit_implements_clause(implements_clause_ast *node)
      {
        {
          class_or_interface_type_name_ast *e = 0;
          handle_list_node(node->type_sequence, e);
        }
        default_visitor::visit_implements_clause(node);
      }

      virtual void visit_import_declaration(import_declaration_ast *node)
      {
        handle_variable(&node->static_import);
        handle_ast_node(node->identifier_name);
        default_visitor::visit_import_declaration(node);
      }

      virtual void visit_interface_body(interface_body_ast *node)
      {
        {
          interface_field_ast *e = 0;
          handle_list_node(node->declaration_sequence, e);
        }
        default_visitor::visit_interface_body(node);
      }

      virtual void visit_interface_declaration(interface_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->interface_name);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->extends);
        handle_ast_node(node->body);
        default_visitor::visit_interface_declaration(node);
      }

      virtual void visit_interface_extends_clause(interface_extends_clause_ast *node)
      {
        {
          class_or_interface_type_name_ast *e = 0;
          handle_list_node(node->type_sequence, e);
        }
        default_visitor::visit_interface_extends_clause(node);
      }

      virtual void visit_interface_field(interface_field_ast *node)
      {
        handle_ast_node(node->class_declaration);
        handle_ast_node(node->enum_declaration);
        handle_ast_node(node->interface_declaration);
        handle_ast_node(node->annotation_type_declaration);
        handle_ast_node(node->interface_method_declaration);
        handle_ast_node(node->variable_declaration);
        default_visitor::visit_interface_field(node);
      }

      virtual void visit_interface_method_declaration(interface_method_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->return_type);
        handle_ast_node(node->method_name);
        handle_ast_node(node->parameters);
        handle_ast_node(node->declarator_brackets);
        handle_ast_node(node->throws_clause);
        default_visitor::visit_interface_method_declaration(node);
      }

      virtual void visit_labeled_statement(labeled_statement_ast *node)
      {
        handle_ast_node(node->label);
        handle_ast_node(node->statement);
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

      virtual void visit_mandatory_array_builtin_type(mandatory_array_builtin_type_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_mandatory_array_builtin_type(node);
      }

      virtual void visit_mandatory_declarator_brackets(mandatory_declarator_brackets_ast *node)
      {
        handle_variable(&node->bracket_count);
        default_visitor::visit_mandatory_declarator_brackets(node);
      }

      virtual void visit_method_call_data(method_call_data_ast *node)
      {
        handle_ast_node(node->type_arguments);
        handle_ast_node(node->method_name);
        handle_ast_node(node->arguments);
        default_visitor::visit_method_call_data(node);
      }

      virtual void visit_method_declaration(method_declaration_ast *node)
      {
        handle_ast_node(node->modifiers);
        handle_ast_node(node->type_parameters);
        handle_ast_node(node->return_type);
        handle_ast_node(node->method_name);
        handle_ast_node(node->parameters);
        handle_ast_node(node->declarator_brackets);
        handle_ast_node(node->throws_clause);
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

      virtual void visit_new_expression(new_expression_ast *node)
      {
        handle_ast_node(node->type_arguments);
        handle_ast_node(node->type);
        handle_ast_node(node->class_constructor_arguments);
        handle_ast_node(node->class_body);
        handle_ast_node(node->array_creator_rest);
        default_visitor::visit_new_expression(node);
      }

      virtual void visit_non_array_type(non_array_type_ast *node)
      {
        handle_ast_node(node->class_or_interface_type);
        handle_ast_node(node->builtin_type);
        default_visitor::visit_non_array_type(node);
      }

      virtual void visit_non_wildcard_type_arguments(non_wildcard_type_arguments_ast *node)
      {
        {
          type_argument_type_ast *e = 0;
          handle_list_node(node->type_argument_type_sequence, e);
        }
        default_visitor::visit_non_wildcard_type_arguments(node);
      }

      virtual void visit_optional_argument_list(optional_argument_list_ast *node)
      {
        {
          expression_ast *e = 0;
          handle_list_node(node->expression_sequence, e);
        }
        default_visitor::visit_optional_argument_list(node);
      }

      virtual void visit_optional_array_builtin_type(optional_array_builtin_type_ast *node)
      {
        handle_ast_node(node->type);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_optional_array_builtin_type(node);
      }

      virtual void visit_optional_declarator_brackets(optional_declarator_brackets_ast *node)
      {
        handle_variable(&node->bracket_count);
        default_visitor::visit_optional_declarator_brackets(node);
      }

      virtual void visit_optional_modifiers(optional_modifiers_ast *node)
      {
        handle_variable(&node->modifiers);
        {
          annotation_ast *e = 0;
          handle_list_node(node->mod_annotation_sequence, e);
        }
        default_visitor::visit_optional_modifiers(node);
      }

      virtual void visit_optional_parameter_declaration_list(optional_parameter_declaration_list_ast *node)
      {
        {
          parameter_declaration_ellipsis_ast *e = 0;
          handle_list_node(node->parameter_declaration_sequence, e);
        }
        default_visitor::visit_optional_parameter_declaration_list(node);
      }

      virtual void visit_optional_parameter_modifiers(optional_parameter_modifiers_ast *node)
      {
        handle_variable(&node->has_mod_final);
        {
          annotation_ast *e = 0;
          handle_list_node(node->mod_annotation_sequence, e);
        }
        default_visitor::visit_optional_parameter_modifiers(node);
      }

      virtual void visit_package_declaration(package_declaration_ast *node)
      {
        {
          annotation_ast *e = 0;
          handle_list_node(node->annotation_sequence, e);
        }
        handle_ast_node(node->package_name);
        default_visitor::visit_package_declaration(node);
      }

      virtual void visit_parameter_declaration(parameter_declaration_ast *node)
      {
        handle_ast_node(node->parameter_modifiers);
        handle_ast_node(node->type);
        handle_ast_node(node->variable_name);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_parameter_declaration(node);
      }

      virtual void visit_parameter_declaration_ellipsis(parameter_declaration_ellipsis_ast *node)
      {
        handle_variable(&node->has_ellipsis);
        handle_ast_node(node->parameter_modifiers);
        handle_ast_node(node->type);
        handle_ast_node(node->variable_name);
        handle_ast_node(node->declarator_brackets);
        default_visitor::visit_parameter_declaration_ellipsis(node);
      }

      virtual void visit_postfix_operator(postfix_operator_ast *node)
      {
        handle_variable(&node->postfix_operator);
        default_visitor::visit_postfix_operator(node);
      }

      virtual void visit_primary_atom(primary_atom_ast *node)
      {
        handle_ast_node(node->literal);
        handle_ast_node(node->new_expression);
        handle_ast_node(node->parenthesis_expression);
        handle_ast_node(node->builtin_type_dot_class);
        handle_ast_node(node->this_call);
        handle_ast_node(node->this_access);
        handle_ast_node(node->super_access);
        handle_ast_node(node->method_call);
        handle_ast_node(node->array_type_dot_class);
        handle_ast_node(node->simple_name_access);
        default_visitor::visit_primary_atom(node);
      }

      virtual void visit_primary_expression(primary_expression_ast *node)
      {
        handle_ast_node(node->primary_atom);
        {
          primary_selector_ast *e = 0;
          handle_list_node(node->selector_sequence, e);
        }
        default_visitor::visit_primary_expression(node);
      }

      virtual void visit_primary_selector(primary_selector_ast *node)
      {
        handle_ast_node(node->class_access);
        handle_ast_node(node->this_access);
        handle_ast_node(node->new_expression);
        handle_ast_node(node->simple_name_access);
        handle_ast_node(node->super_access);
        handle_ast_node(node->method_call);
        handle_ast_node(node->array_access);
        default_visitor::visit_primary_selector(node);
      }

      virtual void visit_qualified_identifier(qualified_identifier_ast *node)
      {
        {
          identifier_ast *e = 0;
          handle_list_node(node->name_sequence, e);
        }
        default_visitor::visit_qualified_identifier(node);
      }

      virtual void visit_qualified_identifier_with_optional_star(qualified_identifier_with_optional_star_ast *node)
      {
        handle_variable(&node->has_star);
        {
          identifier_ast *e = 0;
          handle_list_node(node->name_sequence, e);
        }
        default_visitor::visit_qualified_identifier_with_optional_star(node);
      }

      virtual void visit_relational_expression(relational_expression_ast *node)
      {
        handle_ast_node(node->expression);
        {
          relational_expression_rest_ast *e = 0;
          handle_list_node(node->additional_expression_sequence, e);
        }
        handle_ast_node(node->instanceof_type);
        default_visitor::visit_relational_expression(node);
      }

      virtual void visit_relational_expression_rest(relational_expression_rest_ast *node)
      {
        handle_variable(&node->relational_operator);
        handle_ast_node(node->expression);
        default_visitor::visit_relational_expression_rest(node);
      }

      virtual void visit_return_statement(return_statement_ast *node)
                               {
                                 handle_ast_node(node->return_expression);
                                 default_visitor::visit_return_statement(node);
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

                               virtual void visit_simple_name_access_data(simple_name_access_data_ast *node)
                               {
                                 handle_ast_node(node->name);
                                 default_visitor::visit_simple_name_access_data(node);
                               }

                               virtual void visit_statement_expression(statement_expression_ast *node)
                               {
                                 handle_ast_node(node->expression);
                                 default_visitor::visit_statement_expression(node);
                               }

                               virtual void visit_super_access_data(super_access_data_ast *node)
                               {
                                 handle_ast_node(node->type_arguments);
                                 handle_ast_node(node->super_suffix);
                                 default_visitor::visit_super_access_data(node);
                               }

                               virtual void visit_super_suffix(super_suffix_ast *node)
                               {
                                 handle_ast_node(node->constructor_arguments);
                                 handle_ast_node(node->simple_name_access);
                                 handle_ast_node(node->method_call);
                                 default_visitor::visit_super_suffix(node);
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

                               virtual void visit_synchronized_statement(synchronized_statement_ast *node)
                               {
                                 handle_ast_node(node->locked_type);
                                 handle_ast_node(node->synchronized_body);
                                 default_visitor::visit_synchronized_statement(node);
                               }

                               virtual void visit_this_access_data(this_access_data_ast *node)
                               {
                                 default_visitor::visit_this_access_data(node);
                               }

                               virtual void visit_this_call_data(this_call_data_ast *node)
                               {
                                 handle_ast_node(node->type_arguments);
                                 handle_ast_node(node->arguments);
                                 default_visitor::visit_this_call_data(node);
                               }

                               virtual void visit_throw_statement(throw_statement_ast *node)
                               {
                                 handle_ast_node(node->exception);
                                 default_visitor::visit_throw_statement(node);
                               }

                               virtual void visit_throws_clause(throws_clause_ast *node)
                               {
                                 {
                                   qualified_identifier_ast *e = 0;
                                   handle_list_node(node->identifier_sequence, e);
                                 }
                                 default_visitor::visit_throws_clause(node);
                               }

                               virtual void visit_try_statement(try_statement_ast *node)
                               {
                                 handle_ast_node(node->try_body);
                                 {
                                   catch_clause_ast *e = 0;
                                   handle_list_node(node->catch_clause_sequence, e);
                                 }
                                 handle_ast_node(node->finally_body);
                                 default_visitor::visit_try_statement(node);
                               }

                               virtual void visit_type(type_ast *node)
                               {
                                 handle_ast_node(node->class_type);
                                 handle_ast_node(node->builtin_type);
                                 default_visitor::visit_type(node);
                               }

                               virtual void visit_type_argument(type_argument_ast *node)
                               {
                                 handle_ast_node(node->type_argument_type);
                                 handle_ast_node(node->wildcard_type);
                                 default_visitor::visit_type_argument(node);
                               }

                               virtual void visit_type_argument_type(type_argument_type_ast *node)
                               {
                                 handle_ast_node(node->class_type);
                                 handle_ast_node(node->mandatory_array_builtin_type);
                                 default_visitor::visit_type_argument_type(node);
                               }

                               virtual void visit_type_arguments(type_arguments_ast *node)
                               {
                                 {
                                   type_argument_ast *e = 0;
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
                                 handle_ast_node(node->class_declaration);
                                 handle_ast_node(node->enum_declaration);
                                 handle_ast_node(node->interface_declaration);
                                 handle_ast_node(node->annotation_type_declaration);
                                 default_visitor::visit_type_declaration(node);
                               }

                               virtual void visit_type_parameter(type_parameter_ast *node)
                               {
                                 handle_ast_node(node->identifier);
                                 {
                                   class_or_interface_type_name_ast *e = 0;
                                   handle_list_node(node->extends_type_sequence, e);
                                 }
                                 default_visitor::visit_type_parameter(node);
                               }

                               virtual void visit_type_parameters(type_parameters_ast *node)
                               {
                                 {
                                   type_parameter_ast *e = 0;
                                   handle_list_node(node->type_parameter_sequence, e);
                                 }
                                 default_visitor::visit_type_parameters(node);
                               }

                               virtual void visit_unary_expression(unary_expression_ast *node)
                               {
                                 handle_variable(&node->rule_type);
                                 handle_ast_node(node->unary_expression);
                                 handle_ast_node(node->unary_expression_not_plusminus);
                                 default_visitor::visit_unary_expression(node);
                               }

                               virtual void visit_unary_expression_not_plusminus(unary_expression_not_plusminus_ast *node)
                               {
                                 handle_variable(&node->rule_type);
                                 handle_ast_node(node->bitwise_not_expression);
                                 handle_ast_node(node->logical_not_expression);
                                 handle_ast_node(node->cast_expression);
                                 handle_ast_node(node->primary_expression);
                                 {
                                   postfix_operator_ast *e = 0;
                                   handle_list_node(node->postfix_operator_sequence, e);
                                 }
                                 default_visitor::visit_unary_expression_not_plusminus(node);
                               }

                               virtual void visit_variable_array_initializer(variable_array_initializer_ast *node)
                               {
                                 {
                                   variable_initializer_ast *e = 0;
                                   handle_list_node(node->variable_initializer_sequence, e);
                                 }
                                 default_visitor::visit_variable_array_initializer(node);
                               }

                               virtual void visit_variable_declaration(variable_declaration_ast *node)
                               {
                                 handle_ast_node(node->data);
                                 default_visitor::visit_variable_declaration(node);
                               }

                               virtual void visit_variable_declaration_data(variable_declaration_data_ast *node)
                               {
                                 handle_ast_node(node->modifiers);
                                 handle_ast_node(node->type);
                                 {
                                   variable_declarator_ast *e = 0;
                                   handle_list_node(node->declarator_sequence, e);
                                 }
                                 default_visitor::visit_variable_declaration_data(node);
                               }

                               virtual void visit_variable_declaration_rest(variable_declaration_rest_ast *node)
                               {
                                 handle_ast_node(node->first_initializer);
                                 {
                                   variable_declarator_ast *e = 0;
                                   handle_list_node(node->variable_declarator_sequence, e);
                                 }
                                 default_visitor::visit_variable_declaration_rest(node);
                               }

                               virtual void visit_variable_declaration_split_data(variable_declaration_split_data_ast *node)
                               {
                                 handle_ast_node(node->data);
                                 default_visitor::visit_variable_declaration_split_data(node);
                               }

                               virtual void visit_variable_declaration_statement(variable_declaration_statement_ast *node)
                               {
                                 handle_ast_node(node->variable_declaration);
                                 default_visitor::visit_variable_declaration_statement(node);
                               }

                               virtual void visit_variable_declarator(variable_declarator_ast *node)
                               {
                                 handle_ast_node(node->variable_name);
                                 handle_ast_node(node->declarator_brackets);
                                 handle_ast_node(node->initializer);
                                 default_visitor::visit_variable_declarator(node);
                               }

                               virtual void visit_variable_initializer(variable_initializer_ast *node)
                               {
                                 handle_ast_node(node->expression);
                                 handle_ast_node(node->array_initializer);
                                 default_visitor::visit_variable_initializer(node);
                               }

                               virtual void visit_while_statement(while_statement_ast *node)
                               {
                                 handle_ast_node(node->condition);
                                 handle_ast_node(node->body);
                                 default_visitor::visit_while_statement(node);
                               }

                               virtual void visit_wildcard_type(wildcard_type_ast *node)
                               {
                                 handle_ast_node(node->bounds);
                                 default_visitor::visit_wildcard_type(node);
                               }

                               virtual void visit_wildcard_type_bounds(wildcard_type_bounds_ast *node)
                               {
                                 handle_variable(&node->extends_or_super);
                                 handle_ast_node(node->type);
                                 default_visitor::visit_wildcard_type_bounds(node);
                               }

                             };

} // end of namespace java

#endif


