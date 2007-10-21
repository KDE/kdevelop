/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
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

#include "kdevcodetree.h"

#include <QtGui/QHeaderView>

#include <kmenu.h>
#include <kdebug.h>
#include <kfile.h>
#include <klocale.h>

#include <kdevcore.h>
#include <kdevcodeproxy.h>
#include <kdevlanguagesupport.h>
#include <kdevdocumentcontroller.h>

KDevCodeTree::KDevCodeTree( QWidget *parent )
    : KDevelop::TreeView( parent ),
        m_trackCurrent( true ),
        m_kindFilter( 0 )
{
    header() ->hide();
    header() ->setResizeMode( QHeaderView::Stretch );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( activated( QModelIndex ) ) );
    connect( this, SIGNAL( customContextMenuRequested( QPoint ) ),
             this, SLOT( popupContextMenu( QPoint ) ) );
    connect( KDevelop::Core::documentController(),
             SIGNAL( documentActivated( KDevelop::Document* ) ),
             this, SLOT( documentActivated( KDevelop::Document* ) ) );
}

KDevCodeTree::~KDevCodeTree()
{}

KDevelop::CodeProxy *KDevCodeTree::codeProxy() const
{
    return qobject_cast<KDevelop::CodeProxy*>( model() );
}

void KDevCodeTree::documentActivated( KDevelop::Document* file )
{
    if ( m_trackCurrent &&
         KDevelop::Core::activeLanguage() ->supportsDocument( file ) )
        codeProxy() ->setFilterDocument( file->url() );
}

void KDevCodeTree::activated( const QModelIndex &index )
{
    if ( KDevelop::CodeItem * item = codeProxy() ->proxyToItem( index ) )
    {
        KUrl document( item->fileName() );
        if ( document.isValid() )
            KDevelop::Core::documentController() ->editDocument( document,
                    item->startPosition() );
    }
}

void KDevCodeTree::modeCurrent()
{
    m_trackCurrent = true;
    codeProxy() ->setFilterDocument(
            KDevelop::Core::documentController() ->activeDocumentUrl() );
}

void KDevCodeTree::modeNormalize()
{
    m_trackCurrent = false;
    codeProxy() ->setMode( KDevelop::CodeProxy::Normalize );
}

void KDevCodeTree::modeAggregate()
{
    m_trackCurrent = false;
    codeProxy() ->setMode( KDevelop::CodeProxy::Aggregate );
}

void KDevCodeTree::popupContextMenu( const QPoint &pos )
{
    Q_UNUSED( pos );
}

void KDevCodeTree::filterKind()
{
    QAction * action = qobject_cast<QAction*>( sender() );
    int kind = action->data().toInt();

    if ( action->isChecked() )
        m_kindFilter = m_kindFilter | kind;
    else
        m_kindFilter = m_kindFilter ^ kind;
    codeProxy() ->setKindFilter( m_kindFilter );
}

#include "kdevcodetree.moc"

