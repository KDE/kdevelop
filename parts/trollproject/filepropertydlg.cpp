/***************************************************************************
 *   Copyright (C) 2001 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filepropertydlg.h"
#include <kmessagebox.h>

ScopeItem::ScopeItem(QListView *parent,const QString &text)
: QCheckListItem(parent,text,QCheckListItem::CheckBox)
{
}

ScopeItem::ScopeItem(QCheckListItem *parent,const QString &text)
: QCheckListItem(parent,text,QCheckListItem::CheckBox)
{
}


FilePropertyDlg::FilePropertyDlg(SubprojectItem *spitem, FileItem *fitem,QWidget *parent, const char* name, bool modal, WFlags fl)
: FilePropertyBase(parent,name,modal,fl)
{
  m_subProjectItem = spitem;
  m_fileItem = fitem;
  ScopeTree->setRootIsDecorated(true);
  createScopeTree("");
}


void FilePropertyDlg::diveIntoScope(ScopeItem *sitem,const QString &scopeString)
{
  FileBuffer *subBuf = m_subProjectItem->m_RootBuffer->getSubBuffer(scopeString);
  // just in case
  if (!subBuf)
    return;
  QStringList childScopes = subBuf->getChildScopeNames();
  for (int i=0 ;i<childScopes.count(); i++)
  {
    ScopeItem *item = new ScopeItem(sitem,childScopes[i]);
    item->setScopeString(scopeString+":"+childScopes[i]);
    sitem->insertItem(item);
    diveIntoScope(item,scopeString+":"+childScopes[i]);
  }
}

void FilePropertyDlg::createScopeTree(const QString &scopeString)
{
  FileBuffer *subBuf = m_subProjectItem->m_RootBuffer->getSubBuffer(scopeString);
  // just in case
  if (!subBuf)
    return;
  QStringList childScopes = subBuf->getChildScopeNames();
  for (int i=0 ;i<childScopes.count(); i++)
  {
    ScopeItem *item = new ScopeItem(ScopeTree,childScopes[i]);
    item->setScopeString(childScopes[i]);
    ScopeTree->insertItem(item);
    diveIntoScope(item,childScopes[i]);
  }
}

void FilePropertyDlg::updateFileProperties()
{
  KMessageBox::error(this,i18n(getExcludedScopes()));
  accept();
}

QString FilePropertyDlg::getExcludedScopes(ScopeItem *sitem)
{
  QString scopes;
  if (!sitem)
    sitem = static_cast<ScopeItem*>(ScopeTree->firstChild());
  else
    sitem = static_cast<ScopeItem*>(sitem->firstChild());
  while (sitem)
  {
    if (sitem->isOn())
      scopes = scopes + sitem->getScopeString() + "\n";
    scopes = scopes + getExcludedScopes(sitem);
    sitem = static_cast<ScopeItem*>(sitem->nextSibling());
  }
  return scopes;
}

