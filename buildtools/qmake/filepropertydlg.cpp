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

#include <qmessagebox.h>
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

FilePropertyDlg::FilePropertyDlg(SubqmakeprojectItem *spitem,int grtype, FileItem *fitem, QStringList &dirtyScopes,
                                 QWidget *parent, const char* name, bool modal, WFlags fl)
: FilePropertyBase(parent,name,modal,fl),
m_dirtyScopes(dirtyScopes)
{
  if (grtype == GroupItem::InstallObject)
  {
    GroupItem* gitem = dynamic_cast<GroupItem*>(fitem->parent());
    if (gitem)
      m_installObjectName = gitem->install_objectname;    
  }
  m_gtype = grtype;
  m_subProjectItem = spitem;
  m_fileItem = fitem;
  ScopeTree->setRootIsDecorated(true);
  createScopeTree(m_subProjectItem);
}

GroupItem* FilePropertyDlg::getInstallRoot(SubqmakeprojectItem* item)
{
  QPtrListIterator<GroupItem> it(item->groups);
  for (;it.current();++it)
  {
    if ((*it)->groupType == GroupItem::InstallRoot)
      return *it;
  }
  return 0;
}

GroupItem* FilePropertyDlg::getInstallObject(SubqmakeprojectItem* item, const QString& objectname)
{
  GroupItem* instroot = getInstallRoot(item);
  if (!instroot)
    return 0;
  QPtrListIterator<GroupItem> it(instroot->installs);
  for (;it.current();++it)
  {
    if ((*it)->groupType == GroupItem::InstallObject &&
        (*it)->install_objectname == objectname )
      return *it;
  }
  return 0;
  
}


QStringList* FilePropertyDlg::getExcludeList(SubqmakeprojectItem *spitem)
{
    if (m_gtype == GroupItem::Sources)
      return &(spitem->sources_exclude);
    if (m_gtype == GroupItem::Headers)
      return &(spitem->headers_exclude);
    if (m_gtype == GroupItem::Forms)
      return &(spitem->forms_exclude);
    if (m_gtype == GroupItem::Images)
      return &(spitem->images_exclude);
    if (m_gtype == GroupItem::IDLs)
      return &(spitem->idls_exclude);
    if (m_gtype == GroupItem::Translations)
      return &(spitem->translations_exclude);
    if (m_gtype == GroupItem::Yaccsources)
      return &(spitem->yaccsources_exclude);
    if (m_gtype == GroupItem::Lexsources)
      return &(spitem->lexsources_exclude);
    if (m_gtype == GroupItem::Distfiles)
      return &(spitem->distfiles_exclude);
    if (m_gtype == GroupItem::InstallObject)
    {
      GroupItem* gitem = getInstallObject(spitem,m_installObjectName);
      if (gitem)
        return &(gitem->str_files_exclude);
      return &m_dummy;
    }
    return NULL;
}

void FilePropertyDlg::createScopeTree(SubqmakeprojectItem *spitem,ScopeItem *sitem)
{
  QPtrListIterator<SubqmakeprojectItem> spit(spitem->scopes);
  for (; spit.current(); ++spit)
  {
    SubqmakeprojectItem *nextSubproject = spit;
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
      {
        if (m_gtype != GroupItem::InstallObject)
          sitem->excludeFromScope(m_fileItem->name,true);
      }
      else
      {
        if (m_gtype != GroupItem::InstallObject)
          sitem->excludeFromScope(m_fileItem->name,false);
      }
      scopes.append(sitem->getScopeString());
      kdDebug(9024) << "dirty scope - " << sitem->getScopeString() << endl;
    }
    scopes += getExcludedScopes(sitem);
    sitem = static_cast<ScopeItem*>(sitem->nextSibling());
  }
  return scopes;
}

