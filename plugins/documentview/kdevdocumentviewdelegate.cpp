/* This file is part of KDevelop
Copyright 2005 Adam Treat <treat@kde.org>
Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

KDevDocumentViewDelegate::KDevDocumentViewDelegate( QObject *parent )
    : QItemDelegate( parent )
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
        QStyleOptionViewItem o = option;
        o.textElideMode = Qt::ElideLeft;
        QItemDelegate::paint( painter, o, index );
    }
    else
    {
        QItemDelegate::paint( painter, option, index );
    }
}

#include "kdevdocumentviewdelegate.moc"
