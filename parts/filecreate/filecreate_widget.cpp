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

namespace FileCreate {

  TreeWidget::TreeWidget(FileCreatePart *part)
    : KListView(0, "filecreate widget"), TypeChooser(part)
  {
  
    addColumn("File Type");
    addColumn("Extension");

    setRootIsDecorated(true);

    QWhatsThis::add(this, i18n("This part makes the creation of new files within the project easier."));

  
    connect( this, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotTypeSelected(QListViewItem*)) );
  }


  TreeWidget::~TreeWidget()
  {
  }

  void TreeWidget::setCurrent(const FileType * current) {

    bool found = false;
    QListViewItem * lvi = firstChild();
    while(lvi && !found) {
      ListItem * li = dynamic_cast<ListItem*>(lvi);
      if (li) {
	if (li->filetype()==current) {
	  found=true;
	  setSelected(li,true);
	}
      }
      if (lvi->nextSibling()) 
	lvi = lvi->nextSibling();
      else {
	while (lvi && !lvi->nextSibling())
	  lvi = lvi->parent();
      }
    }

  }


  void TreeWidget::refresh() {
    clear();
    QPtrList<FileType> filetypes = m_part->getFileTypes();
    for(FileType * filetype = filetypes.first();
	filetype!=NULL;
	filetype=filetypes.next()) {
      if (filetype->enabled()) {
	ListItem * listitem = new ListItem( this, filetype );
	QPtrList<FileType> subtypes = filetype->subtypes();
	for(FileType * subtype = subtypes.first();
	    subtype!=NULL;
	    subtype=subtypes.next()) {
	  if (subtype->enabled())
	    new ListItem( listitem, subtype );
	}
      }
    }
  }

  void TreeWidget::slotTypeSelected(QListViewItem * item) {
    ListItem * fileitem = dynamic_cast<ListItem*>(item);
    if (!fileitem) return;
  
    const FileType * filetype = fileitem->filetype();

    TypeChooser::filetypeSelected(filetype);
  }
  

}

