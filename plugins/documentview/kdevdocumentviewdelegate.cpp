/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevdocumentviewdelegate.h"

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

#include "moc_kdevdocumentviewdelegate.cpp"
