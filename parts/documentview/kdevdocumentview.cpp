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

#include "kdevdocumentview.h"
#include "kdevdocumentview_part.h"
#include "kdevdocumentmodel.h"

#include <QtGui/QHeaderView>

#include <kdevcore.h>
#include <kdevapi.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/qdebug.h>

KDevDocumentView::KDevDocumentView( KDevDocumentViewPart *part, QWidget *parent )
        : KDevTreeView( parent ),
        m_part( part )
{
    setRootIsDecorated( false );
    header() ->hide();
    header() ->setResizeMode( QHeaderView::Stretch );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( handleMousePress( QModelIndex ) ) );
    connect( this, SIGNAL( customContextMenuRequested( QPoint ) ),
             this, SLOT( popupContextMenu( QPoint ) ) );
}

KDevDocumentView::~KDevDocumentView()
{}

KDevDocumentViewPart *KDevDocumentView::part() const
{
    return m_part;
}

void KDevDocumentView::currentChanged( const QModelIndex & current,
                                       const QModelIndex & previous )
{
    if ( !current.parent().isValid() )
        setCurrentIndex( previous );
    else
        KDevTreeView::currentChanged( current, previous );
}

void KDevDocumentView::handleMousePress( const QModelIndex & index )
{
    if ( !index.parent().isValid() )
    {
        if ( isExpanded( index ) )
            collapse( index );
        else
            expand( index );
    }
}

void KDevDocumentView::popupContextMenu( const QPoint &pos )
{
    QModelIndex index = indexAt( pos );
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );
    if ( KDevDocumentItem * item = docModel->item( index ) )
        if ( KDevFileItem * fileItem = item->fileItem() )
        {
            QModelIndexList indexes = selectedIndexes();
            KMenu menu( this );
            KUrl::List list;
            list << fileItem->URL();
            FileContext context( list );
            part() ->core() ->fillContextMenu( &menu, &context );
            menu.exec( mapToGlobal( pos ) );
        }
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
