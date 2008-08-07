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
#include "kdevdocumentviewplugin.h"
#include "kdevdocumentmodel.h"

#include <QHeaderView>
#include <QContextMenuEvent>

#include <kaction.h>
#include <kurl.h>
#include <kmenu.h>
#include <kdebug.h>
//#include <klocale.h>
#include <kiconloader.h>
#include <kstandardaction.h>

#include "kdevdocumentselection.h"
#include "kdevdocumentviewdelegate.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/context.h>

KDevDocumentView::KDevDocumentView( KDevDocumentViewPlugin *plugin, QWidget *parent )
    : QTreeView( parent ),
        m_plugin( plugin )
{
    m_documentModel = new KDevDocumentModel();

    m_selectionModel = new KDevDocumentSelection( m_documentModel );
    m_delegate = new KDevDocumentViewDelegate( this, this );

    setModel( m_documentModel );
    setSelectionModel( m_selectionModel );
    setItemDelegate( m_delegate );

    setObjectName( i18n( "Documents" ) );

    setWindowIcon( SmallIcon( "document-multiple" ) );
    setWindowTitle( i18n( "Documents" ) );
    setWhatsThis( i18n( "Document View" ) );

    setFocusPolicy( Qt::NoFocus );

    setRootIsDecorated( false );
    header() ->hide();
    header() ->setResizeMode( QHeaderView::Stretch );

    setSelectionBehavior( QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    m_save = KStandardAction::save(this, SLOT(saveSelected()), this);
    KAction* close  = KStandardAction::close(this, SLOT(closeSelected()), this);

    m_ctxMenu = new KMenu(this);
    m_ctxMenu->addAction(close);
    m_ctxMenu->addAction(m_save);
}

KDevDocumentView::~KDevDocumentView()
{}

KDevDocumentViewPlugin *KDevDocumentView::plugin() const
{
    return m_plugin;
}

void KDevDocumentView::mousePressEvent( QMouseEvent * event )
{
    QModelIndex index = indexAt( event->pos() );
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );

    if ( event->button() == Qt::LeftButton && index.parent().isValid() &&
            event->modifiers() == Qt::NoModifier )
    {
        m_plugin->core()->documentController() ->openDocument(
            static_cast<KDevDocumentItem*>( docModel->itemFromIndex( index ) ) ->fileItem() ->url() );
        return;

    }

    if ( !index.parent().isValid() )
    {
        setExpanded( index, !isExpanded( index ) );
        return;
    }

    QTreeView::mousePressEvent( event );
}

template<typename F> void KDevDocumentView::visitSelected(F f)
{
    KDevelop::IDocumentController* dc = m_plugin->core()->documentController();
    foreach(KUrl url, m_selectedDocs) {
       KDevelop::IDocument* doc = dc->documentForUrl(url);
       if (doc) f(doc);
    }
}

namespace
{
class DocSaver
{
public: void operator()(KDevelop::IDocument* doc) { doc->save(); }
};
class DocCloser
{
public: void operator()(KDevelop::IDocument* doc) { doc->close(); }
};
}

void KDevDocumentView::saveSelected()
{
    visitSelected(DocSaver());
}

void KDevDocumentView::closeSelected()
{
    visitSelected(DocCloser());
}

void KDevDocumentView::contextMenuEvent( QContextMenuEvent * event )
{
    QModelIndexList indexes = selectionModel() ->selectedIndexes();
    KDevDocumentModel *docModel = qobject_cast<KDevDocumentModel*>( model() );

    m_selectedDocs.clear();
    foreach ( QModelIndex index, indexes )
    {
        if ( KDevFileItem * fileItem = dynamic_cast<KDevDocumentItem*>( docModel->itemFromIndex( index ) )->fileItem() )
        {
            m_selectedDocs << fileItem->url();
        }
    }
    if (!m_selectedDocs.isEmpty())
    {
        m_save->setEnabled( someDocHasChanges() );
        m_ctxMenu->exec( event->globalPos() );
    }
}

bool KDevDocumentView::someDocHasChanges()
{
    KDevelop::IDocumentController* dc = m_plugin->core()->documentController();
    foreach(KUrl url, m_selectedDocs)
    {
        KDevelop::IDocument* doc = dc->documentForUrl(url);
        if (!doc) continue;
        if (doc->state() != KDevelop::IDocument::Clean)
        {
            return true;
        }
    }
    return false;
}

void KDevDocumentView::activated( KDevelop::IDocument* document )
{
    setCurrentIndex( m_documentModel->indexFromItem( m_doc2index[ document ] ) );
}

void KDevDocumentView::saved( KDevelop::IDocument* )
{
    kDebug() ;
}

void KDevDocumentView::opened( KDevelop::IDocument* document )
{
    QString mimeType = document->mimeType() ->comment();
    KDevMimeTypeItem *mimeItem = m_documentModel->mimeType( mimeType );
    if ( !mimeItem )
    {
        mimeItem = new KDevMimeTypeItem( mimeType.toLatin1() );
        m_documentModel->insertRow( m_documentModel->rowCount(), mimeItem );
        setExpanded( m_documentModel->indexFromItem( mimeItem ), false);
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

