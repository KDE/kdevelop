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

#include "kdevdocumentview_part.h"
#include "kdevdocumentviewdelegate.h"
#include "kdevdocumentview.h"
#include "kdevdocumentmodel.h"
#include "kdevdocumentselection.h"

#include <QtGui/QVBoxLayout>

#include <kaction.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevplugininfo.h>
#include <kdevmainwindow.h>
#include <kdevgenericfactory.h>
#include <kfiltermodel.h>

typedef KDevGenericFactory<KDevDocumentViewPart> KDevDocumentViewFactory;
static const KDevPluginInfo data( "kdevdocumentview" );
K_EXPORT_COMPONENT_FACTORY( kdevdocumentview,
                            KDevDocumentViewFactory( data ) )

KDevDocumentViewPart::KDevDocumentViewPart( QObject *parent,
        const QStringList& )
        : KDevPlugin( &data, parent )
{
    m_documentModel = new KDevDocumentModel( this );

    setInstance( KDevDocumentViewFactory::instance() );

    m_documentView = new KDevDocumentView( this, 0 );
    m_documentView->setWindowIcon( SmallIcon( "kmultiple" ) );
    m_documentView->setWindowTitle( i18n( "Documents" ) );

    m_documentView->setModel( m_documentModel );
    KDevDocumentViewDelegate *delegate =
        new KDevDocumentViewDelegate( m_documentView, this );
    m_documentView->setSelectionModel(
        new KDevDocumentSelection( m_documentModel ) );
    m_documentView->setItemDelegate( delegate );
    m_documentView->setWhatsThis( i18n( "Document View" ) );

    KDevApi::self() ->mainWindow() ->embedSelectView( m_documentView,
                                    i18n( "Documents" ), i18n( "Documents" ) );

    KDevDocumentController* docController = KDevApi::self() ->documentController();

    connect( m_documentView, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( pressed( QModelIndex ) ) );
    connect( docController, SIGNAL( documentActivated( KDevDocument* ) ),
             this, SLOT( activated( KDevDocument* ) ) );
    connect( docController, SIGNAL( documentSaved( KDevDocument* ) ),
             this, SLOT( saved( KDevDocument* ) ) );
    connect( docController, SIGNAL( documentLoaded( KDevDocument* ) ),
             this, SLOT( loaded( KDevDocument* ) ) );
    connect( docController, SIGNAL( documentClosed( KDevDocument* ) ),
             this, SLOT( closed( KDevDocument* ) ) );
    connect( docController,
             SIGNAL( documentExternallyModified( KDevDocument* ) ),
             this, SLOT( externallyModified( KDevDocument* ) ) );
    connect( docController,
             SIGNAL( documentUrlChanged( KDevDocument*, const KUrl &, const KUrl & ) ),
             this, SLOT( urlChanged( KDevDocument*, const KUrl &, const KUrl & ) ) );
    connect( docController,
             SIGNAL( documentStateChanged( KDevDocument*, KDevDocument::DocumentState ) ),
             this, SLOT( stateChanged( KDevDocument*, KDevDocument::DocumentState ) ) );

    setXMLFile( "kdevdocumentview.rc" );
}

KDevDocumentViewPart::~KDevDocumentViewPart()
{
    if ( m_documentView )
    {
        KDevApi::self() ->mainWindow() ->removeView( m_documentView );
        delete m_documentView;
    }
}

void KDevDocumentViewPart::import( RefreshPolicy /*policy*/ )
{}

void KDevDocumentViewPart::activated( KDevDocument* document )
{
    m_documentView->setCurrentIndex( m_doc2index[ document ] );
}

void KDevDocumentViewPart::saved( KDevDocument* )
{
    kDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::loaded( KDevDocument* document )
{
    QString mimeType = KMimeType::findByURL( document->url() ) ->comment();
    KDevMimeTypeItem *mimeItem = m_documentModel->mimeType( mimeType );
    if ( !mimeItem )
    {
        mimeItem = new KDevMimeTypeItem( mimeType.toLatin1() );
        m_documentModel->appendItem( mimeItem );
        m_documentView->expand( m_documentModel->indexOf( mimeItem ) );
    }

    if ( !mimeItem->file( document->url() ) )
    {
        KDevFileItem * fileItem = new KDevFileItem( document->url() );
        m_documentModel->appendItem( fileItem, mimeItem );
        m_documentView->setCurrentIndex( m_documentModel->indexOf( fileItem ) );
        m_doc2index[ document ] = m_documentModel->indexOf( fileItem );
    }
}

void KDevDocumentViewPart::closed( KDevDocument* document )
{
    kDebug() << k_funcinfo << endl;
    QModelIndex fileIndex = m_doc2index[ document ];
    KDevDocumentItem *fileItem = m_documentModel->item( fileIndex );
    if ( !fileItem )
        return ;

    QModelIndex mimeIndex = m_documentModel->parent( fileIndex );

    m_documentModel->removeItem( fileItem );
    m_doc2index.remove( document );

    if ( m_documentModel->hasChildren( mimeIndex ) )
        return ;

    KDevDocumentItem *mimeItem = m_documentModel->item( mimeIndex );
    if ( !mimeItem )
        return ;

    m_documentModel->removeItem( mimeItem );
}

void KDevDocumentViewPart::externallyModified( KDevDocument* )
{
    kDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::urlChanged( KDevDocument*, const KUrl & /*oldurl*/,
                                       const KUrl & /*newurl*/ )
{
    kDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::stateChanged( KDevDocument* document,
        KDevDocument::DocumentState state )
{
    KDevDocumentItem * documentItem =
        m_documentModel->item( m_doc2index[ document ] );

    if ( !documentItem )
        return ;

    if ( documentItem->documentState() == state )
        return ;

    documentItem->setDocumentState( state );
    m_documentView->doItemsLayout();
}

void KDevDocumentViewPart::pressed( const QModelIndex & index )
{
    if ( index.parent().isValid() )
        KDevApi::self() ->documentController() ->editDocument(
            m_documentModel->item( index ) ->fileItem() ->URL() );
}

#include "kdevdocumentview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
