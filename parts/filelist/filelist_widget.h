/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FILELIST_WIDGET_H__
#define __FILELIST_WIDGET_H__


#include <klistview.h>
#include <kurl.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qtimer.h>

#include "filelist_part.h"
		 
class KDevProject;
class FileListItem;

namespace KParts { class Part; }

class FileListWidget : public KListView, public QToolTip
{
  Q_OBJECT
    
public:
		  
	FileListWidget(FileListPart *part);
  	~FileListWidget();
	
protected:
  	void maybeTip( QPoint const & );

private slots:
//	void partAdded(KParts::Part*);
//	void partRemoved();
	void activePartChanged(KParts::Part*);
	void itemClicked( QListViewItem * );
	void popupMenu( QListViewItem * , const QPoint & , int );
	void closeSelectedFiles();
	void saveSelectedFiles();
	void reloadSelectedFiles();
	void documentChangedState( const KURL &, DocumentState );
	void refreshFileList();
	void startRefreshTimer();

private:
	KURL::List getSelectedURLs();
	FileListItem * itemForURL( KURL const & url );

	QTimer m_refreshTimer;
	FileListPart * _part;

};


#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
