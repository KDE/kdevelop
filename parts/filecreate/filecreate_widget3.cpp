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

#include <qptrlist.h>
#include <qwhatsthis.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdevcore.h>

#include "filecreate_widget3.h"

#include "kdevproject.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"
#include "filecreate_listitem.h"

namespace FileCreate {

  ListWidget::ListWidget(FileCreatePart *part)
    : KListView(0, "KDevFileCreate"), TypeChooser(part)
  {
    setIcon( SmallIcon("filenew2") );
    setCaption(i18n("File Create"));
    setResizeMode( AllColumns );
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);

    addColumn("");
    addColumn("");

    QWhatsThis::add(this, i18n("<b>New file</b><p>This part makes the creation of new files easier. Select a type in the list to create a file. "
        "The list of project file types can be configured in project settings dialog, <b>New File Wizard</b> tab. "
        "Globally available file types are listed and can be configured in KDevelop settings dialog, <b>New File Wizard</b> tab."));


    connect( this, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotTypeSelected(QListViewItem*)) );
  }


  ListWidget::~ListWidget()
  {
  }

  void ListWidget::setCurrent(const FileType * current) {

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

  void ListWidget::resizeEvent(QResizeEvent *event) {
    ListItem *li = dynamic_cast<ListItem*>(firstChild());
    while(li) {
      li->prepareResize();
      li = dynamic_cast<ListItem*>(li->nextSibling());
    }
    KListView::resizeEvent(event);
  }

  void ListWidget::refresh() {
    clear();
    QPtrList<FileType> filetypes = m_part->getFileTypes();
    for(FileType * filetype = filetypes.first();
	filetype!=NULL;
	filetype=filetypes.next()) {
      if (filetype->enabled()) {
	QPtrList<FileType> subtypes = filetype->subtypes();
        if (subtypes.count()==0)
          new ListItem( this, filetype );
	for(FileType * subtype = subtypes.first();
	    subtype!=NULL;
	    subtype=subtypes.next()) {
	  if (subtype->enabled())
	    new ListItem( this, subtype );
	}
      }
    }
  }

  void ListWidget::slotTypeSelected(QListViewItem * item) {
    ListItem * fileitem = dynamic_cast<ListItem*>(item);
    if (!fileitem) return;

    const FileType * filetype = fileitem->filetype();

    TypeChooser::filetypeSelected(filetype);
  }


}
#include "filecreate_widget3.moc"
