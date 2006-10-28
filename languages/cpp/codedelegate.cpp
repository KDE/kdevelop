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

#include "parser/codemodel.h"

CodeDelegate::CodeDelegate( QObject *parent )
        : KDevCodeDelegate( parent )
{}

CodeDelegate::~CodeDelegate()
{}

void CodeDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index ) const
{
    Q_ASSERT( index.isValid() );
    QStyleOptionViewItem opt = option;

    updateStyle( opt, index );

    QItemDelegate::paint( painter, opt, index );
}

QSize CodeDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_ASSERT( index.isValid() );
    QStyleOptionViewItem opt = option;

    updateStyle( opt, index );

    return QItemDelegate::sizeHint( opt, index );
}

void CodeDelegate::updateStyle( QStyleOptionViewItem& opt, const QModelIndex& index ) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT( model );
    QVariant value = model->data( index, Qt::UserRole );
    Q_ASSERT( value.isValid() ); //This should be set to the KDevCodeItem kind()

    switch ( value.toInt() )
    {
    case _CodeModelItem::Kind_Namespace:
        opt.palette.setColor( QPalette::Text, Qt::blue );
        break;
    case _CodeModelItem::Kind_Member:
        opt.palette.setColor( QPalette::Text, Qt::gray );
        break;
    case _CodeModelItem::Kind_Class:
        opt.font.setBold( true );
        break;
    case _CodeModelItem::Kind_File:
        opt.font.setBold( true );
        break;
    case _CodeModelItem::Kind_FunctionDefinition:
        opt.font.setItalic( true );
        break;
    default:
        break;
    }

    opt.fontMetrics = QFontMetrics(opt.font);
}

#include "codedelegate.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
