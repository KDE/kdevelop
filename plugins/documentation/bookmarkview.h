/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BOOKMARKVIEW_H
#define BOOKMARKVIEW_H

#include <qwidget.h>

#include <kbookmarkmanager.h>

class KListView;
class KPushButton;
class DocumentationPart;
class DocumentationWidget;
class QListViewItem;

class DocBookmarkManager: public KBookmarkManager {
public:
    DocBookmarkManager(DocumentationPart *part);
};

class DocBookmarkOwner: public KBookmarkOwner {
public:
    DocBookmarkOwner(DocumentationPart *part);

    virtual void openBookmarkURL(const QString &_url);
    virtual QString currentTitle() const;
    virtual QString currentURL() const;

private:
    DocumentationPart *m_part;
};

class BookmarkView : public QWidget
{
    Q_OBJECT
public:
    BookmarkView(DocumentationWidget *parent = 0, const char *name = 0);
    ~BookmarkView();
    
public slots:
    void addBookmark(const QString &title, const KURL &url);

protected:
    void showBookmarks();
    virtual void focusInEvent(QFocusEvent *e);

protected slots:
    void itemExecuted(QListViewItem *item, const QPoint &p, int col);
    void addBookmark();
    void editBookmark();
    void removeBookmark();
    void itemMouseButtonPressed(int button, QListViewItem *item, const QPoint &pos, int c);

private:
    DocumentationWidget *m_widget;
    KListView *m_view;
    KPushButton *m_addButton;
    KPushButton *m_editButton;    
    KPushButton *m_removeButton;
    
    DocBookmarkManager *m_bmManager;
    DocBookmarkOwner *m_bmOwner;
};

#endif
