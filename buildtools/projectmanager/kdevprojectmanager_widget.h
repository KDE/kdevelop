/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __KDEVPROJECTMANAGER_WIDGET_H__
#define __KDEVPROJECTMANAGER_WIDGET_H__

#include "kdevprojectmodel.h"
#include "kdevprojectmanager_part.h"

#include <qvbox.h>

#include <kxmlguibuilder.h>
#include <klistview.h>

class KDevProject;
class KDevProjectManagerPart;
class KDevToolBar;
class KDevToolBarShell;
class KToolBar;

class ProjectModel;
class ProjectView;
class ProjectViewItem;
class ProjectOverview;
class ProjectDetails;

class KDevProjectManagerWidget: public QVBox
{
    Q_OBJECT    
public:
    KDevProjectManagerWidget(KDevProjectManagerPart *part);
    virtual ~KDevProjectManagerWidget();

    inline KDevProjectManagerPart *part() const
    { return m_part; }

    inline ProjectModel *projectModel() const
    { return part()->projectModel(); }

protected slots:
    void updateDetails(QListViewItem *item);

private:
    KDevProjectManagerPart *m_part;
    ProjectOverview *m_overview;
    ProjectDetails *m_details;
};

class ProjectViewItem: public QListViewItem
{
public:
    enum ProcessOperation
    {
        Insert,
        Remove
    };
    
public:
    ProjectViewItem(ProjectItemDom dom, ProjectViewItem *parent);
    ProjectViewItem(ProjectItemDom dom, ProjectView *parent);
    virtual ~ProjectViewItem();
    
    virtual ProjectView *projectView() const;
    
    inline ProjectItemDom dom() const
    { return m_dom; }
    
    virtual ProjectViewItem *findProjectItem(const QString &path) const;
    
    virtual void setup();
    virtual void setOpen(bool opened);
    
    virtual void process(ProjectItemDom dom, ProcessOperation op = Insert);
    virtual void processWorkspace(ProjectWorkspaceDom dom, ProcessOperation op = Insert);
    virtual void processFolder(ProjectFolderDom dom, ProcessOperation op = Insert);
    virtual void processTarget(ProjectTargetDom dom, ProcessOperation op = Insert);
    virtual void processFile(ProjectFileDom dom, ProcessOperation op = Insert);
    virtual void processModel(FileDom dom, ProcessOperation op = Insert);
            
private:
    ProjectItemDom m_dom;
    QMap<ProjectItemDom, ProjectViewItem*> m_items;
    ProjectView *m_projectView;
};

class ProjectView: public QVBox,
    public KXMLGUIBuilder, 
    virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    ProjectView(KDevProjectManagerWidget *m, QWidget *parentWidget = 0);
    virtual ~ProjectView();
    
    inline KDevProjectManagerWidget *managerWidget() const
    { return m_managerWidget; }
    
    inline KDevProjectManagerPart *part() const
    { return managerWidget()->part(); }
    
    inline ProjectModel *projectModel() const
    { return part()->projectModel(); }

    inline KListView *listView() const
    { return m_listView; }
        
    KToolBar *toolBar() const;
    
    virtual ProjectViewItem *createProjectItem(ProjectItemDom dom, ProjectViewItem *parent);
    virtual ProjectViewItem *findProjectItem(const QString &path) const;
    
    virtual void process(ProjectItemDom dom, ProjectViewItem::ProcessOperation op = ProjectViewItem::Insert);
    
public slots:
    virtual void refresh();
    virtual void insertItem(ProjectItemDom dom);
    virtual void removeItem(ProjectItemDom dom);
    virtual void open(ProjectItemDom dom);

private slots:
    void executed(QListViewItem *item);
    
private:    
    KListView *m_listView;
    ProjectViewItem *fake_root;
    KDevProjectManagerWidget *m_managerWidget;
    KDevToolBarShell *m_toolBarShell;
    KDevToolBar *m_toolBar;
};

class ProjectOverview: public ProjectView
{
    Q_OBJECT
public:
    ProjectOverview(KDevProjectManagerWidget *parent, QWidget *parentWidget = 0);
    virtual ~ProjectOverview();

    virtual ProjectViewItem *createProjectItem(ProjectItemDom dom, ProjectViewItem *parent);
    virtual void refresh();    
};

class ProjectDetails: public ProjectView
{
    Q_OBJECT
public:
    ProjectDetails(KDevProjectManagerWidget *parent, QWidget *parentWidget = 0);
    virtual ~ProjectDetails();
    
public slots:
    void setCurrentItem(ProjectItemDom dom);
};

#endif
