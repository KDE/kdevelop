/*
    SPDX-FileCopyrightText: xxxx KFile Authors
    SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2012 Niko Sams <niko.sams@gmai.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "bookmarkhandler.h"
#include "filemanager.h"
#include "debug.h"
#include <interfaces/icore.h>
#include <interfaces/isession.h>

#include <QMenu>

BookmarkHandler::BookmarkHandler( FileManager *parent, QMenu* kpopupmenu )
    : QObject( parent ),
    KBookmarkOwner(),
    m_parent( parent ),
    m_menu( kpopupmenu )
{
    setObjectName( QStringLiteral( "BookmarkHandler" ) );

    // The subpath had been pluginId/"fsbookmarks.xml" with pluginId="kdevfilemanager" before
    // 715d09e5545758af0ba85c8fea7c11ecb94eb9ad accidentally removed the slash after pluginId while
    // porting from KUrl to QUrl. So now we are stuck with the long filename for backward compatibility.
    const QString bookmarksPath =
        KDevelop::ICore::self()->activeSession()->dataDirectory() + QLatin1String("/kdevfilemanagerfsbookmarks.xml");
    qCDebug(PLUGIN_FILEMANAGER) << bookmarksPath;

    auto* const manager = new KBookmarkManager(bookmarksPath, this);
    m_bookmarkMenu = new KBookmarkMenu(manager, this, m_menu);
    const auto actions = m_menu->actions();

     //remove shortcuts as they might conflict with others (eg. Ctrl+B)
    for (QAction* action : actions) {
        action->setShortcut(QKeySequence());
    }
}

BookmarkHandler::~BookmarkHandler()
{
    delete m_bookmarkMenu;
}

QUrl BookmarkHandler::currentUrl() const
{
    return m_parent->dirOperator()->url();
}

QString BookmarkHandler::currentTitle() const
{
    return currentUrl().toDisplayString();
}

void BookmarkHandler::openBookmark( const KBookmark & bm, Qt::MouseButtons, Qt::KeyboardModifiers )
{
    emit openUrl(bm.url());
}

#include "moc_bookmarkhandler.cpp"
