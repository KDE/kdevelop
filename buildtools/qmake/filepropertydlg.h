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

#ifndef FILEPROPERTYDLG_H
#define FILEPROPERTYDLG_H

#include "trollprojectwidget.h"
#include "filepropertybase.h"

class Q3CheckListItem;
class Q3ListViewItem;
class FilePropertyBase;
class FileItem;
class SubqmakeprojectItem;
class GroupItem;

class ScopeItem : public Q3CheckListItem
{
  public:
                  ScopeItem(Q3CheckListItem *parent,const QString &text,QStringList *excludeList,bool initialMode);
                  ScopeItem(Q3ListView *parent,const QString &text,QStringList *excludeList,bool initialMode);
  QString&        getScopeString() {return m_scopeString;}
  void            setScopeString(const QString &scopeString) {m_scopeString = scopeString;}
  void            excludeFromScope(const QString &fileName,bool b);
  bool            isDirty();

  private:
  QStringList     *m_excludeList; // pointer all the way into the right SubqmakeprojectItem excludelist
  QString         m_scopeString;
  bool            m_initialMode;
};

class FilePropertyDlg : public FilePropertyBase
{
  public:
                  FilePropertyDlg(SubqmakeprojectItem *spitem, int grtype,FileItem *fitem,
                                  QStringList &dirtyScopes,QWidget *parent=0,
                                  const char* name = 0,bool modal = FALSE, Qt::WFlags fl = 0);

  private:
  void            createScopeTree(SubqmakeprojectItem *spitem,ScopeItem *sitem=0);
  GroupItem*      getInstallRoot(SubqmakeprojectItem *spitem);
  GroupItem*      getInstallObject(SubqmakeprojectItem *spitem,const QString& objectname);
  QStringList*    getExcludeList(SubqmakeprojectItem *spitem);
  QStringList     getExcludedScopes(ScopeItem *sitem=0);
  FileItem        *m_fileItem;
  SubqmakeprojectItem  *m_subProjectItem;
  QStringList     &m_dirtyScopes;
  QStringList     m_dummy;
  QString         m_installObjectName;
  int             m_gtype;

  public slots:
  virtual void updateFileProperties();
};

#endif
