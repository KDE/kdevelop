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

#include "kdevcodeview.h"
#include "kdevcodeview_part.h"

#include <QtGui/QHeaderView>

#include <kmenu.h>
#include <kdebug.h>
#include <kfile.h>
#include <klocale.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevcodeproxy.h>
#include <kdevlanguagesupport.h>
#include <kdevdocumentcontroller.h>

#include <QtCore/qdebug.h>

KDevCodeView::KDevCodeView( KDevCodeViewPart *part, QWidget *parent )
        : KDevTreeView( parent ),
        m_part( part ),
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
    connect( m_part ->documentController(),
             SIGNAL( documentActivated( KDevDocument* ) ),
             this, SLOT( documentActivated( KDevDocument* ) ) );
}

KDevCodeView::~KDevCodeView()
{}

KDevCodeViewPart *KDevCodeView::part() const
{
    return m_part;
}

KDevCodeProxy *KDevCodeView::codeProxy() const
{
    return qobject_cast<KDevCodeProxy*>( model() );
}

void KDevCodeView::documentActivated( KDevDocument* file )
{
    if ( m_trackCurrent &&
            part() ->languageSupport() ->supportsDocument( file ) )
        codeProxy() ->setFilterDocument( file->url() );
}

void KDevCodeView::activated( const QModelIndex &index )
{
    if ( KDevCodeItem * item = codeProxy() ->proxyToItem( index ) )
    {
        KUrl document( item->fileName() );
        if ( document.isValid() )
            part() ->documentController() ->editDocument( document,
                    item->startPosition() );
    }
}

void KDevCodeView::modeCurrent()
{
    m_trackCurrent = true;
    codeProxy() ->setFilterDocument(
        part() ->documentController() ->activeDocumentUrl() );
}

void KDevCodeView::modeNormalize()
{
    m_trackCurrent = false;
    codeProxy() ->setMode( KDevCodeProxy::Normalize );
}

void KDevCodeView::modeAggregate()
{
    m_trackCurrent = false;
    codeProxy() ->setMode( KDevCodeProxy::Aggregate );
}

void KDevCodeView::popupContextMenu( const QPoint &pos )
{
    Q_UNUSED( pos );
}

void KDevCodeView::filterKind()
{
    QAction * action = qobject_cast<QAction*>( sender() );
    int kind = action->data().toInt();

    if ( action->isChecked() )
        m_kindFilter = m_kindFilter | kind;
    else
        m_kindFilter = m_kindFilter ^ kind;
    codeProxy() ->setKindFilter( m_kindFilter );
}

#include "kdevcodeview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
