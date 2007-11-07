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

#include "kdevdocumentselection.h"
#include "kdevdocumentviewdelegate.h"

#include <icore.h>
//#include <kdevcontext.h>
//#include <kdevmainwindow.h>
#include <idocumentcontroller.h>
#include <iplugincontroller.h>
#include <context.h>

KDevDocumentView::KDevDocumentView( KDevDocumentViewPart *part, QWidget *parent )
    : QTreeView( parent ),
        m_part( part )
{
    m_documentModel = new KDevDocumentModel();

    m_selectionModel = new KDevDocumentSelection( m_documentModel );
    m_delegate = new KDevDocumentViewDelegate( this, this );

    setModel( m_documentModel );
    setSelectionModel( m_selectionModel );
    setItemDelegate( m_delegate );

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
        m_part->core()->documentController() ->openDocument(
            static_cast<KDevDocumentItem*>( docModel->itemFromIndex( index ) ) ->fileItem() ->url() );

    }

    if ( !index.parent().isValid() )
    {
        setExpanded( index, !isExpanded( index ) );
    }

    QTreeView::mousePressEvent( event );
}

void KDevDocumentView::contextMenuEvent( QContextMenuEvent * event )
{
    QModelIndexList indexes = selectionModel() ->selectedIndexes();
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );

    KUrl::List list;
    foreach ( QModelIndex index, indexes )
    {
        if ( KDevFileItem * fileItem = dynamic_cast<KDevDocumentItem*>( docModel->itemFromIndex( index ) )->fileItem() )
        {
            list.append( fileItem->url() );
        }
    }

    KMenu menu( this );
    KDevelop::FileContext context( list ); //FIXME change filecontext to documentcontext
    m_part->core()->pluginController()->buildContextMenu( &context, &menu );
    menu.exec( event->globalPos() );

    QTreeView::contextMenuEvent( event );
}

void KDevDocumentView::activated( KDevelop::IDocument* document )
{
    setCurrentIndex( m_documentModel->indexFromItem( m_doc2index[ document ] ) );
}

void KDevDocumentView::saved( KDevelop::IDocument* )
{
    kDebug() ;
}

void KDevDocumentView::loaded( KDevelop::IDocument* document )
{
    QString mimeType = document->mimeType() ->comment();
    KDevMimeTypeItem *mimeItem = m_documentModel->mimeType( mimeType );
    if ( !mimeItem )
    {
        mimeItem = new KDevMimeTypeItem( mimeType.toLatin1() );
        m_documentModel->insertRow( m_documentModel->rowCount(), mimeItem );
        expand( m_documentModel->indexFromItem( mimeItem ) );
    }

    if ( !mimeItem->file( document->url() ) )
    {
        KDevFileItem * fileItem = new KDevFileItem( document->url() );
        mimeItem->setChild( mimeItem->rowCount(), fileItem );
        setCurrentIndex( m_documentModel->indexFromItem( fileItem ) );
        m_doc2index[ document ] = fileItem;
    }
}

void KDevDocumentView::closed( KDevelop::IDocument* document )
{
    KDevFileItem* file = m_doc2index[ document ];
    if ( !file )
        return;

    QStandardItem* mimeItem = file->parent();

    qDeleteAll(mimeItem->takeRow(m_documentModel->indexFromItem(file).row()));

    m_doc2index.remove(document);

    if ( mimeItem->hasChildren() )
        return;

    qDeleteAll(m_documentModel->takeRow(m_documentModel->indexFromItem(mimeItem).row()));

    doItemsLayout();
}

void KDevDocumentView::contentChanged( KDevelop::IDocument* )
{
    kDebug() ;
}

void KDevDocumentView::stateChanged( KDevelop::IDocument* document )
{
    KDevDocumentItem * documentItem = m_doc2index[ document ];

    if ( documentItem && documentItem->documentState() != document->state() )
        documentItem->setDocumentState( document->state() );

    doItemsLayout();
}

#include "kdevdocumentview.moc"

