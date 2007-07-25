/* This file is part of KDevelop
Copyright 2005 Adam Treat <treat@kde.org>

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

#include <QAction>
#include <QHeaderView>
#include <QContextMenuEvent>

#include <kurl.h>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdevcore.h>
#include <kdevcontext.h>
#include <kdevmainwindow.h>
#include <kdevdocumentcontroller.h>

KDevDocumentView::KDevDocumentView( KDevDocumentViewPart *part, QWidget *parent )
    : KDevelop::TreeView( parent ),
        m_part( part )
{
    setObjectName( i18n( "Documents" ) );

    setWindowIcon( SmallIcon( "kmultiple" ) );
    setWindowTitle( i18n( "Documents" ) );
    setWhatsThis( i18n( "Document View" ) );

    setFocusPolicy( Qt::NoFocus );

    setRootIsDecorated( false );
    header() ->hide();
    header() ->setResizeMode( QHeaderView::Stretch );

    setSelectionBehavior( QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
}

KDevDocumentView::~KDevDocumentView()
{}

KDevDocumentViewPart *KDevDocumentView::part() const
{
    return m_part;
}

void KDevDocumentView::mousePressEvent( QMouseEvent * event )
{
    QModelIndex index = indexAt( event->pos() );
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );

    if ( event->button() == Qt::LeftButton && index.parent().isValid() &&
            event->modifiers() == Qt::NoModifier )
    {
        KDevelop::Core::documentController() ->editDocument(
            docModel->item( index ) ->fileItem() ->URL() );

    }

    if ( !index.parent().isValid() )
    {
        setExpanded( index, !isExpanded( index ) );
    }

    KDevelop::TreeView::mousePressEvent( event );
}

void KDevDocumentView::contextMenuEvent( QContextMenuEvent * event )
{
    QModelIndexList indexes = selectionModel() ->selectedIndexes();
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );

    KUrl::List list;
    foreach ( QModelIndex index, indexes )
    {
        if ( KDevFileItem * fileItem = docModel->item( index ) ->fileItem() )
        {
            list.append( fileItem->URL() );
        }
    }

    KMenu menu( this );
    KDevelop::FileContext context( list ); //FIXME change filecontext to documentcontext
    KDevelop::Core::mainWindow() ->fillContextMenu( &menu, &context );
    menu.exec( event->globalPos() );

    KDevelop::TreeView::contextMenuEvent( event );
}

#include "kdevdocumentview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
