/* This file is part of KDevelop
   Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 Jakob Petsovits <jpetso@gmx.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CSHARP_BINDER_H
#define CSHARP_BINDER_H

#include "csharp_default_visitor.h"
#include "csharp_codemodel.h"
#include "decoder.h"
#include "csharp_parser.h" // for parser::token_stream_type

#include <QtCore/QSet>

#include <kurl.h>


namespace csharp
{

/**
 * A class which iterates the AST to find all declarations and bind them
 * to the codemodel.
 */
class Binder: protected default_visitor
{
public:
  Binder(CodeModel *model,
         parser::token_stream_type *token_stream);
  virtual ~Binder();

  void run(const KUrl &url, compilation_unit_ast *node);

protected:
  inline CodeModel *model() const { return _M_model; }

  // declarations
  virtual void visit_namespace_declaration(namespace_declaration_ast *node);
  virtual void visit_qualified_identifier(qualified_identifier_ast *node);
  virtual void visit_optional_modifiers(optional_modifiers_ast *node);

  /*
  virtual void visit_accessor_declarations(accessor_declarations_ast *node);
  virtual void visit_accessor_modifier(accessor_modifier_ast *node);
  virtual void visit_attribute(attribute_ast *node);
  virtual void visit_attribute_arguments(attribute_arguments_ast *node);
  virtual void visit_attribute_section(attribute_section_ast *node);
  virtual void visit_attribute_target(attribute_target_ast *node);
  virtual void visit_block(block_ast *node);
  virtual void visit_class_base(class_base_ast *node);
  virtual void visit_class_declaration(class_declaration_ast *node);
  virtual void visit_constant_declaration_data(constant_declaration_data_ast *node);
  virtual void visit_constant_declarator(constant_declarator_ast *node);
  virtual void visit_constructor_constraint(constructor_constraint_ast *node);
  virtual void visit_constructor_declaration(constructor_declaration_ast *node);
  virtual void visit_conversion_operator_declaration(conversion_operator_declaration_ast *node);
  virtual void visit_delegate_declaration(delegate_declaration_ast *node);
  virtual void visit_enum_base(enum_base_ast *node);
  virtual void visit_enum_declaration(enum_declaration_ast *node);
  virtual void visit_enum_member_declaration(enum_member_declaration_ast *node);
  virtual void visit_event_accessor_declaration(event_accessor_declaration_ast *node);
  virtual void visit_event_accessor_declarations(event_accessor_declarations_ast *node);
  virtual void visit_event_declaration(event_declaration_ast *node);
  virtual void visit_extern_alias_directive(extern_alias_directive_ast *node);
  virtual void visit_finalizer_declaration(finalizer_declaration_ast *node);
  virtual void visit_formal_parameter(formal_parameter_ast *node);
  virtual void visit_global_attribute_section(global_attribute_section_ast *node);
  virtual void visit_indexer_declaration(indexer_declaration_ast *node);
  virtual void visit_interface_accessors(interface_accessors_ast *node);
  virtual void visit_interface_base(interface_base_ast *node);
  virtual void visit_interface_declaration(interface_declaration_ast *node);
  virtual void visit_interface_event_declaration(interface_event_declaration_ast *node);
  virtual void visit_interface_indexer_declaration(interface_indexer_declaration_ast *node);
  virtual void visit_interface_member_declaration(interface_member_declaration_ast *node);
  virtual void visit_interface_method_declaration(interface_method_declaration_ast *node);
  virtual void visit_interface_property_declaration(interface_property_declaration_ast *node);
  virtual void visit_method_declaration(method_declaration_ast *node);
  virtual void visit_namespace_or_type_name_part(namespace_or_type_name_part_ast *node);
  virtual void visit_optional_parameter_modifier(optional_parameter_modifier_ast *node);
  virtual void visit_primary_or_secondary_constraint(primary_or_secondary_constraint_ast *node);
  virtual void visit_property_declaration(property_declaration_ast *node);
  virtual void visit_rank_specifier(rank_specifier_ast *node);
  virtual void visit_secondary_constraint(secondary_constraint_ast *node);
  virtual void visit_struct_declaration(struct_declaration_ast *node);
  virtual void visit_struct_member_declaration(struct_member_declaration_ast *node);
  virtual void visit_type_arguments(type_arguments_ast *node);
  virtual void visit_type_arguments_or_parameters_end(type_arguments_or_parameters_end_ast *node);
  virtual void visit_type_parameter(type_parameter_ast *node);
  virtual void visit_type_parameter_constraints(type_parameter_constraints_ast *node);
  virtual void visit_type_parameter_constraints_clause(type_parameter_constraints_clause_ast *node);
  virtual void visit_type_parameters(type_parameters_ast *node);
  virtual void visit_unary_or_binary_operator_declaration(unary_or_binary_operator_declaration_ast *node);
  virtual void visit_using_alias_directive_data(using_alias_directive_data_ast *node);
  virtual void visit_using_namespace_directive_data(using_namespace_directive_data_ast *node);
  virtual void visit_variable_declaration_data(variable_declaration_data_ast *node);
  virtual void visit_variable_declarator(variable_declarator_ast *node);

  // types
  virtual void visit_return_type(return_type_ast *node);
  virtual void visit_type(type_ast *node);
  virtual void visit_unmanaged_type(unmanaged_type_ast *node);
  virtual void visit_unmanaged_type_suffix(unmanaged_type_suffix_ast *node);
  virtual void visit_managed_type(managed_type_ast *node);
  virtual void visit_array_type(array_type_ast *node);
  virtual void visit_simple_type(simple_type_ast *node);
  virtual void visit_class_type(class_type_ast *node);
  virtual void visit_builtin_class_type(builtin_class_type_ast *node);
  virtual void visit_fixed_pointer_declarator(fixed_pointer_declarator_ast *node);
  virtual void visit_floating_point_type(floating_point_type_ast *node);
  virtual void visit_integral_type(integral_type_ast *node);
  virtual void visit_non_array_type(non_array_type_ast *node);
  virtual void visit_non_nullable_type(non_nullable_type_ast *node);
  virtual void visit_numeric_type(numeric_type_ast *node);
  virtual void visit_optionally_nullable_type(optionally_nullable_type_ast *node);
  virtual void visit_pointer_type(pointer_type_ast *node);
  virtual void visit_predefined_type(predefined_type_ast *node);
  */

private:
  std::string decode_string(std::size_t index) const;

  QStringList changeCurrentContext(const QStringList& context);
  bool changeEditContext(bool edit);

  ScopeModelItem changeCurrentScope(ScopeModelItem scope);
  access_policy::access_policy_enum changeCurrentAccess(access_policy::access_policy_enum accessPolicy);
  NamespaceDeclarationModelItem changeCurrentNamespace(NamespaceDeclarationModelItem item);

  void setPositionAt(_CodeModelItem *item, ast_node *ast);

private:
  CodeModel *_M_model;
  parser::token_stream_type *_M_token_stream;
  decoder _M_decoder;
  Lexer *_M_lexer;
  GlobalNamespaceDeclarationModelItem _M_globalNamespace;
  QString _M_currentFile;

  QStringList _M_currentContext;
  bool _M_editContext;

  ScopeModelItem _M_currentScope;
  access_policy::access_policy_enum _M_currentAccess;
  NamespaceDeclarationModelItem _M_currentNamespace;
};

} // end of namespace csharp

#endif // BINDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
