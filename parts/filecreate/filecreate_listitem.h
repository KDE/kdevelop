/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_LISTITEM_H__
#define __FILECREATE_LISTITEM_H__

#include <qlistview.h>
#include <qobject.h>

#include "filecreate_filetype.h"

class FileCreateListItem : public QListViewItem {

public:
  FileCreateListItem(QListView * listview, const FileCreateFileType * filetype);
  FileCreateListItem(FileCreateListItem * listitem, const FileCreateFileType * filetype);
  virtual ~FileCreateListItem() { }

  const FileCreateFileType * filetype() const { return m_filetype; }
  
private:
  const FileCreateFileType * m_filetype;
  
};

#endif

