/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2009 Dominik Haumann <dhaumann kde org>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2012 Niko Sams <niko.sams@gmai.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "bookmarkhandler.h"
#include "bookmarkhandler.moc"
#include "filemanager.h"
#include "kdevfilemanagerplugin.h"
#include <interfaces/icore.h>
#include <interfaces/isession.h>

#include <kdiroperator.h>
#include <kstandarddirs.h>
#include <KDebug>
#include <KActionCollection>


BookmarkHandler::BookmarkHandler( FileManager *parent, KMenu* kpopupmenu )
    : QObject( parent ),
    KBookmarkOwner(),
    m_parent( parent ),
    m_menu( kpopupmenu )
{
    setObjectName( "BookmarkHandler" );

    KUrl bookmarksPath = KDevelop::ICore::self()->activeSession()->pluginDataArea(parent->plugin());
    bookmarksPath.addPath("fsbookmarks.xml");
    kDebug() << bookmarksPath;

    KBookmarkManager *manager = KBookmarkManager::managerForFile( bookmarksPath.toLocalFile(), "kdevplatform" );
    manager->setUpdate( true );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu, parent->actionCollection() );

     //remove shortcuts as they might conflict with others (eg. Ctrl+B)
    foreach (QAction *action, parent->actionCollection()->actions()) {
        action->setShortcut(QKeySequence());
    }
}

BookmarkHandler::~BookmarkHandler()
{
    delete m_bookmarkMenu;
}

QString BookmarkHandler::currentUrl() const
{
    return m_parent->dirOperator()->url().url();
}

QString BookmarkHandler::currentTitle() const
{
    return currentUrl();
}

void BookmarkHandler::openBookmark( const KBookmark & bm, Qt::MouseButtons, Qt::KeyboardModifiers )
{
    emit openUrl(bm.url());
}
