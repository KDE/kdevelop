/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEWIDGET_H_
#define _DOCTREEWIDGET_H_

#include <qmap.h>
#include <qstringlist.h>
#include <qvbox.h>

#include "klistview.h"


class DocTreeItem;
class DocTreeDoxygenFolder;
class DocTreeBookmarksFolder;
class DocTreeProjectFolder;
class DocTreeDocbaseFolder;
class DocTreeQtFolder;
class DocTreeKDELibsFolder;
class DocTreeTocFolder;
class DocTreeViewPart;
class KDevProject;
class CustomizeDialog;
class QToolButton;
class KListView;
class KAction;
class KHistoryCombo;


class DocTreeViewWidget : public QVBox
{
    Q_OBJECT
    
public: 
    DocTreeViewWidget(DocTreeViewPart *part);
    ~DocTreeViewWidget();

    void configurationChanged();
    void projectChanged(KDevProject *project);
    
    static QString locatehtml(const QString &fileName);

private slots:
    void refresh();
    void slotConfigure();
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
	void slotJumpToNextMatch();
	void slotJumpToPrevMatch();
	void slotStartSearching();
	void slotHistoryReturnPressed ( const QString& );
	void slotSelectionChanged ( QListViewItem* );
	void slotAddBookmark();
	void slotRemoveBookmark();

protected:
	void searchForItem ( const QString& );
    bool initKDocKDELibs();

private: 
    bool kdelibskdoc;
    KListView* docView;
    QPtrList<QListViewItem> searchResultList;

    QListViewItem *contextItem;
    DocTreeQtFolder *folder_qt;
    DocTreeDoxygenFolder *folder_doxygen;
    DocTreeKDELibsFolder *folder_kdelibs;
    DocTreeBookmarksFolder *folder_bookmarks;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;
    QPtrList<DocTreeTocFolder> folder_toc;
    DocTreeViewPart *m_part;

	QHBox* searchToolbar;

	QToolButton* docConfigButton;
	QToolButton* prevButton;
	QToolButton* nextButton;
	QToolButton* startButton;
	KHistoryCombo* completionCombo;
	
	DocTreeItem* m_activeTreeItem;
	
	KAction* docConfigAction;
};
#endif
