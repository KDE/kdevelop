/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qptrlist.h>
#include <qwhatsthis.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <kdevcore.h>

#include "kdevproject.h"
#include "filecreate_part.h"
#include "filecreate_widget.h"
#include "filecreate_filetype.h"
#include "filecreate_listitem.h"

#include <iostream>

FileCreateWidget::FileCreateWidget(FileCreatePart *part)
  : KListView(0, "filecreate widget"), FileCreateTypeChooser(part)
{
  
  addColumn("File Type");
  addColumn("Extension");

  setRootIsDecorated(true);

  QWhatsThis::add(this, i18n("This part makes the creation of new files within the project easier."));

  
  connect( this, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotTypeSelected(QListViewItem*)) );
}


FileCreateWidget::~FileCreateWidget()
{
}

void FileCreateWidget::refresh() {
  clear();
  QPtrList<FileCreateFileType> filetypes = m_part->getFileTypes();
  for(FileCreateFileType * filetype = filetypes.first();
      filetype!=NULL;
      filetype=filetypes.next()) {
    if (filetype->enabled()) {
      FileCreateListItem * listitem = new FileCreateListItem( this, filetype );
      QPtrList<FileCreateFileType> subtypes = filetype->subtypes();
      for(FileCreateFileType * subtype = subtypes.first();
          subtype!=NULL;
          subtype=subtypes.next()) {
        if (subtype->enabled())
          new FileCreateListItem( listitem, subtype );
      }
    }
  }
}

void FileCreateWidget::slotTypeSelected(QListViewItem * item) {
  FileCreateListItem * fileitem = dynamic_cast<FileCreateListItem*>(item);
  if (!fileitem) return;
  
  const FileCreateFileType * filetype = fileitem->filetype();

  filetypeSelected(filetype);
}
  


#include "filecreate_widget.moc"
