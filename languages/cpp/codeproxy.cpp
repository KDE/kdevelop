/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "cpplanguagesupport.h"
#include "parser/codemodel.h"

#include <kdebug.h>

CodeProxy::CodeProxy( QObject* parent )
        : KDevCodeProxy( parent ),
        m_kindFilter( 0 )
{
    QMap<QString, int> filter;
    filter.insert( "Namespace", _CodeModelItem::Kind_Namespace );
    filter.insert( "Class", _CodeModelItem::Kind_Class );
    filter.insert( "Enum", _CodeModelItem::Kind_Enum );
    filter.insert( "Enumerator", _CodeModelItem::Kind_Enumerator );
    filter.insert( "Function", _CodeModelItem::Kind_Function );
    filter.insert( "Function Def.", _CodeModelItem::Kind_FunctionDefinition );
    filter.insert( "Argument", _CodeModelItem::Kind_Argument );
    filter.insert( "Template", _CodeModelItem::Kind_Template );
    filter.insert( "Template Param.", _CodeModelItem::Kind_TemplateParameter );
    filter.insert( "Type Alias", _CodeModelItem::Kind_TypeAlias );
    filter.insert( "Variable", _CodeModelItem::Kind_Variable );
    setKindFilterList( filter );

    m_sortKind.append( _CodeModelItem::Kind_Namespace );
    m_sortKind.append( _CodeModelItem::Kind_Class );
    m_sortKind.append( _CodeModelItem::Kind_Enum );
    m_sortKind.append( _CodeModelItem::Kind_File );
    m_sortKind.append( _CodeModelItem::Kind_Scope );
    m_sortKind.append( _CodeModelItem::Kind_Member );
    m_sortKind.append( _CodeModelItem::Kind_Function );
    m_sortKind.append( _CodeModelItem::Kind_FunctionDefinition );
    m_sortKind.append( _CodeModelItem::Kind_Argument );
    m_sortKind.append( _CodeModelItem::Kind_Enumerator );
    m_sortKind.append( _CodeModelItem::Kind_Template );
    m_sortKind.append( _CodeModelItem::Kind_TemplateParameter );
    m_sortKind.append( _CodeModelItem::Kind_TypeAlias );
    m_sortKind.append( _CodeModelItem::Kind_Variable );
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
    int function = _CodeModelItem::Kind_Function |
                   _CodeModelItem::Kind_FunctionDefinition;

    if ( ( function & leftKind ) == leftKind &&
            ( function & rightKind ) == rightKind )
    {
        //!!!Special case for functions and definitions
        return functionLessThan( left, right );
    }
    else if ( leftKind == rightKind )
        return KDevCodeProxy::lessThan( left, right );
    else
        return m_sortKind.indexOf( leftKind ) < m_sortKind.indexOf( rightKind );
}

bool CodeProxy::functionLessThan( const QModelIndex &left,
                                  const QModelIndex &right ) const
{
    _FunctionModelItem * leftItem =
        dynamic_cast<_FunctionModelItem*>( sourceToItem( left ) );
    _FunctionModelItem * rightItem =
        dynamic_cast<_FunctionModelItem*>( sourceToItem( right ) );
    Q_ASSERT( leftItem && rightItem );

    if ( leftItem->isConstructor() &&
            !rightItem->isConstructor() )
    {
        return true;
    }
    else if ( leftItem->isDestructor() &&
              !rightItem->isDestructor() &&
              !rightItem->isConstructor() )
    {
        return true;
    }
    if ( rightItem->isConstructor() &&
            !leftItem->isConstructor() )
    {
        return false;
    }
    else if ( rightItem->isDestructor() &&
              !leftItem->isDestructor() &&
              !leftItem->isConstructor() )
    {
        return false;
    }
    return KDevCodeProxy::lessThan( left, right );
}

void CodeProxy::setKindFilter( int kind )
{
    m_kindFilter = kind;
    clear();
}

#include "codeproxy.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
