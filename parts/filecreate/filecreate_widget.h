/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_WIDGET_H__
#define __FILECREATE_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>

#include <klistview.h>

#include "filecreate_typechooser.h"

class KDevProject;
class FileCreatePart;

namespace FileCreate {

  class FileType;

  class TreeWidget : public KListView, public TypeChooser
    {
      Q_OBJECT
    
      public:
		  
        TreeWidget(FileCreatePart *part);
        virtual ~TreeWidget();

	virtual void refresh();
	virtual void setCurrent(const FileType * current);

      public slots:
	void slotTypeSelected(QListViewItem * item);

    };

}

#endif
