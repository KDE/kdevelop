/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREECONFIGWIDGET_H_
#define _DOCTREECONFIGWIDGET_H_

#include <qtabwidget.h>

class QCheckBox;
class QListView;
class QListViewItem;
class DocTreeViewWidget;


class DocTreeConfigWidget : public QTabWidget
{
    Q_OBJECT
	
public: 
    DocTreeConfigWidget( DocTreeViewWidget *widget, QWidget *parent, const char *name=0 );
    ~DocTreeConfigWidget();

    enum Page { KDevelop, Libraries, Bookmarks };
    void showPage(Page page);

public slots:
    void accept();

private slots:
    void updateLibrary();
    void editLibrary();
    void addLibrary();
    void removeLibrary();
    void addBookmarkClicked();
    void removeBookmarkClicked();
    void updateIndexClicked();

private:
    QWidget *createKDevelopTab();
    QWidget *createLibrariesTab();
    QWidget *createBookmarksTab();
    QWidget *createIndexTab();
    
    DocTreeViewWidget *m_widget;
    QWidget *kdevelopTab;
    QWidget *librariesTab;
    QWidget *bookmarksTab;
    QWidget *indexTab;
    QListView *kdevelop_view;
    QListView *libraries_view;
    QListView *bookmarks_view;
    QCheckBox *indexshownlibs_box;
    QCheckBox *indexhiddenlibs_box;
    QCheckBox *indexbookmarks_box;
    QListView *indexedtocs_view;
    void readConfig();
    void storeConfig();
};

#endif
