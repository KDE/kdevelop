/* This file is part of KDevelop
 Copyright (C) 2005 Adam Treat <treat@kde.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/

#include "kdevdocumentviewdelegate.h"

#include <QtGui/QTreeView>

#include <kdebug.h>

KDevDocumentViewDelegate::KDevDocumentViewDelegate( QTreeView *view, QObject *parent )
        : QItemDelegate( parent ),
        m_view( view )
{}

KDevDocumentViewDelegate::~KDevDocumentViewDelegate()
{}

void KDevDocumentViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QAbstractItemModel * model = index.model();
    Q_ASSERT( model );

    if ( !model->parent( index ).isValid() )
    {
        // this is a top-level item.
/*        QStyleOptionButton buttonOption;*/
        QStyleOptionHeader buttonOption;

        buttonOption.state = option.state;
#ifdef Q_WS_MAC

        buttonOption.state |= QStyle::State_Raised;
#endif

        buttonOption.state &= ~QStyle::State_HasFocus;

        buttonOption.rect = option.rect;
        buttonOption.rect.setLeft( option.rect.left() - 1 );
        buttonOption.rect.setRight( option.rect.right() - 1 );
        buttonOption.palette = option.palette;
/*        buttonOption.features = QStyleOptionButton::None;*/
        buttonOption.section = QStyleOptionHeader::OnlyOneSection;
        buttonOption.textAlignment = Qt::AlignCenter;
        buttonOption.text = model->data( index, Qt::DisplayRole ).toString();
        m_view->style() ->drawControl( QStyle::CE_Header, &buttonOption, painter, m_view );

        QStyleOption branchOption;
        static const int i = 9; // ### hardcoded in qcommonstyle.cpp
        QRect r = option.rect;
        branchOption.rect = QRect( r.left() + i / 2, r.top() + ( r.height() - i ) / 2, i, i );
        branchOption.palette = option.palette;
        branchOption.state = QStyle::State_Children;

        if ( m_view->isExpanded( index ) )
            branchOption.state |= QStyle::State_Open;

        m_view->style() ->drawPrimitive( QStyle::PE_IndicatorBranch, &branchOption, painter, m_view );
    }
    else
    {
        QStyleOptionViewItem opt = option;
        if ( !model->data( index, Qt::DecorationRole ).isNull() )
            opt.rect.setLeft( option.rect.left() - option.decorationSize.width() - 2);
        QItemDelegate::paint( painter, opt, index );
    }
}

QSize KDevDocumentViewDelegate::sizeHint( const QStyleOptionViewItem &opt, const QModelIndex &index ) const
{
    QStyleOptionViewItem option = opt;
    QSize sz = QItemDelegate::sizeHint( opt, index ) + QSize( 2, 2 );
    return sz;
}

#include "kdevdocumentviewdelegate.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
