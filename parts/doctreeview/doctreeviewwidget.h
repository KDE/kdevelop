/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
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
class DocTreeKDELibsFolder;
class DocTreeDoxygenFolder;
class DocTreeBookmarksFolder;
class DocTreeProjectFolder;
class DocTreeDocbaseFolder;
class DocTreeTocFolder;
class DocTreeViewPart;
class KDevProject;
class CustomizeDialog;
class QToolButton;
class KListView;
class KAction;
class KHistoryCombo;
class QHBox;
class QLabel;


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
    void slotConfigure();
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void refresh();
	void slotJumpToNextMatch();
	void slotJumpToPrevMatch();
	void slotStopSearching();
	void slotStartSearching();
	void slotHistoryReturnPressed ( const QString& );
	void slotSelectionChanged ( QListViewItem* );
	void slotShowButtonToggled ( bool );
	
protected:
	DocTreeItem* searchForItem ( DocTreeItem*, const QString& );

private: 
	KListView* docView;

    QListViewItem *contextItem;
    DocTreeItem *folder_kdevelop;
    DocTreeKDELibsFolder *folder_kdelibs;
    DocTreeDoxygenFolder *folder_doxygen;
    DocTreeBookmarksFolder *folder_bookmarks;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;
    QList<DocTreeTocFolder> folder_toc;
    DocTreeViewPart *m_part;

	QHBox* docToolbar;
	QHBox* searchToolbar;
	
	QLabel* hLine;

	QToolButton* docConfigButton;
	QToolButton* showButton;
	QToolButton* prevButton;
	QToolButton* nextButton;
	QToolButton* startButton;
	QToolButton* stopButton;
	KHistoryCombo* completionCombo;
	
	bool m_keepSearching;
	DocTreeItem* m_activeTreeItem;
	
	QStringList historyList;
	QMap < QString, DocTreeItem* > historyMap;

	KAction* docConfigAction;
};
#endif
