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

#include "../cproject.h"
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
  tagtypeComboBox->insertItem(QString("all"));
  tagtypeComboBox->insertItem(QString("files"));
  tagtypeComboBox->insertItem(QString("definitions"));
  tagtypeComboBox->insertItem(QString("declarations"));
}

/*
 *  Destroys the object and frees any allocated resources
 */
searchTagsDialogImpl::~searchTagsDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Set the current Tag type in the dialog combo box
 */
void searchTagsDialogImpl::setTagType(tagType type)
{
  tagtypeComboBox->setCurrentItem(type);
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
  gotoTag(&m_currentTagList[i]);
}
/* 
 * public slot
 */
void searchTagsDialogImpl::slotClear()
{
  m_currentTagList.clear();
  tagsListBox->clear();
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotSearchTag()
{
  kdDebug() << "searchTagsDialogImpl::slotSearchTag():\n";
  QString searchtext = searchTagLineEdit->text();

  CProject* prj = currentProject();
  CTagsDataBase& tagsDB = prj->ctagsDataBase();
  if (tagsDB.is_initialized()) {
    kdDebug() << "found tags data base\n";
    const CTagList* taglist = tagsDB.ctaglist(searchtext);
    if (taglist)
    {
      int ntags = taglist->count();
      kdDebug() << "found: " << ntags << " entries for: "
                << searchtext << "\n";
      setSearchResult(*taglist);
    }
  }
}
/*
 * internal method
 */
void searchTagsDialogImpl::setSearchResult(const CTagList& taglist)
{
  tagType currentTagType = (tagType)tagtypeComboBox->currentItem();
  searchTagLineEdit->setText(taglist.tag());
  int ntags = taglist.count();
  bool append = false;
  for (int it=0; it<ntags; ++it)
  {
    const CTag& ctag = taglist[it];
    switch (currentTagType) {
      case file:
        if (ctag.isFile()) append = true;
      break;
      case definition:
        if (ctag.isDefinition()) append = true;
      break;
      case declaration:
        if (ctag.isDeclaration()) append = true;
      break;
      case all:
        append = true;
      default:
      break;
    }
    if (append) {
      m_currentTagList.QValueList<CTag>::append(ctag);
      QString line = ctag.typeName() + ", in ";
      line = line + ctag.file();
      line = line + ", line: ";
      line = line + QString::number(ctag.line());
      tagsListBox->insertItem(line);
    }
    append = false;
  }
}
/*
 * internal method
 */
void searchTagsDialogImpl::gotoTag(const CTag* tag)
{
  if (!tag) {
    kdDebug() << "searchTagsDialogImpl::slotTagGotoFile, "
              << "Error: null tag\n" ;
    return;
  }
  kdDebug() << "searchTagsDialogImpl::slotTagGotoFile, "
            << "open file " << tag->file()
            << " at line "  << tag->line()  << "\n";
  int line = tag->line();
  if (tag->isFile()&&(line<=1)) line = -1;
  kdDebug() << "emit signal: switchToFile("
            <<  tag->file() << "," << line << ");\n";
  emit switchToFile(tag->file(),line);
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotGotoFile(QString text)
{
  slotGotoTagType(file,text);
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotGotoDefinition(QString text)
{
  slotGotoTagType(definition,text);
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotGotoDeclaration(QString text)
{
  slotGotoTagType(declaration,text);
}
/*
 * public slot
 */
void searchTagsDialogImpl::slotGotoTagType(tagType type, QString text)
{
  kdDebug() << "searchTagsDialogImpl::gotoTagType searching for "
            << text << "\n";
  slotClear();
  setTagType(type);
  searchTagLineEdit->setText(text);
  slotSearchTag();
  if (m_currentTagList.count()==1) {
    gotoTag(&m_currentTagList[0]);
  }
  else {
    show();
    raise();
  }
}
