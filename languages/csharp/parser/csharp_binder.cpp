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

#include "csharp_binder.h"


namespace csharp
{

Binder::Binder( CodeModel *model, parser::token_stream_type *tokenStream )
    : _M_model( model ), _M_tokenStream( tokenStream ),
      _M_typeBinder( new TypeBinder(_M_model, _M_tokenStream) ),
      _M_decoder( _M_tokenStream )
{
}

Binder::~Binder()
{
    delete _M_typeBinder;
}

void Binder::run( const KUrl &url, compilation_unit_ast *node )
{
    _M_currentFile = url.url();

    _M_currentAccessPolicy = access_policy::access_private;
    _M_currentModifiers = 0;

    changeCurrentScope( model_static_cast<ScopeModelItem>(model()->globalNamespace()) );
    visit_node(node);
}


ScopeModelItem Binder::changeCurrentScope( ScopeModelItem item )
{
    ScopeModelItem old = _M_currentScope;
    _M_currentScope = item;
    return old;
}


void Binder::visit_namespace_declaration( namespace_declaration_ast *node )
{
    NamespaceDeclarationModelItem newNamespaceDeclaration =
            model()->create<NamespaceDeclarationModelItem>();
    newNamespaceDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newNamespaceDeclaration, node );

    _M_additionalContext.clear();
    visit_node( node->name );

    NamespaceDeclarationModelItem parentNamespace =
            model_static_cast<NamespaceDeclarationModelItem>( _M_currentScope );

    while (_M_additionalContext.count() > 1)
    {
        QString firstName = _M_additionalContext.takeFirst();
        NamespaceDeclarationModelItem currentNamespace =
                parentNamespace->findNamespace( firstName );

        if (!currentNamespace)
        {
            currentNamespace = model()->create<NamespaceDeclarationModelItem>();
            currentNamespace->setScope( parentNamespace->context() );
            currentNamespace->setName( firstName );
            setPositionAt( currentNamespace, node );
            parentNamespace->addNamespace( currentNamespace );
        }
        parentNamespace = currentNamespace;
    }

    // set up the originally created namespace with the last remaining context name
    newNamespaceDeclaration->setScope( parentNamespace->context() );
    newNamespaceDeclaration->setName( _M_additionalContext.takeFirst() );
    parentNamespace->addNamespace( newNamespaceDeclaration );

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newNamespaceDeclaration) );
    visit_node( node->body );
    changeCurrentScope( oldScope );
}

void Binder::visit_qualified_identifier( qualified_identifier_ast *node )
{
    if (node->name_sequence)
    {
        const list_node<identifier_ast*> *__it = node->name_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );

            identifier_ast *name = __it->element;
            _M_additionalContext.append( decode_string(name->ident).c_str() );

            __it = __it->next;
        }
        while (__it != __end);
    }
}

void Binder::visit_class_declaration( class_declaration_ast *node )
{
    ClassDeclarationModelItem newClassDeclaration =
            model()->create<ClassDeclarationModelItem>();
    newClassDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newClassDeclaration, node );

    visit_node( node->attributes );
    visit_node( node->modifiers );
    visit_node( node->class_name );
    visit_node( node->type_parameters );
    visit_node( node->class_base );

    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while (__it != __end);
    }

    newClassDeclaration->setName( decode_string(node->class_name->ident).c_str() );

    ModelItemChameleon scopeChameleon( _M_currentScope );

    if ( node->partial )
    {
        Nullable<ClassDeclarationModelItem> existingClassDeclaration =
                scopeChameleon->findClass( newClassDeclaration->name() );

        if (!existingClassDeclaration.isNull && existingClassDeclaration.item)
            newClassDeclaration = existingClassDeclaration.item;
    }

    ModelItemChameleon itemChameleon( newClassDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );
    // TODO: class_base
    // TODO: type parameters and constraints

    scopeChameleon->addClass( newClassDeclaration );

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newClassDeclaration) );
    visit_node( node->body );
    changeCurrentScope( oldScope );
}

void Binder::visit_struct_declaration( struct_declaration_ast *node )
{
    StructDeclarationModelItem newStructDeclaration =
            model()->create<StructDeclarationModelItem>();
    newStructDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newStructDeclaration, node );

    visit_node( node->attributes );
    visit_node( node->modifiers );
    visit_node( node->struct_name );
    visit_node( node->type_parameters );
    visit_node( node->struct_interfaces );
    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while (__it != __end);
    }

    newStructDeclaration->setName( decode_string(node->struct_name->ident).c_str() );

    ModelItemChameleon scopeChameleon( _M_currentScope );

    if ( node->partial )
    {
        Nullable<StructDeclarationModelItem> existingStructDeclaration =
                scopeChameleon->findStruct( newStructDeclaration->name() );

        if (!existingStructDeclaration.isNull && existingStructDeclaration.item)
            newStructDeclaration = existingStructDeclaration.item;
    }

    ModelItemChameleon itemChameleon( newStructDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );
    // TODO: struct_interfaces
    // TODO: type parameters and constraints

    scopeChameleon->addStruct( newStructDeclaration );

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newStructDeclaration) );
    visit_node( node->body );
    changeCurrentScope( oldScope );
}

