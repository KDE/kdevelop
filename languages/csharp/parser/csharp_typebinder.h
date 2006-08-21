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

#ifndef CSHARP_TYPEBINDER_H
#define CSHARP_TYPEBINDER_H

#include "csharp_default_visitor.h"
#include "csharp_codemodel.h"
#include "decoder.h"
#include "csharp_parser.h" // for parser::token_stream_type

#include <kurl.h>


namespace csharp
{

/**
 * A class which extracts type information from the AST after the codemodel
 * has been generated.
 */
class TypeBinder: protected default_visitor
{
public:
    TypeBinder( CodeModel *model,
                parser::token_stream_type *tokenStream );
    virtual ~TypeBinder();

    TypeModelItem createType( const QString &url, ScopeModelItem scope, ast_node *node );

protected:
    inline CodeModel *model() const { return _M_model; }
    inline TypeModelItem currentType() { return _M_currentType; }
    inline TypePartModelItem currentTypePart() { return _M_currentTypePart; }

    virtual void visit_return_type( return_type_ast *node );
    virtual void visit_unmanaged_type( unmanaged_type_ast *node );
    virtual void visit_pointer_type( pointer_type_ast *node );
    virtual void visit_unmanaged_type_suffix( unmanaged_type_suffix_ast *node );
    virtual void visit_rank_specifier( rank_specifier_ast *node );
    virtual void visit_builtin_class_type( builtin_class_type_ast *node );
    virtual void visit_optionally_nullable_type( optionally_nullable_type_ast *node );
    virtual void visit_simple_type( simple_type_ast *node );
    virtual void visit_numeric_type( numeric_type_ast *node );
    virtual void visit_integral_type( integral_type_ast *node );
    virtual void visit_floating_point_type( floating_point_type_ast *node );
    virtual void visit_namespace_or_type_name( namespace_or_type_name_ast *node );
    virtual void visit_namespace_or_type_name_safe( namespace_or_type_name_safe_ast *node );
    virtual void visit_namespace_or_type_name_part( namespace_or_type_name_part_ast *node );
    virtual void visit_type_arguments( type_arguments_ast *node );

private:
    std::string decode_string( std::size_t index ) const;

    TypeModelItem descendIntoType( ast_node *node );
    TypeModelItem changeCurrentType( TypeModelItem item );
    TypePartModelItem changeCurrentTypePart( TypePartModelItem part );
    bool changeIsInitialTypePart( bool is );

    void setPositionAt( _CodeModelItem *item, ast_node *ast );

private:
    CodeModel *_M_model;
    parser::token_stream_type *_M_tokenStream;
    decoder _M_decoder;
    QString _M_currentFile;
    ScopeModelItem _M_currentScope;

    TypeModelItem _M_currentType;
    TypePartModelItem _M_currentTypePart;

    bool _M_isInitialTypePart;
};

} // end of namespace csharp

#endif // CSHARP_TYPEBINDER_H

// kate: space-indent on; indent-width 4; replace-tabs on;
