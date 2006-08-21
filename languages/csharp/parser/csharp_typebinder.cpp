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

#include "csharp_typebinder.h"


namespace csharp
{

TypeBinder::TypeBinder( CodeModel *model, parser::token_stream_type *tokenStream )
    : _M_model( model ), _M_tokenStream( tokenStream ), _M_decoder( _M_tokenStream ),
      _M_isInitialTypePart( true )
{
}

TypeBinder::~TypeBinder()
{
}

TypeModelItem TypeBinder::createType( const QString &url, ScopeModelItem scope, ast_node *node )
{
    _M_currentFile = url;
    _M_currentScope = scope;
    return descendIntoType( node );
}

TypeModelItem TypeBinder::descendIntoType( ast_node *node )
{
    TypeModelItem type = model()->create<TypeModelItem>();
    type->setScope( _M_currentScope->context() );
    setPositionAt( type, node );

    TypePartModelItem typePart = model()->create<TypePartModelItem>();
    typePart->setScope( _M_currentScope->context() );
    setPositionAt( typePart, node );

    TypeModelItem oldType = changeCurrentType( type );
    TypePartModelItem oldPart = changeCurrentTypePart( typePart );
    bool oldIsInitialTypePart = changeIsInitialTypePart( true );

    visit_node( node );
    type->addTypePart( typePart );

    changeIsInitialTypePart( oldIsInitialTypePart );
    changeCurrentTypePart( oldPart );
    changeCurrentType( oldType );

    return type;
}

TypeModelItem TypeBinder::changeCurrentType( TypeModelItem item )
{
    TypeModelItem old = _M_currentType;
    _M_currentType = item;
    return old;
}

TypePartModelItem TypeBinder::changeCurrentTypePart( TypePartModelItem part )
{
    TypePartModelItem old = _M_currentTypePart;
    _M_currentTypePart = part;
    return old;
}

bool TypeBinder::changeIsInitialTypePart( bool is )
{
    bool old = _M_isInitialTypePart;
    _M_isInitialTypePart = is;
    return old;
}


void TypeBinder::visit_return_type( return_type_ast *node )
{
    if ( node->type == return_type::type_void )
        currentTypePart()->setName( "void" );

    default_visitor::visit_return_type( node );
}

void TypeBinder::visit_unmanaged_type( unmanaged_type_ast *node )
{
    if ( node->type == pointer_type::type_void_star )
        currentTypePart()->setName( "void*" );

    default_visitor::visit_unmanaged_type( node );
}

void TypeBinder::visit_pointer_type( pointer_type_ast *node )
{
    if ( node->type == pointer_type::type_void_star )
        currentTypePart()->setName( "void*" );

    default_visitor::visit_pointer_type( node );
}

void TypeBinder::visit_unmanaged_type_suffix( unmanaged_type_suffix_ast *node )
{
    if ( node->type == unmanaged_type_suffix::type_star )
        currentTypePart()->setName( currentTypePart()->name() + "*" );

    default_visitor::visit_unmanaged_type_suffix( node );
}

void TypeBinder::visit_rank_specifier( rank_specifier_ast *node )
{
    QString specifierString = "[";

    for ( int i = 0; i < node->dimension_seperator_count; i++ )
        specifierString += ",";

    specifierString += "]";
    currentTypePart()->setName( currentTypePart()->name() + specifierString );

    default_visitor::visit_rank_specifier( node );
}

void TypeBinder::visit_builtin_class_type( builtin_class_type_ast *node )
{
    if ( node->type == builtin_class_type::type_object )
        currentTypePart()->setName( "object" );
    else if ( node->type == builtin_class_type::type_string )
        currentTypePart()->setName( "string" );

    default_visitor::visit_builtin_class_type( node );
}

void TypeBinder::visit_optionally_nullable_type( optionally_nullable_type_ast *node )
{
    default_visitor::visit_optionally_nullable_type( node );

    if ( node->nullable )
        currentTypePart()->setName( currentTypePart()->name() + "?" );
}

void TypeBinder::visit_simple_type( simple_type_ast *node )
{
    if ( node->type == simple_type::type_bool )
        currentTypePart()->setName( "bool" );

    default_visitor::visit_simple_type( node );
}

void TypeBinder::visit_numeric_type( numeric_type_ast *node )
{
    if ( node->type == numeric_type::type_decimal )
        currentTypePart()->setName( "decimal" );

    default_visitor::visit_numeric_type( node );
}

void TypeBinder::visit_integral_type( integral_type_ast *node )
{
    switch ( node->type )
    {
    case integral_type::type_sbyte:
        currentTypePart()->setName( "sbyte" ); break;
    case integral_type::type_byte:
        currentTypePart()->setName( "byte" ); break;
    case integral_type::type_short:
        currentTypePart()->setName( "short" ); break;
    case integral_type::type_ushort:
        currentTypePart()->setName( "ushort" ); break;
    case integral_type::type_int:
        currentTypePart()->setName( "int" ); break;
    case integral_type::type_uint:
        currentTypePart()->setName( "uint" ); break;
    case integral_type::type_long:
        currentTypePart()->setName( "long" ); break;
    case integral_type::type_ulong:
        currentTypePart()->setName( "ulong" ); break;
    case integral_type::type_char:
        currentTypePart()->setName( "char" ); break;
    default:
        Q_ASSERT( false );
    }

    default_visitor::visit_integral_type( node );
}

void TypeBinder::visit_floating_point_type( floating_point_type_ast *node )
{
    if ( node->type == floating_point_type::type_float )
        currentTypePart()->setName( "float" );
    else if ( node->type == floating_point_type::type_double )
        currentTypePart()->setName( "double" );

    default_visitor::visit_floating_point_type( node );
}

void TypeBinder::visit_namespace_or_type_name( namespace_or_type_name_ast *node )
{
    if ( node->qualified_alias_label )
    {
        currentType()->setQualifiedAliasLabel(
                decode_string(node->qualified_alias_label->ident).c_str() );
    }

    default_visitor::visit_namespace_or_type_name( node );
}

void TypeBinder::visit_namespace_or_type_name_safe( namespace_or_type_name_safe_ast *node )
{
    if ( node->qualified_alias_label )
    {
        currentType()->setQualifiedAliasLabel(
                decode_string(node->qualified_alias_label->ident).c_str() );
    }

    default_visitor::visit_namespace_or_type_name_safe( node );
}

void TypeBinder::visit_namespace_or_type_name_part( namespace_or_type_name_part_ast *node )
{
    // write to a new part, but only if it's not the first, already existing one
    if ( _M_isInitialTypePart )
        changeIsInitialTypePart( false );
    else
    {
        currentType()->addTypePart( currentTypePart() );
        changeCurrentTypePart( model()->create<TypePartModelItem>() );
        currentTypePart()->setScope( _M_currentScope->context() );
    }

    setPositionAt( currentTypePart(), node );
    currentTypePart()->setName( decode_string(node->identifier->ident).c_str() );
    default_visitor::visit_namespace_or_type_name_part( node );
}

void TypeBinder::visit_type_arguments( type_arguments_ast *node )
{
    if ( node->type_argument_sequence )
    {
        const list_node<type_ast*> *__it = node->type_argument_sequence->to_front(), *__end = __it;
        do
        {
            currentTypePart()->addTypeArgument( descendIntoType(__it->element) );
            __it = __it->next;
        }
        while ( __it != __end );
    }
}


std::string TypeBinder::decode_string( std::size_t index ) const
{
    return _M_decoder.decode_string( index );
}

void TypeBinder::setPositionAt( _CodeModelItem *item, ast_node *node )
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