void Binder::visit_interface_declaration( interface_declaration_ast *node )
{
    InterfaceDeclarationModelItem newInterfaceDeclaration =
            model()->create<InterfaceDeclarationModelItem>();
    newInterfaceDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newInterfaceDeclaration, node );

    visit_node( node->attributes );
    visit_node( node->modifiers );
    visit_node( node->interface_name );
    visit_node( node->type_parameters );
    visit_node( node->interface_base );
    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while (__it != __end);
    }

    newInterfaceDeclaration->setName( decode_string(node->interface_name->ident).c_str() );

    ModelItemChameleon scopeChameleon( _M_currentScope );

    if ( node->partial )
    {
        Nullable<InterfaceDeclarationModelItem> existingInterfaceDeclaration =
                scopeChameleon->findInterface( newInterfaceDeclaration->name() );

        if (!existingInterfaceDeclaration.isNull && existingInterfaceDeclaration.item)
            newInterfaceDeclaration = existingInterfaceDeclaration.item;
    }

    ModelItemChameleon itemChameleon( newInterfaceDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );
    // TODO: interface_base
    // TODO: type parameters and constraints

    scopeChameleon->addInterface( newInterfaceDeclaration );

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newInterfaceDeclaration) );
    visit_node( node->body );
    changeCurrentScope( oldScope );
}

void Binder::visit_delegate_declaration( delegate_declaration_ast *node )
{
    DelegateDeclarationModelItem newDelegateDeclaration =
            model()->create<DelegateDeclarationModelItem>();
    newDelegateDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newDelegateDeclaration, node );

    _M_currentParameters.clear();

    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->return_type);
    visit_node(node->delegate_name);
    visit_node(node->type_parameters);
    visit_node(node->formal_parameters);
    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while (__it != __end);
    }

    newDelegateDeclaration->setName( decode_string(node->delegate_name->ident).c_str() );

    ModelItemChameleon itemChameleon( newDelegateDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );
    newDelegateDeclaration->setReturnType( createType(node->return_type) );

    foreach ( ParameterModelItem param, _M_currentParameters )
        newDelegateDeclaration->addParameter( param );

    // TODO: type parameters and constraints

    ModelItemChameleon scopeChameleon( _M_currentScope );
    scopeChameleon->addDelegate( newDelegateDeclaration );
}

void Binder::visit_enum_declaration( enum_declaration_ast *node )
{
    EnumDeclarationModelItem newEnumDeclaration =
            model()->create<EnumDeclarationModelItem>();
    newEnumDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newEnumDeclaration, node );

    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->enum_name);
    visit_node(node->enum_base);

    newEnumDeclaration->setName( decode_string(node->enum_name->ident).c_str() );

    ModelItemChameleon itemChameleon( newEnumDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );

    if ( node->enum_base )
        newEnumDeclaration->setBaseIntegralType( createType(node->enum_base->integral_type) );

    ModelItemChameleon scopeChameleon( _M_currentScope );
    scopeChameleon->addEnum( newEnumDeclaration );

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newEnumDeclaration) );
    visit_node( node->body );
    changeCurrentScope( oldScope );
}

void Binder::visit_method_declaration( method_declaration_ast *node )
{
    MethodDeclarationModelItem newMethodDeclaration =
            model()->create<MethodDeclarationModelItem>();
    newMethodDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newMethodDeclaration, node );

    _M_currentParameters.clear();

    visit_node(node->attributes);
    visit_node(node->modifiers);
    visit_node(node->return_type);
    visit_node(node->method_name);
    visit_node(node->type_parameters);
    visit_node(node->formal_parameters);
    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while ( __it != __end );
    }

    newMethodDeclaration->setName(
            decode_string( node->method_name->type_name->name_part_sequence
                           ->to_back()->element->identifier->ident ).c_str()
    );

    ModelItemChameleon itemChameleon( newMethodDeclaration );
    setAccessPolicy( itemChameleon, _M_currentAccessPolicy );
    setModifiers( itemChameleon, _M_currentModifiers );
    newMethodDeclaration->setReturnType( createType(node->return_type) );

    ModelItemChameleon scopeChameleon( _M_currentScope );
    scopeChameleon->addMethod( newMethodDeclaration );

    foreach ( ParameterModelItem param, _M_currentParameters )
        newMethodDeclaration->addParameter( param );

    // TODO: type parameters and constraints

    ScopeModelItem oldScope =
            changeCurrentScope( model_static_cast<ScopeModelItem>(newMethodDeclaration) );
    visit_node( node->method_body );
    changeCurrentScope( oldScope );
}

