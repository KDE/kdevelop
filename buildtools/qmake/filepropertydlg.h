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

class QCheckListItem;
class QListViewItem;
class FilePropertyBase;
class FileItem;
class SubprojectItem;
class GroupItem;

class ScopeItem : public QCheckListItem
{
  public:
                  ScopeItem(QCheckListItem *parent,const QString &text,QStringList *excludeList,bool initialMode);
                  ScopeItem(QListView *parent,const QString &text,QStringList *excludeList,bool initialMode);
  QString&        getScopeString() {return m_scopeString;}
  void            setScopeString(const QString &scopeString) {m_scopeString = scopeString;}
  void            excludeFromScope(const QString &fileName,bool b);
  bool            isDirty();

  private:
  QStringList     *m_excludeList; // pointer all the way into the right SubprojectItem excludelist
  QString         m_scopeString;
  bool            m_initialMode;
};

class FilePropertyDlg : public FilePropertyBase
{
  public:
                  FilePropertyDlg(SubprojectItem *spitem, int grtype,FileItem *fitem,
                                  QStringList &dirtyScopes,QWidget *parent=0,
                                  const char* name = 0,bool modal = FALSE, WFlags fl = 0);

  private:
  void            createScopeTree(SubprojectItem *spitem,ScopeItem *sitem=0);
  GroupItem*      getInstallRoot(SubprojectItem *spitem);
  GroupItem*      getInstallObject(SubprojectItem *spitem,const QString& objectname);
  QStringList*    getExcludeList(SubprojectItem *spitem);
  QStringList     getExcludedScopes(ScopeItem *sitem=0);
  FileItem        *m_fileItem;
  SubprojectItem  *m_subProjectItem;
  QStringList     &m_dirtyScopes;
  QStringList     m_dummy;
  QString         m_installObjectName;
  int             m_gtype;

  public slots:
  virtual void updateFileProperties();
};

#endif
