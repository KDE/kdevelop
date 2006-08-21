/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2006 Jakob Petsovits <jpetso@gmx.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "codeproxy.h"

#include "csharplanguagesupport.h"
#include "parser/csharp_codemodel.h"

#include <kdebug.h>

namespace csharp
{

CodeProxy::CodeProxy( QObject* parent )
        : KDevCodeProxy( parent ),
        m_kindFilter( 0 )
{
    QMap<QString, int> filter;
    filter.insert( "Namespace", _CodeModelItem::Kind_NamespaceDeclaration );
    filter.insert( "Class", _CodeModelItem::Kind_ClassDeclaration );
    filter.insert( "Struct", _CodeModelItem::Kind_StructDeclaration );
    filter.insert( "Interface", _CodeModelItem::Kind_InterfaceDeclaration );
    filter.insert( "Enum", _CodeModelItem::Kind_EnumDeclaration );
    filter.insert( "Delegate", _CodeModelItem::Kind_DelegateDeclaration );
    filter.insert( "Method", _CodeModelItem::Kind_MethodDeclaration );
    filter.insert( "Variable", _CodeModelItem::Kind_VariableDeclaration );
    setKindFilterList( filter );

    m_sortKind.append( _CodeModelItem::Kind_Scope );
    m_sortKind.append( _CodeModelItem::Kind_NamespaceDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_TypeDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_ClassLikeDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_GlobalNamespaceDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_ExternAliasDirective );
    m_sortKind.append( _CodeModelItem::Kind_UsingAliasDirective );
    m_sortKind.append( _CodeModelItem::Kind_UsingNamespaceDirective );
    m_sortKind.append( _CodeModelItem::Kind_ClassDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_StructDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_InterfaceDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_DelegateDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_EnumDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_EnumValue );
    m_sortKind.append( _CodeModelItem::Kind_EventDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_EventAccessorDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_IndexerDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_PropertyDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_AccessorDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_MethodDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_VariableDeclaration );
    m_sortKind.append( _CodeModelItem::Kind_Parameter );
    m_sortKind.append( _CodeModelItem::Kind_TypeParameter );
    m_sortKind.append( _CodeModelItem::Kind_TypeParameterConstraint );
    m_sortKind.append( _CodeModelItem::Kind_PrimaryOrSecondaryConstraint );
    m_sortKind.append( _CodeModelItem::Kind_ConstructorConstraint );
    m_sortKind.append( _CodeModelItem::Kind_TypePart );
    m_sortKind.append( _CodeModelItem::Kind_Type );
    m_sortKind.append( _CodeModelItem::FirstKind );
    m_sortKind.append( _CodeModelItem::KindMask );
}

CodeProxy::~CodeProxy()
{}

bool CodeProxy::filterAcceptsRow( int source_row,
                                  const QModelIndex &source_parent ) const
{
    bool exclude = false;
    if ( m_kindFilter )
    {
        QModelIndex source_index =
            sourceModel() ->index( source_row, 0, source_parent );
        Q_ASSERT( source_index.isValid() );

        KDevCodeItem *codeItem = sourceToItem( source_index );
        exclude = ( ( m_kindFilter & codeItem->kind() ) == codeItem->kind() );
    }

    exclude = KDevCodeProxy::filterAcceptsRow( source_row, source_parent ) ?
              exclude : false;
    return !exclude;
}

bool CodeProxy::lessThan( const QModelIndex &left,
                          const QModelIndex &right ) const
{
    KDevCodeItem * leftItem = sourceToItem( left );
    KDevCodeItem * rightItem = sourceToItem( right );
    Q_ASSERT( leftItem && rightItem );

    int leftKind = leftItem->kind();
    int rightKind = rightItem->kind();

    if ( leftKind == rightKind )
        return KDevCodeProxy::lessThan( left, right );
    else
        return m_sortKind.indexOf( leftKind ) < m_sortKind.indexOf( rightKind );
}

void CodeProxy::setKindFilter( int kind )
{
    m_kindFilter = kind;
    clear();
}

} // end of namespace csharp

#include "codeproxy.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
