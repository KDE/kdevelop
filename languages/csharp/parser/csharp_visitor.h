// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_VISITOR_H_INCLUDED
#define csharp_VISITOR_H_INCLUDED

#include "csharp_ast.h"

namespace csharp
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
      virtual void visit_event_accessor_declaration(event_accessor_declaration_ast *)
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
      virtual void visit_optional_parameter_modifier(optional_parameter_modifier_ast *)
      {}
      virtual void visit_optionally_nullable_type(optionally_nullable_type_ast *)
      {}
      virtual void visit_overloadable_binary_only_operator(overloadable_binary_only_operator_ast *)
      {}
      virtual void visit_overloadable_unary_only_operator(overloadable_unary_only_operator_ast *)
      {}
      virtual void visit_overloadable_unary_or_binary_operator(overloadable_unary_or_binary_operator_ast *)
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
                                                        virtual void visit_secondary_constraint(secondary_constraint_ast *)
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
                                                        virtual void visit_using_alias_directive_data(using_alias_directive_data_ast *)
                                                        {}
                                                        virtual void visit_using_directive(using_directive_ast *)
                                                        {}
                                                        virtual void visit_using_namespace_directive_data(using_namespace_directive_data_ast *)
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

} // end of namespace csharp

#endif


