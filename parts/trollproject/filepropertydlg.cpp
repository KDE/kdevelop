/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
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
#include <kdebug.h>

ScopeItem::ScopeItem(QListView *parent,const QString &text,QStringList *excludeList,bool initialMode)
: QCheckListItem(parent,text,QCheckListItem::CheckBox)
{
  m_excludeList = excludeList;
  setOn(initialMode);
  m_initialMode = initialMode;
}

ScopeItem::ScopeItem(QCheckListItem *parent,const QString &text,QStringList *excludeList,bool initialMode)
: QCheckListItem(parent,text,QCheckListItem::CheckBox)
{
  m_excludeList = excludeList;
  setOn(initialMode);
  m_initialMode = initialMode;
}

bool ScopeItem::isDirty()
{
  if (m_initialMode==isOn())
    return false;  // no change totally
  return true; // changed
}

void ScopeItem::excludeFromScope(const QString &fileName,bool b)
{
  if (!m_excludeList)
    return;
  // remove entries of same text before appending to ensure only one resulting entry
  m_excludeList->remove(fileName);
  if (b)
    m_excludeList->append(fileName);
}

FilePropertyDlg::FilePropertyDlg(SubprojectItem *spitem,int grtype, FileItem *fitem, QStringList &dirtyScopes,
                                 QWidget *parent, const char* name, bool modal, WFlags fl)
: FilePropertyBase(parent,name,modal,fl),
m_dirtyScopes(dirtyScopes)
{
  m_gtype = grtype;
  m_subProjectItem = spitem;
  m_fileItem = fitem;
  ScopeTree->setRootIsDecorated(true);
  createScopeTree(m_subProjectItem);
}

QStringList* FilePropertyDlg::getExcludeList(SubprojectItem *spitem)
{
    if (m_gtype == GroupItem::Sources)
      return &(spitem->sources_exclude);
    if (m_gtype == GroupItem::Headers)
      return &(spitem->headers_exclude);
    if (m_gtype == GroupItem::Forms)
      return &(spitem->forms_exclude);
    return NULL;
}

void FilePropertyDlg::createScopeTree(SubprojectItem *spitem,ScopeItem *sitem)
{
  QListIterator<SubprojectItem> spit(spitem->scopes);
  for (; spit.current(); ++spit)
  {
    SubprojectItem *nextSubproject = spit;
    QStringList *excludeList = getExcludeList(nextSubproject);
    if (!excludeList)
      continue;
    // check if file is already excluded in current scope
    bool initiallyExcluded = false;
    if (excludeList->find(m_fileItem->name)!=excludeList->end())
      initiallyExcluded = true;
    ScopeItem *item;
    if (!sitem)
       item = new ScopeItem(ScopeTree,nextSubproject->text(0),excludeList,initiallyExcluded);
    else
       item = new ScopeItem(sitem,nextSubproject->text(0),excludeList,initiallyExcluded);
    item->setScopeString(nextSubproject->scopeString);
    if (!sitem)
      ScopeTree->insertItem(item);
    else
      sitem->insertItem(item);
    createScopeTree(nextSubproject,item);
  }
}


void FilePropertyDlg::updateFileProperties()
{
  m_dirtyScopes = getExcludedScopes();
  accept();
}

QStringList FilePropertyDlg::getExcludedScopes(ScopeItem *sitem)
{
  QStringList scopes;
  if (!sitem)
    sitem = static_cast<ScopeItem*>(ScopeTree->firstChild());
  else
    sitem = static_cast<ScopeItem*>(sitem->firstChild());
  while (sitem)
  {
    if (sitem->isDirty())
    {
      if (sitem->isOn())
        sitem->excludeFromScope(m_fileItem->name,true);
      else
        sitem->excludeFromScope(m_fileItem->name,false);
      scopes.append(sitem->getScopeString());
      kdDebug(9024) << "dirty scope - " << sitem->getScopeString() << endl;
    }
    scopes += getExcludedScopes(sitem);
    sitem = static_cast<ScopeItem*>(sitem->nextSibling());
  }
  return scopes;
}

