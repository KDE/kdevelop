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
#include "kdevprojectmanager_part.h"
#include "kdevprojectmanager_widget.h"

#include <kdevprojectimporter.h>
#include <kdevprojectbuilder.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>

#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kurl.h>

#include <qlayout.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qheader.h>
#include <qsplitter.h>

class KDevToolBar: public KToolBar
{
public:
    KDevToolBar(QWidget *parent);
    virtual ~KDevToolBar();

    virtual void setMovingEnabled( bool b );
};

class KDevToolBarShell: public QFrame
{
public:
    KDevToolBarShell(QWidget *parent);
    virtual ~KDevToolBarShell();
    
    void setToolBar(KDevToolBar *tb);

private:
    KDevToolBar *m_tb;

protected:
    virtual void resizeEvent ( QResizeEvent * );
};


KDevToolBar::KDevToolBar(QWidget *parent)
    : KToolBar( parent, "KDevToolbar", true )
{
    setMinimumWidth(10);
}

KDevToolBar::~KDevToolBar()
{}

void KDevToolBar::setMovingEnabled( bool)
{
    KToolBar::setMovingEnabled(false);
}


KDevToolBarShell::KDevToolBarShell(QWidget *parent)
    : QFrame(parent),
      m_tb(0)
{}

KDevToolBarShell::~KDevToolBarShell()
{}

void KDevToolBarShell::setToolBar(KDevToolBar *tb)
{
    m_tb=tb;
}

void KDevToolBarShell::resizeEvent ( QResizeEvent * )
{
    if (m_tb)
    {
        setMinimumHeight(m_tb->sizeHint().height());
        m_tb->resize(width(),height());
    }
}


// ----------------------------------------------------------------------------------
KDevProjectManagerWidget::KDevProjectManagerWidget(KDevProjectManagerPart *part)
    : QVBox(0, "kdevprojectmanager widget"),
      m_part(part)
{
    m_actionReload = new KAction(i18n("Reload"), SmallIcon("reload"), 0, this, SLOT(reload()),
        part->actionCollection(), "project_reload");
        
    m_actionBuildAll = new KAction(i18n("Build All"), SmallIcon("launch"), Key_F8, this, SLOT(buildAll()),
        part->actionCollection(), "project_buildall");
        
    m_actionBuild = new KAction(i18n("Build"), SmallIcon("launch"), SHIFT + Key_F8, this, SLOT(build()),
        part->actionCollection(), "project_build");

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    m_overview = new ProjectOverview(this, splitter);
    m_details = new ProjectDetails(this, splitter);
    
    connect(m_overview->listView(), SIGNAL(selectionChanged(QListViewItem*)), 
        this, SLOT(updateDetails(QListViewItem*)));        
}

KDevProjectManagerWidget::~KDevProjectManagerWidget()
{
}

void KDevProjectManagerWidget::buildAll()
{
    m_overview->buildAll();
}

void KDevProjectManagerWidget::build()
{
    m_details->build();
}

void KDevProjectManagerWidget::updateDetails(QListViewItem *item)
{
    kdDebug(9000) << "KDevProjectManagerWidget::updateDetails()" << endl;
    m_details->setCurrentItem(item ? static_cast<ProjectViewItem*>(item)->dom() : 0);
}


// ---------------------------------------------------------------------------------
class ProjectRoot: public ProjectViewItem
{
public:
    ProjectRoot(ProjectView *parent)
        : ProjectViewItem(ProjectItemDom(), parent),
          m_projectView(parent)
    { m_projectView->listView()->takeItem(this); }

    virtual ProjectView *projectView() const
    { return m_projectView; }
    
    virtual void insertItem(QListViewItem *item)
    { m_projectView->listView()->insertItem(item); }
    
private:
    ProjectView *m_projectView;
};

ProjectView::ProjectView(KDevProjectManagerWidget *m, QWidget *parentWidget)
    : QVBox(parentWidget), 
      m_managerWidget(m)
{
    m_toolBarShell = new KDevToolBarShell(this);
    m_toolBar = new KDevToolBar(m_toolBarShell);
    m_toolBarShell->setToolBar(m_toolBar);
    
    m_toolBar->setMovingEnabled(false);
    m_toolBar->setFlat(true);
    
    m_listView = new KListView(this);
    fake_root = new ProjectRoot(this);
    
    m_listView->header()->hide();
    m_listView->addColumn(QString::null);
    m_listView->setRootIsDecorated(QListView::LastColumn);
    m_listView->setResizeMode(QListView::LastColumn);
    
    connect(m_listView, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(executed(QListViewItem*)));
    connect(m_listView, SIGNAL(executed(QListViewItem*)), this, SLOT(executed(QListViewItem*)));    
}

