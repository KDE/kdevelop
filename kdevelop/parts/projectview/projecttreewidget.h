/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROJECTTREEWIDGET_H_
#define _PROJECTTREEWIDGET_H_

#include <qlistview.h>

class ProjectView;
class ProjectSpace;

class ProjectTreeWidget : public QListView
{
    Q_OBJECT
    
public:
    ProjectTreeWidget(ProjectView* pPart);
    ~ProjectTreeWidget();
    void setProjectSpace(ProjectSpace* pProjectSpace);
 protected:
    
    ProjectSpace* m_pProjectSpace;
};

class ProjectTreeItem : public QListViewItem {
    public:
  ProjectTreeItem (ProjectTreeWidget* parent) : QListViewItem(parent) {}
  ProjectTreeItem (ProjectTreeItem* parent) : QListViewItem(parent) {}
  ProjectTreeItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : QListViewItem(parent,after) {}
  ProjectTreeItem (ProjectTreeItem* parent,ProjectTreeItem* after) : QListViewItem(parent,after) {}
};

class ProjectSpaceItem : public  ProjectTreeItem {
public:
  ProjectSpaceItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  ProjectSpaceItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  ProjectSpaceItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  ProjectSpaceItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class ProjectItem : public  ProjectTreeItem {
public:
  ProjectItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  ProjectItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  ProjectItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  ProjectItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class GroupItem : public  ProjectTreeItem {
public:
  GroupItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  GroupItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  GroupItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  GroupItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class FileItem : public  ProjectTreeItem {
public:
  FileItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  FileItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  FileItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  FileItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  void setAbsFileName(QString fileName);
  QString absFileName();
protected:
  QString m_absFileName;
};


#endif