void Binder::visit_interface_method_declaration( interface_method_declaration_ast *node )
{
    MethodDeclarationModelItem newMethodDeclaration =
            model()->create<MethodDeclarationModelItem>();
    newMethodDeclaration->setScope( _M_currentScope->context() );
    setPositionAt( newMethodDeclaration, node );

    _M_currentParameters.clear();

    visit_node(node->attributes);
    visit_node(node->return_type);
    visit_node(node->method_name);
    visit_node(node->type_parameters);
    visit_node(node->formal_parameters);
    if (node->type_parameter_constraints_sequence)
    {
        const list_node<type_parameter_constraints_clause_ast*> *__it =
                node->type_parameter_constraints_sequence->to_front(), *__end = __it;
        do
        {
            visit_node( __it->element );
            __it = __it->next;
        }
        while ( __it != __end );
    }

    newMethodDeclaration->setName( decode_string(node->method_name->ident).c_str() );
    newMethodDeclaration->setInterfaceMethodDeclaration( true );

    ModelItemChameleon itemChameleon( newMethodDeclaration );
    newMethodDeclaration->setAccessPolicy( access_policy::access_public );
    newMethodDeclaration->setNew( node->decl_new );
    newMethodDeclaration->setReturnType( createType(node->return_type) );

    ModelItemChameleon scopeChameleon( _M_currentScope );
    scopeChameleon->addMethod( newMethodDeclaration );

    foreach ( ParameterModelItem param, _M_currentParameters )
        newMethodDeclaration->addParameter( param );

    // TODO: type parameters and constraints
}

void Binder::visit_formal_parameter( formal_parameter_ast *node )
{
    ParameterModelItem newParameter =
            model()->create<ParameterModelItem>();
    newParameter->setScope( _M_currentScope->context() );
    setPositionAt( newParameter, node );

    default_visitor::visit_formal_parameter( node );

    newParameter->setName( decode_string(node->variable_name->ident).c_str() );

    if ( node->type )
    {
        newParameter->setType( createType(node->type) );
        newParameter->setArray( false );
        newParameter->setParameterType( node->modifier->parameter_type );
    }
    else if ( node->params_type )
    {
        newParameter->setType( createType(node->params_type) );
        newParameter->setArray( true );
        newParameter->setParameterType( parameter::value_parameter );
    }

    _M_currentParameters.append( newParameter );
}

void Binder::visit_enum_member_declaration( enum_member_declaration_ast *node )
{
    EnumValueModelItem newEnumValue =
            model()->create<EnumValueModelItem>();
    newEnumValue->setScope( _M_currentScope->context() );
    setPositionAt( newEnumValue, node );

    default_visitor::visit_enum_member_declaration( node );

    newEnumValue->setName( decode_string(node->member_name->ident).c_str() );
    // TODO: setValue out of the node->value constant_expression

    ModelItemChameleon scopeChameleon( _M_currentScope );
    scopeChameleon->addEnumValue( newEnumValue );
}

void Binder::visit_optional_modifiers(optional_modifiers_ast *node)
{
    _M_currentAccessPolicy = node->access_policy;
    _M_currentModifiers = node->modifiers;
}

void Binder::setAccessPolicy( ModelItemChameleon item, access_policy::access_policy_enum accessPolicy )
{
    // for partial classes, structs or interfaces,
    // don't change already specified access policies back to the default
    Nullable<access_policy::access_policy_enum> nullableAccessPolicy = item->accessPolicy();

    if (!nullableAccessPolicy.isNull
        && nullableAccessPolicy.item == access_policy::access_private)
    {
        item->setAccessPolicy( accessPolicy );
    }
}

void Binder::setModifiers( ModelItemChameleon item, uint mods )
{
    item->setNew( mods & modifiers::mod_new );
    item->setAbstract( mods & modifiers::mod_abstract );
    item->setSealed( mods & modifiers::mod_sealed );
    item->setStatic( mods & modifiers::mod_static );
    item->setReadonly( mods & modifiers::mod_readonly );
    item->setVolatile( mods & modifiers::mod_volatile );
    item->setVirtual( mods & modifiers::mod_virtual );
    item->setOverride( mods & modifiers::mod_override );
    item->setExtern( mods & modifiers::mod_extern );
    item->setUnsafe( mods & modifiers::mod_unsafe );
}

TypeModelItem Binder::createType( ast_node *node )
{
    return typeBinder()->createType( _M_currentFile, _M_currentScope, node );
}

std::string Binder::decode_string( std::size_t index ) const
{
    return _M_decoder.decode_string( index );
}

void Binder::setPositionAt( _CodeModelItem *item, ast_node *node )
{
    int startLine, startColumn;
    int endLine, endColumn;

    _M_tokenStream->start_position( node->start_token, &startLine, &startColumn );
    _M_tokenStream->end_position( node->end_token, &endLine, &endColumn );

    item->setFileName( _M_currentFile );
    item->setStartPosition( KTextEditor::Cursor(startLine, startColumn) );
    item->setEndPosition( KTextEditor::Cursor(endLine, endColumn) );
}

} // end of namespace csharp

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
