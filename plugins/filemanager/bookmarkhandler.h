/*
    SPDX-FileCopyrightText: xxxx KFile Authors
    SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>
    SPDX-FileCopyrightText: 2012 Niko Sams <niko.sams@gmai.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_BOOKMARKHANDLER_H
#define KDEVPLATFORM_PLUGIN_BOOKMARKHANDLER_H

#include <KBookmarkManager>
#include <KBookmarkMenu>

#include <QObject>

class FileManager;

class BookmarkHandler : public QObject, public KBookmarkOwner
{
Q_OBJECT

public:
    explicit BookmarkHandler( FileManager *parent, QMenu *kpopupmenu = nullptr );
    ~BookmarkHandler() override;

    // KBookmarkOwner interface:
    QUrl currentUrl() const override;
    QString currentTitle() const override;

    QMenu *menu() const
    {
      return m_menu;
    }
    void openBookmark( const KBookmark &, Qt::MouseButtons, Qt::KeyboardModifiers ) override;

Q_SIGNALS:
    void openUrl( const QUrl& url );

private:
    FileManager *m_parent;
    QMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;
};

#endif // KDEVPLATFORM_PLUGIN_BOOKMARKHANDLER_H

