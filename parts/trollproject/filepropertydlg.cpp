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
  updateScopeTree();
}

void FilePropertyDlg::updateScopeTree()
{
  FileBuffer *subBuf = m_subProjectItem->m_RootBuffer->getSubBuffer(m_subProjectItem->scopeString);
  QStringList childScopes = subBuf->getChildScopeNames();
  for (int i=0 ;i<childScopes.count(); i++)
  {
    ScopeItem *item = new ScopeItem(ScopeTree,childScopes[i]);
    item->setScopeString(m_subProjectItem->scopeString+":"+childScopes[i]);
    ScopeTree->insertItem(item);
  }
}

