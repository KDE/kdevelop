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
#include "kdevfilemanagerplugin.h"
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

    QUrl bookmarksPath = KDevelop::ICore::self()->activeSession()->pluginDataArea(parent->plugin());
    bookmarksPath.setPath(bookmarksPath.path() + QLatin1String("fsbookmarks.xml"));
    qCDebug(PLUGIN_FILEMANAGER) << bookmarksPath;

    auto* const manager = new KBookmarkManager(bookmarksPath.toLocalFile(), this);
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
