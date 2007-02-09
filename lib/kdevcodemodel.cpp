/*
 * This file is part of KDevelop
 *
 * Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
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

#include "kdevcodemodel.h"

#include <QMutexLocker>

#include <ktexteditor/smartrange.h>

namespace KDevelop
{

CodeItem::CodeItem( const QString &name, ItemGroup *parent )
        : ItemCollection( name, parent )
        , m_declaration(0L)
        , m_definition(0L)
{}

CodeItem::~CodeItem()
{
    qDeleteAll(m_references);
    delete m_declaration;
    delete m_definition;
}

CodeItem *CodeItem::itemAt( int index ) const
{
    return static_cast<CodeItem*>( ItemCollection::itemAt( index ) );
}

CodeModel::CodeModel( QObject *parent )
        : ItemModel( parent )
{}

CodeModel::~CodeModel()
{}

QVariant CodeModel::data( const QModelIndex &index, int role ) const
{
    if ( CodeItem * code_item = item( index ) )
    {
        switch ( role )
        {
        case Qt::DisplayRole:
            return code_item->display();
        case Qt::DecorationRole:
            return code_item->decoration();
        case Qt::ToolTipRole:
            return code_item->toolTip();
        case Qt::WhatsThisRole:
            return code_item->whatsThis();
        case Qt::UserRole:
            return code_item->kind();
        default:
            break;
        }
    }

    return QVariant();
}

void CodeModel::beginAppendItem( CodeItem *item,
                                     ItemCollection *collection )
{
    QMutexLocker locker( &m_mutex );
    Q_ASSERT( item != root() );
    QModelIndex parent;

    if ( !collection )
        collection = root();

    parent = indexOf( collection );

    int row = collection->itemCount();

    beginInsertRows( parent, row, row );
    collection->add
    ( item );
}

void CodeModel::endAppendItem()
{
    QMutexLocker locker( &m_mutex );
    endInsertRows();
}

void CodeModel::beginRemoveItem( CodeItem *item )
{
    QMutexLocker locker( &m_mutex );
    Q_ASSERT( item != 0 && item->parent() != 0 );
    Q_ASSERT( item->parent() ->collection() != 0 );

    ItemCollection *parent_collection = item->parent() ->collection();

    int row = positionOf( item );

    beginRemoveRows( indexOf( parent_collection ), row, row );
    parent_collection->remove
    ( row );
}

void CodeModel::endRemoveItem()
{
    QMutexLocker locker( &m_mutex );
    endRemoveRows();
}

CodeItem *CodeModel::item( const QModelIndex &index ) const
{
    return reinterpret_cast<CodeItem*>( ItemModel::item( index ) );
}

const QList< KTextEditor::SmartRange * > & CodeItem::references( ) const
{
    return m_references;
}

void CodeItem::addReference( KTextEditor::SmartRange * range )
{
    m_references.append(range);
    range->addWatcher(this);
}

void CodeItem::deleted( KTextEditor::SmartRange * range )
{
    m_references.removeAll(range);
}

KTextEditor::SmartRange * CodeItem::declaration( ) const
{
    return m_declaration;
}

void CodeItem::setDeclaration( KTextEditor::SmartRange * range )
{
    m_declaration = range;
    range->addWatcher(this);
}

KTextEditor::SmartRange * CodeItem::definition( ) const
{
    return m_definition;
}

void CodeItem::setDefinition( KTextEditor::SmartRange * range )
{
    m_definition = range;
    range->addWatcher(this);
}

}
#include "kdevcodemodel.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
