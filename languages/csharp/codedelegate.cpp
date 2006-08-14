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

#include "codedelegate.h"

#include "parser/csharp_codemodel.h"

namespace csharp
{

CodeDelegate::CodeDelegate( QObject *parent )
        : KDevCodeDelegate( parent )
{}

CodeDelegate::~CodeDelegate()
{}

void CodeDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index ) const
{
    Q_ASSERT( index.isValid() );
    const QAbstractItemModel *model = index.model();
    Q_ASSERT( model );
    QStyleOptionViewItem opt = option;

    QVariant value = model->data( index, Qt::UserRole );
    Q_ASSERT( value.isValid() ); //This should be set to the KDevCodeItem kind()

    switch ( value.toInt() )
    {
    case _CodeModelItem::Kind_NamespaceDeclaration:
        opt.palette.setColor( QPalette::Text, Qt::blue );
        break;
    case _CodeModelItem::Kind_VariableDeclaration:
        opt.palette.setColor( QPalette::Text, Qt::gray );
        break;
    case _CodeModelItem::Kind_ClassDeclaration:
        opt.font.setBold( true );
        break;
    default:
        break;
    }

    opt.fontMetrics = QFontMetrics(opt.font);

    QItemDelegate::paint( painter, opt, index );
}

QSize CodeDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_ASSERT( index.isValid() );
    const QAbstractItemModel *model = index.model();
    Q_ASSERT( model );
    QStyleOptionViewItem opt = option;

    QVariant value = model->data( index, Qt::UserRole );
    Q_ASSERT( value.isValid() ); //This should be set to the KDevCodeItem kind()

    switch ( value.toInt() )
    {
    case _CodeModelItem::Kind_NamespaceDeclaration:
        opt.palette.setColor( QPalette::Text, Qt::blue );
        break;
    case _CodeModelItem::Kind_VariableDeclaration:
        opt.palette.setColor( QPalette::Text, Qt::gray );
        break;
    case _CodeModelItem::Kind_ClassDeclaration:
        opt.font.setBold( true );
        break;
    default:
        break;
    }

    opt.fontMetrics = QFontMetrics(opt.font);

    return QItemDelegate::sizeHint( opt, index );
}

} // end of namespace csharp

#include "codedelegate.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
