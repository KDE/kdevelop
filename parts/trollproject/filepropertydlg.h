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

#ifndef FILEPROPERTYDLG_H
#define FILEPROPERTYDLG_H

#include "trollprojectwidget.h"
#include "filepropertybase.h"

class QCheckListItem;
class QListViewItem;
class FilePropertyBase;
class FileItem;
class SubprojectItem;

class ScopeItem : public QCheckListItem
{
  public:
                  ScopeItem(QCheckListItem *parent,const QString &text);
                  ScopeItem(QListView *parent,const QString &text);
  QString&        getScopeString() {return m_scopeString;}
  void            setScopeString(const QString &scopeString) {m_scopeString = scopeString;}

  private:
  QString         m_scopeString;
};

class FilePropertyDlg : public FilePropertyBase
{
  public:
                  FilePropertyDlg(SubprojectItem *spitem, FileItem *fitem,QWidget *parent=0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);

  private:
  void            createScopeTree(const QString &scopeString);
  void            diveIntoScope(ScopeItem *sitem,const QString &scopeString);
  QString         getExcludedScopes(ScopeItem *sitem=0);
  FileItem        *m_fileItem;
  SubprojectItem  *m_subProjectItem;

  public slots:
  virtual void updateFileProperties();
};

#endif
