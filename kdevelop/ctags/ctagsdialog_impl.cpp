/***************************************************************************
                          ctagsdialog_impl.cpp
                          --------------------
    begin                : Wed April 26 2001
    copyright            : (C) 2001 by rokrau, the kdevelop-team
    email                : rokrau@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ctagsdialog_impl.h"
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <kdebug.h>

/* 
 *  Constructs a searchTagsDialogImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
searchTagsDialogImpl::searchTagsDialogImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : searchTagsDialog( parent, name, modal, fl ), m_currentTagList()
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
searchTagsDialogImpl::~searchTagsDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot slotLBItemSelected(int i)
 * The slot gets called upon double click on an item in the
 * QListBox, it emits a signal that is connected to a method
 * in CKdevelop which in turn opens the file at the appropriate
 * line.
 */
void searchTagsDialogImpl::slotLBItemSelected(int i)
{
  kdDebug() << "in searchTagsDialogImpl::slotLBItemSelected() \n";
  kdDebug() << "selected item: " << i << "\n";
  emit gotoTag(&m_currentTagList[i]);
}
/* 
 * public slot
 */
void searchTagsDialogImpl::slotClear()
{
  tagsListBox->clear();
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotSearchTag()
{
  kdDebug() << "searchTagsDialogImpl::slotSearchTag():\n";
  QString text = searchTagLineEdit->text();

}

void searchTagsDialogImpl::setSearchResult(const CTagList& taglist)
{
  tagsListBox->clear();
  searchTagLineEdit->setText(taglist.tag());
  int ntags = taglist.count();
  for (int it=0; it<ntags; ++it)
  {
    const CTag& tag = taglist[it];
    QString line = tag.kind() + ", in ";
    line = line + tag.file();
    line = line + ", line: ";
    line = line + QString::number(tag.line());
    tagsListBox->insertItem(line);
  }
  m_currentTagList = taglist;
}
