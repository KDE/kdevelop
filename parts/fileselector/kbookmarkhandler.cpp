/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <stdio.h>
#include <stdlib.h>

#include <qtextstream.h>

#include <kbookmarkimporter.h>
#include <kpopupmenu.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kdiroperator.h>
#include <kaction.h>

#include "kbookmarkhandler.h"
#include "kbookmarkhandler.moc"


KBookmarkHandler::KBookmarkHandler( KDevFileSelector *parent, KPopupMenu* kpopupmenu )
    : QObject( parent, "KBookmarkHandler" ),
      KBookmarkOwner(),
      mParent( parent ),
      m_menu( kpopupmenu ),
      m_importStream( 0L )
{
    if (!m_menu)
      m_menu = new KPopupMenu( parent, "bookmark menu" );

    QString file = locate( "data", "kdevfileselector/fsbookmarks.xml" );
    if ( file.isEmpty() )
        file = locateLocal( "data", "kdevfileselector/fsbookmarks.xml" );

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);
    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu, 0, true );
}

KBookmarkHandler::~KBookmarkHandler()
{
    //     delete m_bookmarkMenu; ###
}

QString KBookmarkHandler::currentURL() const
{
    return mParent->dirOperator()->url().url();
}


void KBookmarkHandler::slotNewBookmark( const QString& text,
                                            const QCString& url,
                                            const QString& additionalInfo )
{
    Q_UNUSED( text );
    *m_importStream << "<bookmark icon=\"" << KMimeType::iconForURL( KURL(  url ) );
    *m_importStream << "\" href=\"" << QString::fromUtf8(url) << "\">\n";
    *m_importStream << "<title>" << (additionalInfo.isEmpty() ? QString::fromUtf8(url) : additionalInfo) << "</title>\n</bookmark>\n";
}

void KBookmarkHandler::slotNewFolder( const QString& text, bool /*open*/,
                                          const QString& /*additionalInfo*/ )
{
    *m_importStream << "<folder icon=\"bookmark_folder\">\n<title=\"";
    *m_importStream << text << "\">\n";
}

void KBookmarkHandler::newSeparator()
{
    *m_importStream << "<separator/>\n";
}

void KBookmarkHandler::endFolder()
{
    *m_importStream << "</folder>\n";
}

void KBookmarkHandler::virtual_hook( int id, void* data )
{ KBookmarkOwner::virtual_hook( id, data ); }

