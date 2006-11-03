/*
  KDevelop Autotools Support
  Copyright (c) 2002 by Victor Roeder <victor_roeder@gmx.de>
  Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
*/

#ifndef AUTODETAILSVIEW_H
#define AUTODETAILSVIEW_H

#include "autoprojectviewbase.h"

#include "domutil.h"
#include "autolistviewitems.h"

class KAction;

class AutoProjectPart;
class AutoProjectWidget;
class AutoToolsAction;

class AutoDetailsView : protected AutoProjectViewBase
{
	friend class RemoveFileDialog;
	friend class FileItem;
	Q_OBJECT

public:
	AutoDetailsView( AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name );
	virtual ~AutoDetailsView();

	QString getUiFileLink( const QString &path, const QString& filename );
	KListView* listView()
	{
		return m_listView;
	}

public slots:
	void slotSelectionChanged( QListViewItem* item );
	void slotSelectionChanged();

signals:
	void selectionChanged( QListViewItem* );

protected:
	void initActions ();
	virtual void focusOutEvent( QFocusEvent *e );

private slots:
	void slotDetailsExecuted( QListViewItem *item );
	void slotDetailsContextMenu( KListView *, QListViewItem *item, const QPoint &p );

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

	AutoToolsAction* targetOptionsAction;
	AutoToolsAction* addNewFileAction;
	AutoToolsAction* addExistingFileAction;
	KAction* addIconAction;
	AutoToolsAction* buildTargetAction;
	AutoToolsAction* executeTargetAction;
	KAction* setActiveTargetAction;
	AutoToolsAction* removeDetailAction;

};

#endif
// kate: indent-mode csands; tab-width 4;
