/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include "filecreate_listitem.h"

#include <kglobal.h>
#include <kiconloader.h>
//#include <kicon.h>

#include <qpixmap.h>

namespace FileCreate {

ListItem::ListItem(QListView * listview, const FileType * filetype) :
  QListViewItem(listview, filetype->name(), filetype->ext() ), m_filetype(filetype) {

  KIconLoader * loader = KGlobal::iconLoader();
  QPixmap iconPix = loader->loadIcon(filetype->icon(), KIcon::Small, 0,
                                     KIcon::DefaultState, NULL,
                                     true);
  if (!iconPix.isNull())
    setPixmap(0, iconPix);
  
}


ListItem::ListItem(ListItem * listitem, const FileType * filetype) :
  QListViewItem(listitem, filetype->name(), filetype->ext() ), m_filetype(filetype) {

  KIconLoader * loader = KGlobal::iconLoader();
  QPixmap iconPix = loader->loadIcon(filetype->icon(), KIcon::Small, 0,
                                     KIcon::DefaultState, NULL,
                                     true);
  if (!iconPix.isNull())
    setPixmap(0, iconPix);

}

}