ProjectView::~ProjectView()
{
    delete fake_root;
    fake_root = 0;
}

KToolBar *ProjectView::toolBar() const
{
    return m_toolBar;
}

void ProjectView::refresh()
{
    listView()->clear();
    // ###
}

void ProjectView::insertItem(ProjectItemDom dom)
{
    Q_UNUSED(dom)    
    // ###
}

void ProjectView::removeItem(ProjectItemDom dom)
{
    Q_UNUSED(dom)    
    // ###
}

void ProjectView::process(ProjectItemDom dom, ProjectViewItem::ProcessOperation op)
{
    fake_root->process(dom, op);
}

// ---------------------------------------------------------------------------------
ProjectViewItem::ProjectViewItem(ProjectItemDom dom, ProjectViewItem *parent)
    : QListViewItem(parent),
      m_dom(dom),
      m_projectView(parent->projectView())
{
}

ProjectViewItem::ProjectViewItem(ProjectItemDom dom, ProjectView *parent)
    : QListViewItem(parent->listView()),
      m_dom(dom),
      m_projectView(parent)
{
}

ProjectViewItem::~ProjectViewItem()
{
}

ProjectView *ProjectViewItem::projectView() const
{
    return m_projectView;
}

void ProjectViewItem::process(ProjectItemDom dom, ProcessOperation op)
{
    Q_ASSERT(dom);
    
    if (ProjectWorkspaceDom workspace = dom->toWorkspace())
        processWorkspace(workspace, op);
    else if (ProjectTargetDom target = dom->toTarget())
        processTarget(target, op);
    else if (ProjectFolderDom folder = dom->toFolder())
        processFolder(folder, op);
    else if (ProjectFileDom file = dom->toFile())
        processFile(file, op);
    else 
        Q_ASSERT(0);
}

void ProjectViewItem::processWorkspace(ProjectWorkspaceDom dom, ProcessOperation op)
{
    Q_ASSERT(dom);    
    processFolder(dom->toFolder(), op);
}

