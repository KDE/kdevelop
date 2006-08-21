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
#include "csharp_typebinder.h"
#include "csharp_codemodel.h"
#include "csharp_codemodel_chameleon.h"
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
    Binder( CodeModel *model,
            parser::token_stream_type *tokenStream );
    virtual ~Binder();

    void run( const KUrl &url, compilation_unit_ast *node );

protected:
    inline CodeModel *model() const { return _M_model; }
    inline TypeBinder *typeBinder() const { return _M_typeBinder; }

    // declarations
    virtual void visit_namespace_declaration( namespace_declaration_ast *node );
    virtual void visit_qualified_identifier( qualified_identifier_ast *node );
    virtual void visit_class_declaration( class_declaration_ast *node );
    virtual void visit_optional_modifiers( optional_modifiers_ast *node );
    virtual void visit_struct_declaration( struct_declaration_ast *node );
    virtual void visit_interface_declaration( interface_declaration_ast *node );
    virtual void visit_delegate_declaration( delegate_declaration_ast *node );
    virtual void visit_enum_declaration( enum_declaration_ast *node );
    virtual void visit_enum_member_declaration( enum_member_declaration_ast *node );
    virtual void visit_formal_parameter( formal_parameter_ast *node );
    virtual void visit_method_declaration( method_declaration_ast *node );
    virtual void visit_interface_method_declaration( interface_method_declaration_ast *node );
    virtual void visit_variable_declaration_data( variable_declaration_data_ast *node );
    virtual void visit_variable_declarator( variable_declarator_ast *node );

    /*
    virtual void visit_accessor_declarations( accessor_declarations_ast *node );
    virtual void visit_accessor_modifier( accessor_modifier_ast *node );
    virtual void visit_attribute( attribute_ast *node );
    virtual void visit_attribute_arguments( attribute_arguments_ast *node );
    virtual void visit_attribute_section( attribute_section_ast *node );
    virtual void visit_attribute_target( attribute_target_ast *node );
    virtual void visit_block( block_ast *node );
    virtual void visit_class_base( class_base_ast *node );
    virtual void visit_constant_declaration_data( constant_declaration_data_ast *node );
    virtual void visit_constant_declarator( constant_declarator_ast *node );
    virtual void visit_constructor_constraint( constructor_constraint_ast *node );
    virtual void visit_constructor_declaration( constructor_declaration_ast *node );
    virtual void visit_conversion_operator_declaration( conversion_operator_declaration_ast *node );
    virtual void visit_enum_base( enum_base_ast *node );
    virtual void visit_event_accessor_declaration( event_accessor_declaration_ast *node );
    virtual void visit_event_accessor_declarations( event_accessor_declarations_ast *node );
    virtual void visit_event_declaration( event_declaration_ast *node );
    virtual void visit_extern_alias_directive( extern_alias_directive_ast *node );
    virtual void visit_finalizer_declaration( finalizer_declaration_ast *node );
    virtual void visit_global_attribute_section( global_attribute_section_ast *node );
    virtual void visit_indexer_declaration( indexer_declaration_ast *node );
    virtual void visit_interface_accessors( interface_accessors_ast *node );
    virtual void visit_interface_base( interface_base_ast *node );
    virtual void visit_interface_event_declaration( interface_event_declaration_ast *node );
    virtual void visit_interface_indexer_declaration( interface_indexer_declaration_ast *node );
    virtual void visit_interface_member_declaration( interface_member_declaration_ast *node );
    virtual void visit_interface_property_declaration( interface_property_declaration_ast *node );
    virtual void visit_namespace_or_type_name_part( namespace_or_type_name_part_ast *node );
    virtual void visit_optional_parameter_modifier( optional_parameter_modifier_ast *node );
    virtual void visit_primary_or_secondary_constraint( primary_or_secondary_constraint_ast *node );
    virtual void visit_property_declaration( property_declaration_ast *node );
    virtual void visit_rank_specifier( rank_specifier_ast *node );
    virtual void visit_secondary_constraint( secondary_constraint_ast *node );
    virtual void visit_struct_member_declaration( struct_member_declaration_ast *node );
    virtual void visit_type_parameter( type_parameter_ast *node );
    virtual void visit_type_parameter_constraints( type_parameter_constraints_ast *node );
    virtual void visit_type_parameter_constraints_clause( type_parameter_constraints_clause_ast *node );
    virtual void visit_type_parameters( type_parameters_ast *node );
    virtual void visit_unary_or_binary_operator_declaration( unary_or_binary_operator_declaration_ast *node );
    virtual void visit_using_alias_directive_data( using_alias_directive_data_ast *node );
    virtual void visit_using_namespace_directive_data( using_namespace_directive_data_ast *node );
    */

private:
    std::string decode_string( std::size_t index ) const;

    ScopeModelItem changeCurrentScope( ScopeModelItem item );

    void setModifiers( ModelItemChameleon item, uint modifiers );
    void setAccessPolicy( ModelItemChameleon item, access_policy::access_policy_enum accessPolicy );
    TypeModelItem createType( ast_node *node );

    void setPositionAt( _CodeModelItem *item, ast_node *ast );

private:
    // will be generated as pre-defined text
    CodeModel *_M_model;
    parser::token_stream_type *_M_tokenStream;
    TypeBinder *_M_typeBinder;
    decoder _M_decoder;
    QString _M_currentFile;
    ScopeModelItem _M_currentScope;

    // will be generated depending on the codemodel, or inserted manually, or whatever
    access_policy::access_policy_enum _M_currentAccessPolicy;
    uint _M_currentModifiers;
    ParameterList _M_currentParameters;
    TypeModelItem _M_currentType;
    QStringList _M_additionalContext;
};

} // end of namespace csharp

#endif // BINDER_H

// kate: space-indent on; indent-width 4; replace-tabs on;
