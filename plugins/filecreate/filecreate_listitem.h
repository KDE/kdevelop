/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *   thanks: Roberto Raggi for QSimpleRichText stuff                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_LISTITEM_H__
#define __FILECREATE_LISTITEM_H__

#include <klistview.h>
#include <qobject.h>

#include "filecreate_filetype.h"

class QSimpleRichText;

namespace FileCreate {

class ListItem : public KListViewItem {

public:
  ListItem(QListView *listview, const FileType *filetype);
  ListItem(ListItem *listitem, const FileType *filetype);
  virtual ~ListItem();

  const FileType *filetype() const { return m_filetype; }

  virtual void setup();
  virtual void setHeight( int height );
  virtual void paintCell( QPainter* p, const QColorGroup& gc, int column, int width, int align );
  virtual void prepareResize();

private:
  const FileType *m_filetype;
  int m_iconHeight;
  void init();
  QSimpleRichText *m_filetypeRenderer;

};

}

#endif