void ProjectViewItem::processFolder(ProjectFolderDom dom, ProcessOperation op)
{
    Q_ASSERT(dom);
    Q_ASSERT(projectView());
    
    if (ProjectViewItem *item = projectView()->createProjectItem(dom->toItem(), this)) {
        ProjectFolderList folder_list = dom->folderList();
        for (ProjectFolderList::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
            item->processFolder(*it, op);
            
        ProjectFileList file_list = dom->fileList();
        for (ProjectFileList::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            item->processFile(*it, op);
            
        ProjectTargetList target_list = dom->targetList();
        for (ProjectTargetList::Iterator it = target_list.begin(); it != target_list.end(); ++it)
            item->processTarget(*it, op);        
    }
}

void ProjectViewItem::processTarget(ProjectTargetDom dom, ProcessOperation op)
{
    Q_ASSERT(dom);

    if (ProjectViewItem *item = projectView()->createProjectItem(dom->toItem(), this)) {
        ProjectFileList file_list = dom->fileList();
        for (ProjectFileList::Iterator it = file_list.begin(); it != file_list.end(); ++it)
            item->processFile(*it, op);
    }
}

void ProjectViewItem::processFile(ProjectFileDom dom, ProcessOperation op)
{
    Q_ASSERT(dom);
    Q_UNUSED(op);

    (void) projectView()->createProjectItem(dom->toItem(), this);
}

void ProjectViewItem::processModel(FileDom dom, ProcessOperation op)
{
    Q_UNUSED(dom);
    Q_UNUSED(op);
}

ProjectViewItem *ProjectView::createProjectItem(ProjectItemDom dom, ProjectViewItem *parent)
{
    Q_ASSERT(dom);

    ProjectViewItem *item = new ProjectViewItem(dom, parent);
    item->setText(0, dom->shortDescription());
    return item;
}

void ProjectViewItem::setup()
{
    QListViewItem::setup();
    
    if (dom()) {    
        if (ProjectWorkspaceDom workspace = dom()->toWorkspace())
            setPixmap(0, SmallIcon("window"));
        else if (ProjectFolderDom folder = dom()->toFolder())
            setPixmap(0, SmallIcon("folder"));
        else if (ProjectTargetDom target = dom()->toTarget())
            setPixmap(0, SmallIcon("target_kdevelop"));
        else if (ProjectFileDom file = dom()->toFile())
            setPixmap(0, SmallIcon("document"));
    }
}

void ProjectViewItem::setOpen(bool opened)
{
    QListViewItem::setOpen(opened);
}

// ---------------------------------------------------------------------------------
ProjectOverview::ProjectOverview(KDevProjectManagerWidget *manager, QWidget *parentWidget)
    : ProjectView(manager, parentWidget)
{
    if (KToolBar *tb = toolBar()) {
        part()->actionCollection()->action("project_buildall")->plug(tb);
        tb->insertSeparator();
        part()->actionCollection()->action("project_reload")->plug(tb);
        
#if 0 // ###
        tb->insertButton(SmallIcon("folder_new"), -1, true);
        tb->insertButton(SmallIcon("targetnew_kdevelop"), -1, true);
        tb->insertButton(SmallIcon("window_new"), -1, true);
        tb->insertButton(SmallIcon("launch"), -1, true);
        tb->insertButton(SmallIcon("configure"), -1, true);
#endif
    }
    
    connect(part(), SIGNAL(refresh()),
        this, SLOT(refresh()));
    
    connect(part(), SIGNAL(addedProjectItem(ProjectItemDom)),
        this, SLOT(insertItem(ProjectItemDom)));
    connect(part(), SIGNAL(aboutToRemoveProjectItem(ProjectItemDom)),
        this, SLOT(removeItem(ProjectItemDom)));        
        
    connect(listView(), SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
        this, SLOT(contextMenu(KListView *, QListViewItem *, const QPoint &)));
}

ProjectOverview::~ProjectOverview()
{
}

ProjectViewItem *ProjectOverview::createProjectItem(ProjectItemDom dom, ProjectViewItem *parent)
{
    Q_ASSERT(dom);
    
    ProjectViewItem *item = 0;
    if (dom->toFolder()) {
        item = ProjectView::createProjectItem(dom, parent);
        item->setOpen(true);
    }
        
    return item;
}

void KDevProjectManagerWidget::reload()
{
    kdDebug(9000) << "KDevProjectManagerWidget::reload()" << endl;
    m_overview->reload();
}

void ProjectOverview::buildAll()
{
    kdDebug(9000) << "ProjectOverview::buildAll()" << endl;
    
    part()->partController()->saveAllFiles();
    
    if (KDevProjectBuilder *builder = part()->defaultBuilder()) {
        ProjectItemList item_list = projectModel()->itemList();
        for (ProjectItemList::Iterator it = item_list.begin(); it != item_list.end(); ++it)
            builder->build(*it);
    }
}

void ProjectOverview::reload()
{
    kdDebug(9000) << "ProjectOverview::reload()" << endl;
    part()->import(KDevProjectManagerPart::ForceRefresh);
}

void ProjectOverview::refresh()
{
    kdDebug(9000) << "ProjectOverview::refresh()" << endl;

    QString currentText;
    if (listView()->selectedItem())
        currentText = listView()->selectedItem()->text(0);
        
    ProjectView::refresh();

    ProjectItemList item_list = projectModel()->itemList();
    for (ProjectItemList::Iterator it = item_list.begin(); it != item_list.end(); ++it)
        process(*it, ProjectViewItem::Insert);
        
    if (!currentText.isEmpty()) {
        if (QListViewItem *item = listView()->findItem(currentText, 0, KListView::ExactMatch))
            listView()->setSelected(item, true);
    } else {
        listView()->setSelected(listView()->firstChild(), true);
    }
}

// ---------------------------------------------------------------------------------
ProjectDetails::ProjectDetails(KDevProjectManagerWidget *parent, QWidget *parentWidget)
    : ProjectView(parent, parentWidget)
{
    if (KToolBar *tb = toolBar()) {
        part()->actionCollection()->action("project_build")->plug(tb);

#if 0 // ### 
        m_actionBuild->plug(tb);
        
        tb->insertButton(SmallIcon("filenew"), -1, true);
        tb->insertButton(SmallIcon("fileimport"), -1, true);
        tb->insertButton(SmallIcon("editdelete"), -1, true);
        tb->insertButton(SmallIcon("launch"), -1, true);
        tb->insertButton(SmallIcon("exec"), -1, true);
        tb->insertButton(SmallIcon("configure"), -1, true);
#endif
    }

    connect(listView(), SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
        this, SLOT(contextMenu(KListView *, QListViewItem *, const QPoint &)));
}

ProjectDetails::~ProjectDetails()
{
}

void ProjectDetails::build()
{
    if (KDevProjectBuilder *builder = part()->defaultBuilder()) {
        builder->build(m_currentItem);
    }
}

void ProjectDetails::setCurrentItem(ProjectItemDom dom)
{
    m_currentItem = dom;
    
    QString currentText;
    if (QListViewItem *sel = listView()->currentItem()) {
        currentText = sel->text(0);
    }

    ProjectView::refresh();
    
    if (dom && dom->toFolder()) {
        ProjectTargetList target_list = dom->toFolder()->targetList();

        for (ProjectTargetList::Iterator it = target_list.begin(); it != target_list.end(); ++it) {
            process((*it)->toItem());
        }
    }

    if (!currentText.isEmpty()) {
        if (QListViewItem *item = listView()->findItem(currentText, 0, KListView::ExactMatch)) {
            listView()->setSelected(item, true);
            while (item) {
                item->setOpen(true);
                item = item->parent();
            }
        }
    } else {
        listView()->setSelected(listView()->firstChild(), true);
    }
}

void ProjectView::open(ProjectItemDom dom)
{
    Q_ASSERT(dom);
    
    if (ProjectFileDom file = dom->toFile()) {
        part()->partController()->editDocument(KURL(file->name()));
    }
}

void ProjectView::showProperties(ProjectItemDom dom)
{
    Q_ASSERT(dom);
    // ### implement me ;)
}

void ProjectView::showProperties(QListViewItem *item)
{
    if (ProjectViewItem *projectItem = static_cast<ProjectViewItem*>(item)) {
        showProperties(projectItem->dom());
    }
}

void ProjectView::executed(QListViewItem *item)
{
    if (ProjectViewItem *projectItem = static_cast<ProjectViewItem*>(item)) {
        open(projectItem->dom());
    }
}

ProjectViewItem *ProjectView::findProjectItem(const QString &path) const
{
    return fake_root->findProjectItem(path);
}

ProjectViewItem *ProjectViewItem::findProjectItem(const QString &path) const
{
    if (dom() && dom()->name() == path)
        return const_cast<ProjectViewItem*>(this);
        
    ProjectViewItem *current = static_cast<ProjectViewItem*>(firstChild());
    while (current) {
        if (ProjectViewItem *i = current->findProjectItem(path))
            return i;
            
        current = static_cast<ProjectViewItem*>(current->nextSibling());
    }
    
    return 0;
}

void ProjectOverview::contextMenu(KListView *listView, QListViewItem *item, const QPoint &pt)
{
    Q_UNUSED(listView);
    
    Q_ASSERT(part()->defaultImporter());

    ProjectViewItem *projectItem = static_cast<ProjectViewItem*>(item);
    
    if (ProjectFolderDom folder = projectItem->dom()->toFolder()) { 
        QString makefile = part()->defaultImporter()->findMakefile(folder);
        if (!makefile.isEmpty()) {
            KPopupMenu menu(this);
            menu.insertTitle(i18n("Folder: %1").arg(folder->shortDescription()));
            
            menu.insertItem(i18n("Edit"), 1000);
            
            if (part()->defaultBuilder()) {
                menu.insertSeparator();
                menu.insertItem(i18n("Build"), 1010);
            }
            
            switch (menu.exec(pt)) {
                case 1000: {
                    part()->partController()->editDocument(KURL(makefile));
                } break;
                
                case 1010: {
                    if (KDevProjectBuilder *builder = part()->defaultBuilder())
                        builder->build(projectItem->dom());
                } break;
                
                default: break;
            } // end switch
        }
    } 
}

void ProjectDetails::contextMenu(KListView *listView, QListViewItem *item, const QPoint &pt)
{
    Q_UNUSED(listView);
    
    if (!item)
        return;
        
    ProjectViewItem *projectItem = static_cast<ProjectViewItem*>(item);
        
    if (ProjectFileDom file = projectItem->dom()->toFile()) {
        KPopupMenu menu(this);
        menu.insertTitle(i18n("File: %1").arg(file->shortDescription()));
                    
        FileContext context(file->name(), false);
        part()->core()->fillContextMenu(&menu, &context);
        
        if (part()->defaultBuilder()) {
            menu.insertSeparator();
            menu.insertItem(i18n("Build"), 1010);
        }
        
        switch (menu.exec(pt)) {
            case 1010: {
                if (KDevProjectBuilder *builder = part()->defaultBuilder())
                    builder->build(m_currentItem); // ### TODO: compile the target not subproject
            } break;
            
            default: break;
        } // end switch
    }
}


#include "kdevprojectmanager_widget.moc"
