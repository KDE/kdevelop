/***************************************************************************
                          flatdirtreeview.h  -  description
                             -------------------
    begin                : Thu Oct 5 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FLATDIRTREEVIEW_H
#define FLATDIRTREEVIEW_H

#include <qwidget.h>
#include <qlistview.h>

/**
  *@author Sandy Meier
  */

class FlatDirTreeView : public QListView  {
   Q_OBJECT
public: 
	FlatDirTreeView(QWidget *parent=0, const char *name=0);
	~FlatDirTreeView();
	void setDirLocation(QString absPath);
	QString currentDir();

	protected slots:
     void slotClicked(QListViewItem* pItem);
	
 signals:
	void dirSelected(QString absPath);
 private:
	QString m_currentDir;
};

#endif
