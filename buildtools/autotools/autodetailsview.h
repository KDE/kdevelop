/***************************************************************************
                             -------------------
    begin                : 19.01.2003
    copyright            : (C) 2002 by Victor Rder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUTODETAILSVIEW_H
#define AUTODETAILSVIEW_H

#include <qwidget.h>

#include <klistview.h>

#include "domutil.h"
#include "autolistviewitems.h"

class KAction;

class AutoProjectPart;
class AutoProjectWidget;

class AutoDetailsView : public KListView
{
	friend class RemoveFileDialog;
	friend class FileItem;
	Q_OBJECT

	public:
		AutoDetailsView(AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name);
		virtual ~AutoDetailsView();

		QString getUiFileLink(const QString &path, const QString& filename);
	
	protected:
		void initActions ();
		virtual void focusOutEvent(QFocusEvent *e);
		
	private slots:
		void slotDetailsExecuted(QListViewItem *item);
		void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);
		
		void slotTargetOptions ();
		void slotAddNewFile();
		void slotAddExistingFile();
		void slotAddIcon();
		void slotBuildTarget();
		void slotExecuteTarget();
		void slotRemoveDetail();
		void slotSetActiveTarget();
		
	private:
		AutoProjectWidget* m_widget;
		AutoProjectPart* m_part;

		DomUtil::PairList m_subclasslist;
		
		KAction* targetOptionsAction;
		KAction* addNewFileAction;
		KAction* addExistingFileAction;
		KAction* addIconAction;
		KAction* buildTargetAction;
		KAction* executeTargetAction;		
		KAction* setActiveTargetAction;
		KAction* removeDetailAction;

};

#endif
