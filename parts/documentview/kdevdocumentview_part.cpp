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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "kdevdocumentview_part.h"
#include "kdevdocumentviewdelegate.h"
#include "kdevdocumentview.h"
#include "kdevdocumentmodel.h"

#include <QtGui/QVBoxLayout>

#include <kaction.h>
#include <klocale.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include <kdevapi.h>
#include <urlutil.h>
#include <kdevcore.h>
#include <kdevplugininfo.h>
#include <kdevmainwindow.h>
#include <kdevgenericfactory.h>
#include <kdevpartcontroller.h>
#include <kfiltermodel.h>

typedef KDevGenericFactory<KDevDocumentViewPart> KDevDocumentViewFactory;
static const KDevPluginInfo data( "kdevdocumentview" );
K_EXPORT_COMPONENT_FACTORY( libkdevdocumentview, KDevDocumentViewFactory( data ) );

KDevDocumentViewPart::KDevDocumentViewPart( QObject *parent, const char *name, const QStringList& )
        : KDevPlugin( &data, parent )
{
    setObjectName( QString::fromUtf8( name ) );

    m_documentModel = new KDevDocumentModel( this );

    setInstance( KDevDocumentViewFactory::instance() );

    m_widget = new QWidget( 0 );
    new QVBoxLayout( m_widget );

    m_documentView = new KDevDocumentView( this, m_widget );
    KDevDocumentViewDelegate *delegate = new KDevDocumentViewDelegate( m_documentView, this );
    m_documentView->setModel( m_documentModel );
    m_documentView->setItemDelegate( delegate );
    m_documentView->setWhatsThis( i18n( "Document View" ) );
    m_widget->layout() ->add
    ( m_documentView );

    mainWindow() ->embedSelectView( m_widget, i18n( "Documents" ), i18n( "Documents" ) );

    connect( m_documentView, SIGNAL( pressed( QModelIndex ) ), this, SLOT( filePressed( QModelIndex ) ) );
    connect( partController(), SIGNAL( savedFile( const KURL & ) ), this, SLOT( saveFile( const KURL & ) ) );
    connect( partController(), SIGNAL( loadedFile( const KURL & ) ), this, SLOT( loadFile( const KURL & ) ) );
    connect( partController(), SIGNAL( closedFile( const KURL & ) ), this, SLOT( closeFile( const KURL & ) ) );
    connect( partController(), SIGNAL( fileDirty( const KURL & ) ), this, SLOT( dirtyFile( const KURL & ) ) );
    //     connect( partController(), SIGNAL( partURLChanged(KParts::ReadOnlyPart *) ), this, SLOT() );
    //     connect( partController(), SIGNAL( documentChangedState(const KURL &, DocumentState) ), this, SLOT() );

    setXMLFile( "kdevdocumentview.rc" );
}

KDevDocumentViewPart::~KDevDocumentViewPart()
{
    if ( m_documentView )
    {
        mainWindow() ->removeView( m_widget );
        delete m_widget;
    }
}

void KDevDocumentViewPart::import( RefreshPolicy /*policy*/ )
{}

void KDevDocumentViewPart::saveFile( const KURL &/*url*/ )
{
    kdDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::loadFile( const KURL &url )
{
    QString mimeType = KMimeType::findByURL( url ) ->comment();
    KDevMimeTypeItem *mimeItem = m_documentModel->mimeType( mimeType );
    if ( !mimeItem )
    {
        mimeItem = new KDevMimeTypeItem( mimeType.toLatin1() );
        m_documentModel->appendItem( mimeItem );
        m_documentView->expand( m_documentModel->indexOf( mimeItem ) );
    }

    if ( !mimeItem->file( url ) )
    {
        m_documentModel->appendItem( new KDevFileItem( url ), mimeItem );
        m_documentModel->sort( 0, Qt::Ascending );
    }
}

void KDevDocumentViewPart::closeFile( const KURL &/*url*/ )
{
    kdDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::dirtyFile( const KURL &/*url*/ )
{
    kdDebug() << k_funcinfo << endl;
}

void KDevDocumentViewPart::filePressed( const QModelIndex & index )
{
    if ( index.parent().isValid() )
        partController() ->editDocument(
            m_documentModel->item( index ) ->fileItem() ->URL() );
}

#include "kdevdocumentview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
