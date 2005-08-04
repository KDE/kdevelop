/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_WIDGET3_H__
#define __FILECREATE_WIDGET3_H__


#include <qwidget.h>
#include <qstring.h>
//Added by qt3to4:
#include <QResizeEvent>

#include <klistview.h>

#include "filecreate_typechooser.h"

class KDevProject;
class FileCreatePart;
class QResizeEvent;

namespace FileCreate {

  class FileType;

  class ListWidget : public KListView, public TypeChooser
    {
      Q_OBJECT

      public:

        ListWidget(FileCreatePart *part);
        virtual ~ListWidget();

	virtual void refresh();
	virtual void setCurrent(const FileType * current);
        virtual void resizeEvent(QResizeEvent *event);

      public slots:
	void slotTypeSelected(Q3ListViewItem * item);

    };

}

#endif
