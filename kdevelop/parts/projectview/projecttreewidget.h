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
#include "projectspace.h"
#include <qlist.h>
#include <qstring.h>
#include <kaction.h>
#include "kdevactions.h"

class ProjectView;
class QPopupMenu;
class ProjectTreeItem;


class ProjectTreeWidget : public QListView
{
    Q_OBJECT
    
public:
    ProjectTreeWidget(ProjectView* pPart);
    ~ProjectTreeWidget();
    void setProjectSpace(ProjectSpace* pProjectSpace);

    // from KDevComponent
    void readProjectSpaceGlobalConfig(QDomDocument& doc);
    /**
       <Project name = "">
       <FileGroup name="" filter ="" />
       <FileGroup name="" filter ="" />
       </project>
       <project name = "">
       <FileGroup name="" filter ="" />
       <FileGroup name="" filter ="" />
       </project>
    */
    void writeProjectSpaceGlobalConfig(QDomDocument& doc);
    
    protected slots:
      void slotRightButtonPressed( QListViewItem* item, const QPoint&,int);
    void    slotOpenFile();
 protected:
    void createDefaultFileGroups();
    QPopupMenu* createPopup(ProjectTreeItem* item);
    ProjectSpace* m_pProjectSpace;
    // the "document"
    QMap<QString,QList< FileGroup> > m_projectFileGroups;

    ProjectView* m_pProjectView;
};

class ProjectTreeItem : public QObject, public QListViewItem {
  Q_OBJECT
    public:
  ProjectTreeItem (ProjectTreeWidget* parent) : QListViewItem(parent) {m_bold=false;}
  ProjectTreeItem (ProjectTreeItem* parent) : QListViewItem(parent) {m_bold=false;}
  ProjectTreeItem (ProjectTreeWidget* parent,ProjectTreeItem* after) 
    : QListViewItem(parent,after) {m_bold=false;}
  ProjectTreeItem (ProjectTreeItem* parent,ProjectTreeItem* after) 
    : QListViewItem(parent,after) {m_bold=false;}
  virtual void paintCell( QPainter * p, const QColorGroup & cg,
			  int column, int width, int align );

  /** set the font to bold for this treeItem if enable=true*/
  void setBold(bool enable);
 protected:
  bool m_bold;
};

class ProjectSpaceItem : public  ProjectTreeItem {
  Q_OBJECT
public:
  ProjectSpaceItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  ProjectSpaceItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  ProjectSpaceItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  ProjectSpaceItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class ProjectItem : public  ProjectTreeItem {
  Q_OBJECT
public:
  ProjectItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  ProjectItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  ProjectItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  ProjectItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class GroupItem : public  ProjectTreeItem {
  Q_OBJECT
public:
  GroupItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  GroupItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  GroupItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  GroupItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
};

class FileItem : public  ProjectTreeItem {
  Q_OBJECT
public:
  FileItem (ProjectTreeWidget* parent) : ProjectTreeItem(parent) {}
  FileItem (ProjectTreeItem* parent) : ProjectTreeItem(parent) {}
  FileItem (ProjectTreeWidget* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  FileItem (ProjectTreeItem* parent,ProjectTreeItem* after) : ProjectTreeItem(parent,after) {}
  void setAbsFileName(QString fileName);
  void setProjectName(QString projectName);
  QString absFileName();
  QString projectName();
protected:
  QString m_absFileName;
  QString m_projectName;
};


#endif
