/***************************************************************************
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2000-2001 by Trolltech AS.                              *
 *   info@trolltech.com                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *                                                                         *
 *   Part of this file is taken from Qt Designer.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trollprojectwidget.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qdir.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <kfiledialog.h>
#include <qtooltip.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>
#include <qmessagebox.h>
#include <iostream>
#include <kparts/part.h>
#include <kaction.h>
#include <kprocess.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "trollprojectpart.h"
#include "kdevappfrontend.h"
#include "kdevmakefrontend.h"
#include "kdevlanguagesupport.h"
#include "kdevcreatefile.h"
#include "subclassesdlg.h"

#define VALUES_PER_ROW  1

/**
 * Class ProjectViewItem
 */

ProjectItem::ProjectItem(Type type, QListView *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


ProjectItem::ProjectItem(Type type, ProjectItem *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


/**
 * Class SubprojectItem
 */

SubprojectItem::SubprojectItem(QListView *parent, const QString &text, const QString &scopeString)
    : ProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    configuration.m_template = QTMP_APPLICATION;
    init();
}


SubprojectItem::SubprojectItem(SubprojectItem *parent, const QString &text,const QString &scopeString)
    : ProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    init();
}

SubprojectItem::~SubprojectItem()
{
}
QString SubprojectItem::getRelativPath()
{
  if(this->parent()==NULL||this->parent()->parent()==NULL) return("/"+configuration.m_subdirName);
  else return(((SubprojectItem*)this->parent())->getRelativPath()+"/"+configuration.m_subdirName);
}
QString SubprojectItem::getDownDirs()
{
  if(this->parent()==NULL) return("./");
  else return(((SubprojectItem*)this->parent())->getDownDirs()+"../");
}
QString SubprojectItem::getLibAddObject(QString downDirs)
{
  if(configuration.m_requirements & QD_SHARED)
  {
    if(configuration.m_target!="")
    {
      return("-l"+configuration.m_target);
    }else{
      return("-l"+this->subdir);
    }
  }else if(configuration.m_requirements & QD_STATIC)
  {
    QString tmpPath;
    if(configuration.m_destdir!="")
    {
      tmpPath=downDirs+this->subdir+configuration.m_destdir;
    }else{
      tmpPath=downDirs+this->getRelativPath()+"/";      
    }

    tmpPath=QDir::cleanDirPath(tmpPath);

    QString libString;
    if(configuration.m_target!="")
    {
      libString = tmpPath+"/lib"+this->configuration.m_target+".a";
      
    }else{
      libString = tmpPath+"/lib"+this->configuration.m_subdirName+".a";
      
    }
    return(libString);
  }

  return("");
}
QString SubprojectItem::getLibAddPath(QString downDirs)
{

  //PATH only add if shared lib
  if(!(configuration.m_requirements & QD_SHARED))return("");

    QString tmpPath;
    if(configuration.m_destdir!="")
    {
      tmpPath=downDirs+this->subdir+configuration.m_destdir;
    }else{
      tmpPath=downDirs+this->getRelativPath()+"/";
    }

    tmpPath=QDir::cleanDirPath(tmpPath);
    
  return(tmpPath);

}
QString SubprojectItem::getIncAddPath(QString downDirs)
{
  QString tmpPath=downDirs+this->getRelativPath();
  tmpPath=QDir::cleanDirPath(tmpPath);

  return(tmpPath);
}

void SubprojectItem::init()
{
  configuration.m_template = QTMP_APPLICATION;
  configuration.m_warnings = QWARN_ON;
  configuration.m_buildMode = QBM_RELEASE;
  configuration.m_requirements = QD_QT;
  groups.setAutoDelete(true);
  if (scopeString.isEmpty())
  {
    isScope = false;
  }
  else
  {
    isScope = true;
    setPixmap(0, SmallIcon("qmake_scope"));
  }
}


/**
 * Class GroupItem
 */

GroupItem::GroupItem(QListView *lv, GroupType type, const QString &text, const QString &scopeString)
    : ProjectItem(Group, lv, text)
{
    this->scopeString = scopeString;
    groupType = type;
    files.setAutoDelete(true);
    setPixmap(0, SmallIcon("tar"));
}

GroupItem::GroupType GroupItem::groupTypeForExtension(const QString &ext)
{
    if (ext == "cpp" || ext == "cc" || ext == "c" || ext == "C" || ext == "c++" || ext == "cxx")
        return Sources;
    else if (ext == "hpp" || ext == "h" || ext == "hxx" || ext == "hh" || ext == "h++" || ext == "H")
        return Headers;
    else if (ext == "ui")
        return Forms;
    else
        return NoType;
}

/**
 * Class FileItem
 */

FileItem::FileItem(QListView *lv, const QString &text, bool exclude/*=false*/)
    : ProjectItem(File, lv, text)
{
    // if excluded is set the file is excluded in the subproject/project.
    // by default excluded is set to false, thus file is included
    excluded = exclude;
    setPixmap(0, SmallIcon("document"));
}


TrollProjectWidget::TrollProjectWidget(TrollProjectPart *part)
    : QVBox(0, "troll project widget")
{
    QSplitter *splitter = new QSplitter(Vertical, this);

    //////////////////
    // PROJECT VIEW //
    //////////////////

    overviewContainer = new QVBox(splitter,"Projects");
    overviewContainer->setMargin ( 2 );
    overviewContainer->setSpacing ( 2 );
    projectTools = new QHBox(overviewContainer,"Project buttons");
    projectTools->setMargin ( 2 );
    projectTools->setSpacing ( 2 );
    // Add subdir
    addSubdirButton = new QToolButton ( projectTools, "Make button" );
    addSubdirButton->setPixmap ( SmallIcon ( "folder_new",22 ) );
    addSubdirButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, addSubdirButton->sizePolicy().hasHeightForWidth() ) );
    addSubdirButton->setEnabled ( true );
    QToolTip::add( addSubdirButton, i18n( "Add new subdirectory..." ) );
    // Create scope
    createScopeButton = new QToolButton ( projectTools, "Make button" );
    createScopeButton->setPixmap ( SmallIcon ( "qmake_scopenew",22 ) );
    createScopeButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, createScopeButton->sizePolicy().hasHeightForWidth() ) );
    createScopeButton->setEnabled ( true );
    QToolTip::add( createScopeButton, i18n( "Create scope..." ) );
    // build selected file
    buildFileButton = new QToolButton ( projectTools, "Make file button" );
    buildFileButton->setPixmap ( SmallIcon ( "compfile",22 ) );
    buildFileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildFileButton->sizePolicy().hasHeightForWidth() ) );
    buildFileButton->setEnabled ( true );
    QToolTip::add( buildFileButton, i18n( "Build file" ) );
    // build
    buildProjectButton = new QToolButton ( projectTools, "Make button" );
    buildProjectButton->setPixmap ( SmallIcon ( "make_kdevelop",22 ) );
    buildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildProjectButton->sizePolicy().hasHeightForWidth() ) );
    buildProjectButton->setEnabled ( true );
    QToolTip::add( buildProjectButton, i18n( "Build" ) );
    // rebuild
    rebuildProjectButton = new QToolButton ( projectTools, "Rebuild button" );
    rebuildProjectButton->setPixmap ( SmallIcon ( "rebuild",22 ) );
    rebuildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, rebuildProjectButton->sizePolicy().hasHeightForWidth() ) );
    rebuildProjectButton->setEnabled ( true );
    QToolTip::add( rebuildProjectButton, i18n( "Rebuild" ) );
    // run
    executeProjectButton = new QToolButton ( projectTools, "Run button" );
    executeProjectButton->setPixmap ( SmallIcon ( "exec",22 ) );
    executeProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, executeProjectButton->sizePolicy().hasHeightForWidth() ) );
    executeProjectButton->setEnabled ( true );
    QToolTip::add( executeProjectButton, i18n( "Run" ) );
    // spacer
    QWidget *spacer = new QWidget(projectTools);
    projectTools->setStretchFactor(spacer, 1);
    // Project configuration
    projectconfButton = new QToolButton ( projectTools, "Project configuration button" );
    projectconfButton->setPixmap ( SmallIcon ( "configure",22 ) );
    projectconfButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, projectconfButton->sizePolicy().hasHeightForWidth() ) );
    projectconfButton->setEnabled ( true );
    QToolTip::add( projectconfButton, i18n( "Configure the project" ) );

    // Project button connections
    connect ( addSubdirButton, SIGNAL ( clicked () ), this, SLOT ( slotAddSubdir () ) );
    connect ( createScopeButton, SIGNAL ( clicked () ), this, SLOT ( slotCreateScope () ) );
    connect ( buildFileButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildFile () ) );
    
    connect ( buildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildProject () ) );
    connect ( rebuildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotRebuildProject () ) );
    connect ( executeProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotExecuteProject () ) );


    
    connect ( projectconfButton, SIGNAL ( clicked () ), this, SLOT ( slotConfigureProject () ) );

    // Project tree
    overview = new KListView(overviewContainer, "project overview widget");
    overview->setResizeMode(QListView::LastColumn);
    overview->setSorting(-1);
    overview->header()->hide();
    overview->addColumn(QString::null);

    // Project tree connections
    connect( overview, SIGNAL(selectionChanged(QListViewItem*)),
             this, SLOT(slotOverviewSelectionChanged(QListViewItem*)) );
    connect( overview, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotOverviewContextMenu(KListView*, QListViewItem*, const QPoint&)) );


    /////////////////
    // DETAIL VIEW //
    /////////////////

    // Details tree
    detailContainer = new QVBox(splitter,"Details");
    detailContainer->setMargin ( 2 );
    detailContainer->setSpacing ( 2 );

    // Details Toolbar
    fileTools = new QHBox(detailContainer,"Detail buttons");
    fileTools->setMargin ( 2 );
    fileTools->setSpacing ( 2 );

    // Add existing files button
    addfilesButton = new QToolButton ( fileTools, "Add existing files" );
    addfilesButton->setPixmap ( SmallIcon ( "fileimport",22 ) );
    addfilesButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, addfilesButton->sizePolicy().hasHeightForWidth() ) );
    addfilesButton->setEnabled ( true );
    QToolTip::add( addfilesButton, i18n( "Add existing files" ) );

    // Add new file button
    newfileButton = new QToolButton ( fileTools, "Add new file" );
    newfileButton->setPixmap ( SmallIcon ( "filenew",22 ) );
    newfileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, newfileButton->sizePolicy().hasHeightForWidth() ) );
    newfileButton->setEnabled ( true );
    QToolTip::add( newfileButton, i18n( "Add new files" ) );

    // remove file button
    removefileButton = new QToolButton ( fileTools, "Remove file" );
    removefileButton->setPixmap ( SmallIcon ( "button_cancel",22 ) );
    removefileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, removefileButton->sizePolicy().hasHeightForWidth() ) );
    removefileButton->setEnabled ( true );
    QToolTip::add( removefileButton, i18n( "Remove file" ) );






    // build
    buildTargetButton = new QToolButton ( fileTools, "Make button" );
    buildTargetButton->setPixmap ( SmallIcon ( "make_kdevelop",22 ) );
    buildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildTargetButton->sizePolicy().hasHeightForWidth() ) );
    buildTargetButton->setEnabled ( true );
    QToolTip::add( buildTargetButton, i18n( "Build" ) );
    // rebuild
    rebuildTargetButton = new QToolButton ( fileTools, "Rebuild button" );
    rebuildTargetButton->setPixmap ( SmallIcon ( "rebuild",22 ) );
    rebuildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, rebuildTargetButton->sizePolicy().hasHeightForWidth() ) );
    rebuildTargetButton->setEnabled ( true );
    QToolTip::add( rebuildProjectButton, i18n( "Rebuild" ) );
    // run
    executeTargetButton = new QToolButton ( fileTools, "Run button" );
    executeTargetButton->setPixmap ( SmallIcon ( "exec",22 ) );
    executeTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, executeTargetButton->sizePolicy().hasHeightForWidth() ) );
    executeTargetButton->setEnabled ( true );
                
    
    // spacer
    spacer = new QWidget(fileTools);
    projectTools->setStretchFactor(spacer, 1);

    // Configure file button
    configurefileButton = new QToolButton ( fileTools, "Configure file" );
    configurefileButton->setPixmap ( SmallIcon ( "configure_file",22 ) );
    configurefileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, configurefileButton->sizePolicy().hasHeightForWidth() ) );
    configurefileButton->setEnabled ( true );
    QToolTip::add( configurefileButton, i18n( "Configure file" ) );

    // detail tree
    details = new KListView(detailContainer, "details widget");
    details->setRootIsDecorated(true);
    details->setResizeMode(QListView::LastColumn);
    details->setSorting(-1);
    details->header()->hide();
    details->addColumn(QString::null);

    // Detail button connections
    connect ( addfilesButton, SIGNAL ( clicked () ), this, SLOT ( slotAddFiles () ) );
    connect ( newfileButton, SIGNAL ( clicked () ), this, SLOT ( slotNewFile () ) );
    connect ( removefileButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveFile () ) );
    connect ( configurefileButton, SIGNAL ( clicked () ), this, SLOT ( slotConfigureFile () ) );

    // Detail tree connections
    connect( details, SIGNAL(selectionChanged(QListViewItem*)),
             this, SLOT(slotDetailsSelectionChanged(QListViewItem*)) );
    connect( details, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotDetailsExecuted(QListViewItem*)) );
    connect( details, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotDetailsContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    connect ( buildTargetButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildTarget () ) );
    connect ( rebuildTargetButton, SIGNAL ( clicked () ), this, SLOT ( slotRebuildTarget () ) );
    connect ( executeTargetButton, SIGNAL ( clicked () ), this, SLOT ( slotExecuteTarget () ) );
    buildTargetButton->setEnabled(false);
    rebuildTargetButton->setEnabled(false);
    executeTargetButton->setEnabled(false);


             
    m_part = part;
    m_shownSubproject = 0;
    
}


TrollProjectWidget::~TrollProjectWidget()
{}


void TrollProjectWidget::openProject(const QString &dirName)
{
    QDomDocument &dom = *(m_part->projectDom());
    m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                                    "subclass","sourcefile", "uifile");
    SubprojectItem *item = new SubprojectItem(overview, "/","");
    item->subdir = dirName.right(dirName.length()-dirName.findRev('/')-1);
    item->path = dirName;
    item->m_RootBuffer = &(item->m_FileBuffer);
    parse(item);
    item->setOpen(true);
    overview->setSelected(item, true);
}


void TrollProjectWidget::closeProject()
{
    overview->clear();
    details->clear();
}


QStringList TrollProjectWidget::allSubprojects()
{
    int prefixlen = projectDirectory().length()+1;
    QStringList res;

    QListViewItemIterator it(overview);
    for (; it.current(); ++it) {
        if (it.current() == overview->firstChild())
            continue;
        QString path = static_cast<SubprojectItem*>(it.current())->path;
        res.append(path.mid(prefixlen));
    }

    return res;
}


QStringList TrollProjectWidget::allFiles()
{
    QPtrStack<QListViewItem> s;
    QStringList res;

    for ( QListViewItem *item = overview->firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() ) {
        if (item->firstChild())
            s.push(item->firstChild());

        SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
        QString path = spitem->path;
        QPtrListIterator<GroupItem> tit(spitem->groups);
        for (; tit.current(); ++tit) {
            GroupItem::GroupType type = (*tit)->groupType;
            if (type == GroupItem::Sources || type == GroupItem::Headers) {
                QPtrListIterator<FileItem> fit(tit.current()->files);
		for (; fit.current(); ++fit){
		    QString filePath = path.mid( projectDirectory().length() + 1 );
		    if( !filePath.isEmpty() && !filePath.endsWith("/") )
			filePath += "/";
                    res.append( filePath + (*fit)->name);
		}
            }
        }
    }

    return res;
}

QString TrollProjectWidget::projectDirectory()
{
    if (!overview->firstChild())
        return QString::null; //confused

    return static_cast<SubprojectItem*>(overview->firstChild())->path;
}


QString TrollProjectWidget::subprojectDirectory()
{
    if (!m_shownSubproject)
        return QString::null;

    return m_shownSubproject->path;
}

void TrollProjectWidget::setupContext()
{
    if (!m_shownSubproject)
      return;
    bool buildable = true;
    bool runable = true;
    bool projectconfigurable = true;
    bool fileconfigurable = true;
    bool hasSourceFiles = true;
    bool hasSubdirs = false;


    if (m_shownSubproject->configuration.m_template==QTMP_LIBRARY)
    {
      runable = false;
    }
    if (m_shownSubproject->configuration.m_template==QTMP_SUBDIRS)
    {
      hasSubdirs = true;
      addSubdirButton->setEnabled(true);
      runable = false;
      hasSourceFiles = false;
      fileconfigurable = false;
    }
    if (m_shownSubproject->isScope)
    {
      runable = false;
      projectconfigurable = false;
      buildable = false;
    }


    // Setup toolbars according to context
    addSubdirButton->setEnabled(hasSubdirs);
    buildTargetButton->setEnabled(buildable);
    m_part->actionCollection()->action("build_build_target")->setEnabled(buildable);
    
    rebuildTargetButton->setEnabled(buildable);
    m_part->actionCollection()->action("build_rebuild_target")->setEnabled(buildable);

    executeTargetButton->setEnabled(runable);
    m_part->actionCollection()->action("build_execute_target")->setEnabled(runable);

    
    projectconfButton->setEnabled(projectconfigurable);
    configurefileButton->setEnabled(fileconfigurable);
    newfileButton->setEnabled(hasSourceFiles);
    removefileButton->setEnabled(hasSourceFiles);
    addfilesButton->setEnabled(hasSourceFiles);
    details->setEnabled(hasSourceFiles);
}

void TrollProjectWidget::slotOverviewSelectionChanged(QListViewItem *item)
{
    if (!item)
        return;
    cleanDetailView(m_shownSubproject);
    m_shownSubproject = static_cast<SubprojectItem*>(item);
    setupContext();
    buildProjectDetailTree(m_shownSubproject,details);

    QString subProjPath = m_shownSubproject->path;
    QString relpath = subProjPath.remove(0,projectDirectory().length());
    QDomDocument &dom = *(m_part->projectDom());
    DomUtil::writeEntry(dom, "/kdevtrollproject/general/activedir",relpath);

}

QString TrollProjectWidget::getCurrentTarget()
{
  if (!m_shownSubproject)
    return "";
  if (m_shownSubproject->configuration.m_destdir.right(1)=='/'||
      m_shownSubproject->configuration.m_destdir.isEmpty())
    return m_shownSubproject->configuration.m_destdir+m_shownSubproject->configuration.m_target;
  else
    return m_shownSubproject->configuration.m_destdir+'/'+m_shownSubproject->configuration.m_target;
}

QString TrollProjectWidget::getCurrentDestDir()
{
  if (!m_shownSubproject)
    return "";
  return m_shownSubproject->configuration.m_destdir;
}

QString TrollProjectWidget::getCurrentOutputFilename()
{
  if (!m_shownSubproject)
    return "";
  return m_shownSubproject->configuration.m_target;
}

void TrollProjectWidget::cleanDetailView(SubprojectItem *item)
{
  // If no children in detailview
  // it is a subdir template
  if (item && details->childCount())
  {
//    if (item->configuration.m_template == QTMP_SUBDIRS)
//      return;
    // Remove all GroupItems and all of their children from the view
//    QPtrListIterator<SubprojectItem> it(item->scopes);
//    for (; it.current(); ++it)
//    {
//      cleanDetailView(*it);
//      details->takeItem(*it);
//    }
    QPtrListIterator<GroupItem> it1(item->groups);
    for (; it1.current(); ++it1) {
      // After AddTargetDialog, it can happen that an
      // item is not yet in the list view, so better check...
      if (it1.current()->parent())
        while ((*it1)->firstChild())
          (*it1)->takeItem((*it1)->firstChild());
      details->takeItem(*it1);
    }
  }
}

void TrollProjectWidget::buildProjectDetailTree(SubprojectItem *item,KListView *listviewControl)
{
//  if (item->configuration.m_template == QTMP_SUBDIRS)
//    return;

  // Insert all GroupItems and all of their children into the view
  if (listviewControl)
  {
//    QPtrListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      listviewControl->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QPtrListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        listviewControl->insertItem(*it2);
        QPtrListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }
  else
  {
//    QPtrListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      item->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QPtrListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        item->insertItem(*it2);
        QPtrListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }
}

void TrollProjectWidget::slotDetailsExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are ProjectItem's
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);
    if (pvitem->type() != ProjectItem::File)
        return;

    QString dirName = m_shownSubproject->path;
    FileItem *fitem = static_cast<FileItem*>(pvitem);
    
    bool isUiFile = QFileInfo(fitem->name).extension() == "ui";
    if( m_part->isTMakeProject() && isUiFile ){
	// start designer in your PATH
	KShellProcess proc;
        proc << "designer" << (dirName + "/" + QString(fitem->name));
        proc.start( KProcess::DontCare, KProcess::NoCommunication );

    } else
	m_part->partController()->editDocument(KURL(dirName + "/" + QString(fitem->name)));

    m_part->mainWindow()->lowerView(this);
}


void TrollProjectWidget::slotConfigureProject()
{
//  ProjectOptionsDlg *d = new ProjectOptionsDlg(m_part,this);
//  d->exec();

  ProjectConfigurationDlg *dlg = new ProjectConfigurationDlg(m_shownSubproject,overview);
  dlg->exec();
  updateProjectConfiguration(m_shownSubproject);
  setupContext();
}

void TrollProjectWidget::slotExecuteTarget()
{
  //m_part->slotExecute();

  // no subproject selected
  if (!m_shownSubproject)
    return;

  // can't build from scope
  if (m_shownSubproject->isScope)
    return;
  if (m_shownSubproject->isScope)
    return;

  // Only run application projects
  if (m_shownSubproject->configuration.m_template!=QTMP_APPLICATION)
    return;

  QString dircmd = "cd "+subprojectDirectory() + "/" + getCurrentDestDir() + " && ";
  QString program = getCurrentOutputFilename();
//  std::cerr<<dircmd + "./"+program<<std::endl;
//  m_part->execute(dircmd + "./"+program);
//  m_part->appFrontend()->startAppCommand(dircmd +"./"+program,true);

  bool inTerminal = DomUtil::readBoolEntry(*m_part->projectDom(), "/kdevtrollproject/run/terminal");
  m_part->appFrontend()->startAppCommand(subprojectDirectory() + "/" + getCurrentDestDir(), "./" + getCurrentOutputFilename(),inTerminal );

}

void TrollProjectWidget::slotBuildProject()
{
  m_part->partController()->saveAllFiles();
  QString dir = projectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString buildcmd = "make";
  m_part->queueCmd(dir,dircmd + buildcmd);
  m_part->mainWindow()->lowerView(this);
  
}
void TrollProjectWidget::slotBuildTarget()
{
  // no subproject selected
  m_part->partController()->saveAllFiles();
  if (!m_shownSubproject)
    return;
  // can't build from scope
  if (m_shownSubproject->isScope)
    return;
  QString dir = subprojectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString buildcmd = "make";
  m_part->queueCmd(dir,dircmd + buildcmd);
  m_part->mainWindow()->lowerView(this);
}

void TrollProjectWidget::slotRebuildProject()
{
  m_part->partController()->saveAllFiles();
  QString dir = this->  projectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }


  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString rebuildcmd = "make clean && make";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
  m_part->mainWindow()->lowerView(this);
  
}

void TrollProjectWidget::slotRebuildTarget()
{
  // no subproject selected
  m_part->partController()->saveAllFiles();
  if (!m_shownSubproject)
    return;
  // can't build from scope
  if (m_shownSubproject->isScope)
    return;

  QString dir = subprojectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString rebuildcmd = "make clean && make";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
  m_part->mainWindow()->lowerView(this);
}

void TrollProjectWidget::slotCreateScope(SubprojectItem *spitem)
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
    spitem = m_shownSubproject;
  QString relpath = spitem->path.mid(projectDirectory().length());
  bool ok = FALSE;
  QString scopename = QInputDialog::getText(
                      i18n( "Create Scope" ),
                      i18n( "Please enter a name for the new scope:" ),
                      QLineEdit::Normal, QString::null, &ok, this );
  if ( ok && !scopename.isEmpty() )
  {
    QString newScopeString;
    if (!spitem->scopeString.isEmpty())
      newScopeString = spitem->scopeString + ":" + scopename;
    else
      newScopeString = scopename;

    spitem->m_RootBuffer->makeScope(newScopeString);
    parseScope(spitem,newScopeString,spitem->m_RootBuffer);
    updateProjectFile(spitem);
  }
  else
    return;
}

void TrollProjectWidget::slotAddSubdir(SubprojectItem *spitem)
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
    spitem = m_shownSubproject;
  QString relpath = spitem->path.mid(projectDirectory().length());

  bool ok = FALSE;
  QString subdirname = QInputDialog::getText(
                    i18n( "Add Subdirectory" ),
                    i18n( "Please enter a name for the new subdirectory:" ),
                    QLineEdit::Normal, QString::null, &ok, this );
  if ( ok && !subdirname.isEmpty() )
  {
    QDir dir(projectDirectory()+relpath);
    if (!dir.mkdir(subdirname))
    {
      KMessageBox::error(this,i18n("Failed to create subdirectory. "
                                   "Do you have write permission "
                                   "in the project folder?" ));
      return;
    }
    spitem->subdirs.append(subdirname);
    updateProjectFile(spitem);
    SubprojectItem *newitem = new SubprojectItem(spitem, subdirname,"");
    newitem->subdir = subdirname;
    newitem->m_RootBuffer = &(newitem->m_FileBuffer);
    newitem->path = spitem->path + "/" + subdirname;
    newitem->relpath = newitem->path;
    newitem->relpath.remove(0,projectDirectory().length());

    parse(newitem);
  }
  else
    return;
}

void TrollProjectWidget::slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    SubprojectItem *spitem = static_cast<SubprojectItem*>(item);

    KPopupMenu popup(i18n("Subproject %1").arg(item->text(0)), this);

    int idBuild = -2;
    int idQmake = -2;
    int idProjectConfiguration = -2;
    int idAddSubproject = -2;
    int idRemoveScope = -2;
    int idAddScope = -2;


    if (!spitem->isScope)
    {
      idBuild = popup.insertItem(SmallIcon("make_kdevelop"),i18n("Build"));
      idQmake = popup.insertItem(SmallIcon("qmakerun"),i18n("Run qmake"));
      popup.insertSeparator();
      idAddSubproject = popup.insertItem(SmallIcon("folder_new"),i18n("Add Subproject..."));
      idAddScope = popup.insertItem(SmallIcon("qmake_scopenew"),i18n("Create Scope..."));
      popup.insertSeparator();
      idProjectConfiguration = popup.insertItem(SmallIcon("configure"),i18n("Subproject Settings"));
    }
    else
    {
      idAddScope = popup.insertItem(SmallIcon("qmake_scopenew"),i18n("Create Scope..."));
      idRemoveScope = popup.insertItem(SmallIcon("qmake_scoperemove"),i18n("Remove Scope"));
    }

    int r = popup.exec(p);

    QString relpath = spitem->path.mid(projectDirectory().length());
    if (r == idAddSubproject)
    {
      slotAddSubdir(spitem);
    }
    if (r == idAddScope)
    {
      slotAddSubdir(spitem);
    }
    else if (r == idBuild)
    {
        m_part->startMakeCommand(projectDirectory() + relpath, QString::fromLatin1(""));
        m_part->mainWindow()->lowerView(this);
    }
    else if (r == idQmake)
    {
        m_part->startQMakeCommand(projectDirectory() + relpath);
        m_part->mainWindow()->lowerView(this);
    }
    else if (r == idProjectConfiguration)
    {
      ProjectConfigurationDlg *dlg = new ProjectConfigurationDlg(spitem,overview);
      dlg->exec();
      updateProjectConfiguration(spitem);
    }
}

void TrollProjectWidget::updateProjectConfiguration(SubprojectItem *item)
//=======================================================================
{
  updateProjectFile(item); //for update buildorder

  FileBuffer *Buffer = &(item->m_FileBuffer);
  QString relpath = item->path.mid(projectDirectory().length());
  // Template variable
  Buffer->removeValues("TEMPLATE");
  if (item->configuration.m_template == QTMP_APPLICATION)
    Buffer->setValues("TEMPLATE",QString("app"),FileBuffer::VSM_RESET);
  if (item->configuration.m_template == QTMP_LIBRARY) {
    Buffer->setValues("TEMPLATE",QString("lib"),FileBuffer::VSM_RESET);
    Buffer->removeValues("VERSION");
    Buffer->setValues("VERSION",item->configuration.m_libraryversion,FileBuffer::VSM_RESET);
  }
  if (item->configuration.m_template == QTMP_SUBDIRS)
    Buffer->setValues("TEMPLATE",QString("subdirs"),FileBuffer::VSM_RESET);

  Buffer->removeValues("IDL_COMPILER");
  Buffer->setValues("IDL_COMPILER",item->configuration.idl_compiler,FileBuffer::VSM_RESET);

  Buffer->removeValues("IDL_OPTIONS");
  Buffer->setValues("IDL_OPTIONS",item->configuration.idl_options,FileBuffer::VSM_RESET);


  //building idl targets
  QStringList::Iterator it=item->idls.begin();
  for(;it!=item->idls.end();++it){
    QString target=(*it)+".target";
    QString command=(*it)+".commands";
    QString commandStr="$$IDL_COMPILER $$IDL_OPTIONS $$"+target;
                          
    Buffer->removeValues(target);
    Buffer->setValues(target,*it,FileBuffer::VSM_RESET);

    Buffer->removeValues(command);
    Buffer->setValues(command,commandStr,FileBuffer::VSM_RESET);

  }                          
    




  // Config variable
  Buffer->removeValues("CONFIG");
  QStringList configList;
  if (item->configuration.m_buildMode == QBM_RELEASE)
    configList.append("release");
  else if (item->configuration.m_buildMode == QBM_DEBUG)
    configList.append("debug");
  if (item->configuration.m_warnings == QWARN_ON)
    configList.append("warn_on");
  else if (item->configuration.m_warnings == QWARN_OFF)
    configList.append("warn_off");
  if (item->configuration.m_requirements & QD_QT)
    configList.append("qt");
  if (item->configuration.m_requirements & QD_OPENGL)
    configList.append("opengl");
  if (item->configuration.m_requirements & QD_THREAD)
    configList.append("thread");
  if (item->configuration.m_requirements & QD_X11)
    configList.append("x11");
  if (item->configuration.m_requirements & QD_STATIC)
    configList.append("staticlib");
  if (item->configuration.m_requirements & QD_SHARED)
    configList.append("dll");
  if (item->configuration.m_requirements & QD_PLUGIN)
    configList.append("plugin");
  if (item->configuration.m_requirements & QD_EXCEPTIONS)
    configList.append("exceptions");
  if (item->configuration.m_requirements & QD_STL)
    configList.append("stl");
  if (item->configuration.m_requirements & QD_RTTI)
    configList.append("rtti");
  if (item->configuration.m_inheritconfig == true)
    Buffer->setValues("CONFIG",configList,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  else
    Buffer->setValues("CONFIG",configList,FileBuffer::VSM_RESET,VALUES_PER_ROW);

  // Config strings
  Buffer->removeValues("DESTDIR");
  if (!item->configuration.m_destdir.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("DESTDIR",QString(item->configuration.m_destdir),FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("TARGET");
  if (!item->configuration.m_target.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("TARGET",QString(item->configuration.m_target),FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("INCLUDEPATH");
  if (item->configuration.m_includepath.count())
      Buffer->setValues("INCLUDEPATH",item->configuration.m_includepath,FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("DEFINES");
  if (item->configuration.m_defines.count())
    Buffer->setValues("DEFINES",item->configuration.m_defines,FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("QMAKE_CXXFLAGS_DEBUG");
  if (item->configuration.m_cxxflags_debug.count())
    Buffer->setValues("QMAKE_CXXFLAGS_DEBUG",item->configuration.m_cxxflags_debug,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  Buffer->removeValues("QMAKE_CXXFLAGS_RELEASE");
  if (item->configuration.m_cxxflags_release.count())
    Buffer->setValues("QMAKE_CXXFLAGS_RELEASE",item->configuration.m_cxxflags_release,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  Buffer->removeValues("QMAKE_LFLAGS_DEBUG");
  if (item->configuration.m_lflags_debug.count())
    Buffer->setValues("QMAKE_LFLAGS_DEBUG",item->configuration.m_lflags_debug,FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("QMAKE_LFLAGS_RELEASE");
  if (item->configuration.m_lflags_release.count())
    Buffer->setValues("QMAKE_LFLAGS_RELEASE",item->configuration.m_lflags_release,FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("QMAKE_LIBDIR");
  if (item->configuration.m_librarypath.count())
    Buffer->setValues("QMAKE_LIBDIR",item->configuration.m_librarypath,FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("OBJECTS_DIR");
  if (!item->configuration.m_objectpath.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("OBJECTS_DIR",QString(item->configuration.m_objectpath),FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("UI_DIR");
  if (!item->configuration.m_uipath.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("UI_DIR",QString(item->configuration.m_uipath),FileBuffer::VSM_RESET,VALUES_PER_ROW);
  Buffer->removeValues("MOC_DIR");
  if (!item->configuration.m_mocpath.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("MOC_DIR",QString(item->configuration.m_mocpath),FileBuffer::VSM_RESET,VALUES_PER_ROW);




  Buffer->removeValues("INCLUDEPATH");
  if (item->configuration.m_incadd.count()>0)
    Buffer->setValues("INCLUDEPATH",item->configuration.m_incadd,FileBuffer::VSM_RESET,VALUES_PER_ROW);

  Buffer->removeValues("LIBS");
  if (item->configuration.m_libadd.count()>0)
    Buffer->setValues("LIBS",item->configuration.m_libadd,FileBuffer::VSM_APPEND,VALUES_PER_ROW);

  Buffer->removeValues("TARGETDEPS");
  if (item->configuration.m_libadd.count()>0)
    Buffer->setValues("TARGETDEPS",item->configuration.m_prjdeps,FileBuffer::VSM_APPEND,VALUES_PER_ROW);


    
  // Write to .pro file
//  Buffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->subdir+".pro",getHeader());
  Buffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->pro_file,getHeader());
}

SubprojectItem* TrollProjectWidget::getScope(SubprojectItem *baseItem,const QString &scopeString)
//===============================================================================================
{
  QStringList baseScopeParts = QStringList::split(':',baseItem->scopeString);
  QStringList subScopeParts = QStringList::split(':',scopeString);
  kdDebug(9024) << "baseitem" << baseItem->scopeString << endl;
  // Stop if baseItem not an ansister
  if (baseScopeParts.count() > subScopeParts.count())
    return NULL;
  uint i;
  for (i=0; i<baseScopeParts.count(); i++)
  {
    // Stop if baseItem in wrong treepart
    kdDebug(9024) << "baseScopeParts[i]" << "!=" << subScopeParts[i] << endl;
    if (baseScopeParts[i] != subScopeParts[i])
      return NULL;
  }
  // if all scopeparts matched and the amount of parts are equal this must be it
  if (baseScopeParts.count() == subScopeParts.count())
    return baseItem;
  // process next step of recursive function
  QString nextScopePart = subScopeParts[i];
  QPtrListIterator<SubprojectItem> spit(baseItem->scopes);
  for (; spit.current(); ++spit)
  {
    SubprojectItem *spitem = spit;
    kdDebug(9024) << spitem->text(0) << "==" << nextScopePart << endl;
    if (spitem->text(0)==nextScopePart)
    {
      return getScope(spit,scopeString);
      break;
    }
  }
  return NULL;
}

void TrollProjectWidget::updateProjectFile(QListViewItem *item)
{
  SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
  QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
  FileBuffer *subBuffer=m_shownSubproject->m_RootBuffer->getSubBuffer(spitem->scopeString);
  subBuffer->removeValues("SUBDIRS");
  subBuffer->setValues("SUBDIRS",spitem->subdirs,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->removeValues("SOURCES");
  subBuffer->setValues("SOURCES",spitem->sources,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("SOURCES",spitem->sources_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
  subBuffer->removeValues("HEADERS");
  subBuffer->setValues("HEADERS",spitem->headers,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("HEADERS",spitem->headers_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
//  subBuffer->removeValues("FORMS");
//  subBuffer->setValues("FORMS",spitem->forms,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
//  subBuffer->setValues("FORMS",spitem->forms_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("IDLS");
  subBuffer->setValues("IDLS",spitem->idls,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  //subBuffer->setValues("IDLS",spitem->idls_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("IMAGES");
  subBuffer->setValues("IMAGES",spitem->images,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  //subBuffer->setValues("IMAGES",spitem->images_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  
  
  if( m_part->isTMakeProject() ) {
      subBuffer->removeValues("INTERFACES");
      subBuffer->setValues("INTERFACES",spitem->forms,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
      subBuffer->setValues("INTERFACES",spitem->forms_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
  } else {
      subBuffer->removeValues("FORMS");
      subBuffer->setValues("FORMS",spitem->forms,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
      subBuffer->setValues("FORMS",spitem->forms_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
  }
//  m_shownSubproject->m_RootBuffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->subdir+".pro",getHeader());
  m_shownSubproject->m_RootBuffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->pro_file,getHeader());  
}

QString TrollProjectWidget::getHeader()
{
  QString header,templateString,targetString;
  QString relpath = "."+m_shownSubproject->path.mid(projectDirectory().length());
  if (m_shownSubproject->configuration.m_template==QTMP_APPLICATION)
  {
    templateString = i18n("an application: ");
    targetString = m_shownSubproject->configuration.m_target;
  }
  if (m_shownSubproject->configuration.m_template==QTMP_LIBRARY)
  {
    templateString = i18n("a library: ");
    targetString = m_shownSubproject->configuration.m_target;
  }
  if (m_shownSubproject->configuration.m_template==QTMP_SUBDIRS)
    templateString = i18n("a subdirs project");
  header.sprintf(m_part->getQMakeHeader(),
                 relpath.ascii(),
                 templateString.ascii(),
                 targetString.ascii());
  return header;
}


void TrollProjectWidget::addFileToCurrentSubProject(GroupItem *titem,const QString &filename)
{
  FileItem *fitem = createFileItem(filename);
  fitem->uiFileLink = getUiFileLink(titem->owner->relpath+"/",filename);
  titem->files.append(fitem);
  switch (titem->groupType)
  {
    case GroupItem::Sources:
      titem->owner->sources.append(filename);
      break;
    case GroupItem::Headers:
      titem->owner->headers.append(filename);
      break;
    case GroupItem::Forms:
      titem->owner->forms.append(filename);
    case GroupItem::IDLs:
      titem->owner->idls.append(filename);
      break;
    case GroupItem::Images:
      titem->owner->images.append(filename);
      break;
    default:
      break;
  }
}

void TrollProjectWidget::addFileToCurrentSubProject(GroupItem::GroupType gtype,const QString &filename)
{
  if (!m_shownSubproject)
    return;
  FileItem *fitem = createFileItem(filename);
  GroupItem *gitem = 0;

  QPtrListIterator<GroupItem> it(m_shownSubproject->groups);
  for (; it.current(); ++it)
  {
    if ((*it)->groupType == gtype)
    {
      gitem = *it;
      break;
    }
  }
  if (!gitem)
    return;
  fitem->uiFileLink = getUiFileLink(gitem->owner->relpath+"/",filename);
  gitem->files.append(fitem);
  switch (gtype)
  {
    case GroupItem::Sources:
      m_shownSubproject->sources.append(filename);
      break;
    case GroupItem::Headers:
      m_shownSubproject->headers.append(filename);
      break;
    case GroupItem::Forms:
      m_shownSubproject->forms.append(filename);
      break;
    case GroupItem::IDLs:
      m_shownSubproject->idls.append(filename);
      break;
    case GroupItem::Images:
      m_shownSubproject->images.append(filename);
      break;
    default:
      break;
  }
}

/**
* Method adds a file to the current project by grouped
* by file extension
*/
void TrollProjectWidget::addFile(const QString &fileName)
{
  if (!m_shownSubproject)
    return;
/*  QStringList splitFile = QStringList::split('.',fileName);
  QString ext = splitFile[splitFile.count()-1];
  splitFile = QStringList::split('/',fileName);
  QString fileWithNoSlash = splitFile[splitFile.count()-1];
  ext = ext.simplifyWhiteSpace();
  if (QString("cpp cc c").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::Sources,fileWithNoSlash);
  else if (QString("hpp h").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::Headers,fileWithNoSlash);
  else if (QString("ui").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::Forms,fileWithNoSlash);
  else if (QString("idl").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::IDLs,fileWithNoSlash);
  else if (QString("jpg png xpm").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::Images,fileWithNoSlash);
  else
    addFileToCurrentSubProject(GroupItem::NoType,fileWithNoSlash);*/

  QFileInfo info(fileName);
  QString ext = info.extension(false).simplifyWhiteSpace();
  QString noPathFileName = info.fileName();

  addFileToCurrentSubProject(GroupItem::groupTypeForExtension(ext), noPathFileName);

  updateProjectFile(m_shownSubproject);
  slotOverviewSelectionChanged(m_shownSubproject);
  emitAddedFile ( fileName );
}


void TrollProjectWidget::slotAddFiles()
{
  QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
  QString  filter = "*.cpp *.cc *.c *.hpp *.h *.ui|" + i18n("Source files");
  filter += "\n*|" + i18n("All files");
  KFileDialog *dialog = new KFileDialog(projectDirectory()+relpath,
                                        filter,
                                        this,
                                        i18n("Insert existing files"),
                                        TRUE);
  dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
  dialog->exec();
  QStringList files = dialog->selectedFiles();
  for (unsigned int i=0;i<files.count();i++)
  {
    // Copy selected files to current subproject folder
    QProcess *proc = new QProcess( this );
    proc->addArgument( "cp" );
    proc->addArgument( "-f" );
    proc->addArgument( files[i] );
    proc->addArgument( projectDirectory()+relpath );
    proc->start();
    QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
    // and add them to the filelist
    QFile testExist(projectDirectory()+relpath+"/"+filename);
    if (testExist.exists())
      addFile(filename);
  }
}

void TrollProjectWidget::slotNewFile()
{
    KDevCreateFile * createFileSupport = m_part->createFileSupport();
    if (createFileSupport)
    {
        KDevCreateFile::CreatedFile crFile =
            createFileSupport->createNewFile(QString::null, projectDirectory()+m_shownSubproject->path.mid(projectDirectory().length()));
    } else {
        bool ok = FALSE;
        QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
        QString filename = QInputDialog::getText(
                            i18n( "Insert New File"),
                            i18n( "Please enter a name for the new file:" ),
                            QLineEdit::Normal, QString::null, &ok, this );
        if ( ok && !filename.isEmpty() )
        {
            QFile newfile(projectDirectory()+relpath+'/'+filename);
            if (!newfile.open(IO_WriteOnly))
            {
            KMessageBox::error(this,i18n("Failed to create new file. "
                                        "Do you have write permission "
                                        "in the project folder?" ));
            return;
            }
            newfile.close();
            addFile(projectDirectory()+relpath+'/'+filename);
        }
    }
}

void TrollProjectWidget::slotRemoveFile()
{
  QListViewItem *selectedItem = details->currentItem();
  if (!selectedItem)
    return;
  ProjectItem *pvitem = static_cast<ProjectItem*>(selectedItem);
  // Check that it is a file (just in case)
  if (pvitem->type() != ProjectItem::File)
    return;
  FileItem *fitem = static_cast<FileItem*>(pvitem);
  removeFile(m_shownSubproject, fitem);
}

void TrollProjectWidget::slotConfigureFile()
{
  QListViewItem *selectedItem = details->currentItem();
  if (!selectedItem)
    return;
  ProjectItem *pvitem = static_cast<ProjectItem*>(selectedItem);
  // Check that it is a file (just in case)
  if (pvitem->type() != ProjectItem::File)
    return;
  FileItem *fitem = static_cast<FileItem*>(pvitem);

  GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());
  if (!gitem)
    return;
  QStringList dirtyScopes;
  FilePropertyDlg *propdlg = new FilePropertyDlg(m_shownSubproject,gitem->groupType,fitem,dirtyScopes);
  SubprojectItem *scope;
  propdlg->exec();
  for (uint i=0; i<dirtyScopes.count();i++)
  {
    scope = getScope(m_shownSubproject,dirtyScopes[i]);
    if (scope)
      updateProjectFile(scope);
  }
}

void TrollProjectWidget::slotDetailsSelectionChanged(QListViewItem *item)
{
    if (!item)
        return;
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);
    if (pvitem->type() == ProjectItem::Group)
    {
        removefileButton->setEnabled(false);
        configurefileButton->setEnabled(false);
    }
    else if (pvitem->type() == ProjectItem::File)
    {
        removefileButton->setEnabled(true);
        configurefileButton->setEnabled(true);
    }
}

void TrollProjectWidget::slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    ProjectItem *pvitem = static_cast<ProjectItem*>(item);
    if (pvitem->type() == ProjectItem::Group) {
        GroupItem *titem = static_cast<GroupItem*>(pvitem);
        QString title,ext;
        switch (titem->groupType) {
        case GroupItem::Sources:
            title = i18n("Sources");
            ext = "*.cpp *.c";
            break;
        case GroupItem::Headers:
            title = i18n("Headers");
            ext = "*.h *.hpp";
            break;
        case GroupItem::Forms:
            title = i18n("Forms");
            ext = "*.ui";
            break;
        case GroupItem::IDLs:
            title = i18n("Corba IDLs");
            ext = "*.idl *.kidl";
            break;
        case GroupItem::Images:
            title = i18n("Images");
            ext = "*.jpg *.png *.xpm *.gif";
            break;
        default: ;
        }

        KPopupMenu popup(title, this);
        int idInsExistingFile = popup.insertItem(SmallIconSet("fileopen"),i18n("Insert existing files..."));
        int idInsNewFile = popup.insertItem(SmallIconSet("filenew"),i18n("Insert New File..."));
 //       int idFileProperties = popup.insertItem(SmallIconSet("filenew"),i18n("Properties..."));
        int r = popup.exec(p);
        QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
        if (r == idInsExistingFile)
        {
          KFileDialog *dialog = new KFileDialog(projectDirectory()+relpath,
                                                ext + "|" + title + " (" + ext + ")",
                                                this,
                                                i18n("Insert existing %1").arg(title),
                                                TRUE);
          dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
          if ( dialog->exec() == QDialog::Rejected )
	    return;
          QStringList files = dialog->selectedFiles();
          for (unsigned int i=0;i<files.count();i++)
          {
            // Copy selected files to current subproject folder
            QProcess *proc = new QProcess( this );
            proc->addArgument( "cp" );
            proc->addArgument( "-f" );
            proc->addArgument( files[i] );
            proc->addArgument( projectDirectory()+relpath );
            proc->start();
            QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
            // and add them to the filelist
            addFileToCurrentSubProject(titem,filename);
          }
          // Update project file
          if ( titem && titem->owner )
            updateProjectFile(titem->owner);
          // Update subprojectview
          slotOverviewSelectionChanged(m_shownSubproject);
        }
        if (r == idInsNewFile)
        {
            KDevCreateFile * createFileSupport = m_part->createFileSupport();
            if (createFileSupport)
            {
                QString fcext;
                switch (titem->groupType) {
                case GroupItem::Sources:
                    fcext = "cpp";
                    break;
                case GroupItem::Headers:
                    fcext = "h";
                    break;
                case GroupItem::Forms:
                    fcext = "ui-widget";
                    break;
                default:
                    fcext = QString::null;
                }
                KDevCreateFile::CreatedFile crFile =
                    createFileSupport->createNewFile(fcext, projectDirectory()+m_shownSubproject->path.mid(projectDirectory().length()));
            } else {
                bool ok = FALSE;
                QString filename = QInputDialog::getText(
                                    i18n( "Insert New File"),
                                    i18n( "Please enter a name for the new file:" ),
                                    QLineEdit::Normal, QString::null, &ok, this );
                if ( ok && !filename.isEmpty() )
                {
                    QFile newfile(projectDirectory()+relpath+'/'+filename);
                    if (!newfile.open(IO_WriteOnly))
                    {
                    KMessageBox::error(this,i18n("Failed to create new file. "
                                                "Do you have write permission "
                                                "in the project folder?" ));
                    return;
                    }
                    newfile.close();
                    addFileToCurrentSubProject(titem,filename);
                    updateProjectFile(titem->owner);
                    slotOverviewSelectionChanged(m_shownSubproject);
                }
            }
        }

    } else if (pvitem->type() == ProjectItem::File) {

        removefileButton->setEnabled(true);
        FileItem *fitem = static_cast<FileItem*>(pvitem);

        KPopupMenu popup(i18n("File: %1").arg(fitem->name), this);
        int idRemoveFile = popup.insertItem(SmallIconSet("stop"),i18n("Remove File"));
        int idSubclassWidget = popup.insertItem(SmallIconSet("qmake_subclass.png"),i18n("Subclass widget..."));
        int idUpdateWidgetclass = popup.insertItem(SmallIconSet("qmake_subclass.png"),i18n("Edit ui-subclass..."));
        int idUISubclasses = popup.insertItem(SmallIconSet("qmake_subclass.png"),i18n("List of Subclasses..."));
        int idViewUIH = popup.insertItem(SmallIconSet("qmake_ui_h.png"),i18n("Open ui.h File"));
        int idFileProperties = popup.insertItem(SmallIconSet("configure_file"),i18n("Properties..."));

        if (fitem->uiFileLink.isEmpty())
        {
          popup.removeItem(idUpdateWidgetclass);
        }
        if(!fitem->name.contains(".ui"))
        {
          popup.removeItem(idUISubclasses);
          popup.removeItem(idViewUIH);
          popup.removeItem(idSubclassWidget);
        }

        FileContext context(m_shownSubproject->path + "/" + fitem->name, false);
        m_part->core()->fillContextMenu(&popup, &context);

        int r = popup.exec(p);
        if (r == idRemoveFile)
            removeFile(m_shownSubproject, fitem);
        // Fileproperties
        else if (r == idFileProperties)
        {
          GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());
          if (!gitem)
            return;
          QStringList dirtyScopes;
          FilePropertyDlg *propdlg = new FilePropertyDlg(m_shownSubproject,gitem->groupType,fitem,dirtyScopes);
          SubprojectItem *scope;
          propdlg->exec();
          for (uint i=0; i<dirtyScopes.count();i++)
          {
            scope = getScope(m_shownSubproject,dirtyScopes[i]);
            if (scope)
               updateProjectFile(scope);
          }
        }
        else if(r == idViewUIH) {
          m_part->partController()->editDocument(KURL(m_shownSubproject->path + "/" +
             QString(fitem->name + ".h")));

        }
        else if (r == idSubclassWidget)
        {
            QStringList newFileNames;
            newFileNames = m_part->languageSupport()->subclassWidget(m_shownSubproject->path + "/" + fitem->name);
            if (!newFileNames.empty())
            {
                QDomDocument &dom = *(m_part->projectDom());
                for (uint i=0; i<newFileNames.count(); i++)
                {
                    QString srcfile_relpath = newFileNames[i].remove(0,projectDirectory().length());
                    QString uifile_relpath = QString(m_shownSubproject->path + "/" + fitem->name).remove(0,projectDirectory().length());
                    DomUtil::PairList list = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                                            "subclass","sourcefile", "uifile");

                    list << DomUtil::Pair(srcfile_relpath,uifile_relpath);
                    DomUtil::writePairListEntry(dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list);
//                    newFileNames[i] = newFileNames[i].replace(QRegExp(projectDirectory()+"/"),"");
                    newFileNames[i] = projectDirectory() + newFileNames[i];
                    qWarning("new file: %s", newFileNames[i].latin1());
                }
                m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                                                "subclass","sourcefile", "uifile");

                m_part->addFiles(newFileNames);
            }
        }
        else if (r == idUpdateWidgetclass)
        {
          QString noext = m_shownSubproject->path + "/" + fitem->name;
          if (noext.findRev('.')>-1)
            noext = noext.left(noext.findRev('.'));
          QStringList dummy;
          QString uifile = fitem->uiFileLink;
          if (uifile.findRev('/')>-1)
          {
            QStringList uisplit = QStringList::split('/',uifile);
            uifile=uisplit[uisplit.count()-1];
          }
          m_part->languageSupport()->updateWidget(m_shownSubproject->path + "/" + uifile, noext);
        }
        else if (r == idUISubclasses)
        {
            QDomDocument &dom = *(m_part->projectDom());
            DomUtil::PairList list = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                                    "subclass","sourcefile", "uifile");
            SubclassesDlg *sbdlg = new SubclassesDlg( QString(m_shownSubproject->path + "/" + fitem->name).remove(0,projectDirectory().length()),
                list, projectDirectory());
                
            if (sbdlg->exec())
            {
                QDomElement el = DomUtil::elementByPath( dom,  "/kdevtrollproject");
                QDomElement el2 = DomUtil::elementByPath( dom,  "/kdevtrollproject/subclassing");
                if ( (!el.isNull()) && (!el2.isNull()) ) 
                {
                    el.removeChild(el2);
                }
                
                DomUtil::writePairListEntry(dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list);
                
                m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                    "subclass","sourcefile", "uifile");
            }
        }
    }
}


void TrollProjectWidget::removeFile(SubprojectItem *spitem, FileItem *fitem)
{
    GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());

    emitRemovedFile(spitem->path + "/" + fitem->text(0));
    
    
    //remove subclassing info
    QDomDocument &dom = *(m_part->projectDom());
    DomUtil::PairList list = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                            "subclass","sourcefile", "uifile");
    QPtrList<DomUtil::Pair> pairsToRemove;
    DomUtil::PairList::iterator it;
    QString file = QString(spitem->path + "/" + fitem->name).remove(0,projectDirectory().length());
    for ( it = list.begin(); it != list.end(); ++it )
    {
        if ( ((*it).first == file) || ((*it).second == file) )
        {
            pairsToRemove.append(&(*it));
        }
    }
    DomUtil::Pair *pair;
    for ( pair = pairsToRemove.first(); pair; pair = pairsToRemove.next() )
    {
        list.remove(*pair);
    }
    QDomElement el = DomUtil::elementByPath( dom,  "/kdevtrollproject");
    QDomElement el2 = DomUtil::elementByPath( dom,  "/kdevtrollproject/subclassing");
    if ( (!el.isNull()) && (!el2.isNull()) ) 
    {
        el.removeChild(el2);
    }
    DomUtil::writePairListEntry(dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list);
    
    
    
    switch (gitem->groupType)
    {
      case GroupItem::Sources:
        spitem->sources.remove(fitem->text(0));
        break;
      case GroupItem::Headers:
        spitem->headers.remove(fitem->text(0));
        break;
      case GroupItem::Forms:
        spitem->forms.remove(fitem->text(0));
        break;
      default: ;
    }
    gitem->files.remove(fitem);
    updateProjectFile(m_shownSubproject);
}


GroupItem *TrollProjectWidget::createGroupItem(GroupItem::GroupType groupType, const QString &name,const QString &scopeString)
{
    // Workaround because for QListView not being able to create
    // items without actually inserting them
    GroupItem *titem = new GroupItem(overview, groupType, name,scopeString);
    overview->takeItem(titem);

    return titem;
}


FileItem *TrollProjectWidget::createFileItem(const QString &name)
{
    FileItem *fitem = new FileItem(overview, name);
    overview->takeItem(fitem);
    fitem->name = name;

    return fitem;
}

void TrollProjectWidget::emitAddedFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );
    emit m_part->addedFilesToProject(fileList);
}


void TrollProjectWidget::emitRemovedFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );
    emit m_part->removedFilesFromProject(fileList);
}


QString TrollProjectWidget::getUiFileLink(const QString &relpath, const QString& filename)
{
  DomUtil::PairList::iterator it;
  for (it=m_subclasslist.begin();it != m_subclasslist.end(); ++it)
  {
    if ((*it).first==relpath+filename)
      return (*it).second;
  }
  return "";
}

void TrollProjectWidget::parseScope(SubprojectItem *item, QString scopeString, FileBuffer *buffer)
{
    if (!scopeString.isEmpty())
    {
      QStringList scopeNames = QStringList::split(':',scopeString);
      SubprojectItem *sitem;
      sitem = new SubprojectItem(item, scopeNames[scopeNames.count()-1],scopeString);
      sitem->path = item->path;
      sitem->m_RootBuffer = buffer;
      sitem->subdir = item->subdir;
      item->scopes.append(sitem);
      item=sitem;
    }

    item->relpath = item->path;
    item->relpath.remove(0,projectDirectory().length());

    QStringList minusListDummy;
    FileBuffer *subBuffer = buffer->getSubBuffer(scopeString);
    if( m_part->isTMakeProject() )
	subBuffer->getValues("INTERFACES",item->forms,item->forms_exclude);
    else
	subBuffer->getValues("FORMS",item->forms,item->forms_exclude);
    
    subBuffer->getValues("SOURCES",item->sources,item->sources_exclude);
    subBuffer->getValues("HEADERS",item->headers,item->headers_exclude);
    subBuffer->getValues("IMAGES",item->images,item->images_exclude);
    subBuffer->getValues("IDLS",item->idls,item->idls_exclude);

    // Create list view items

    GroupItem * titem = createGroupItem(GroupItem::Images, "IMAGES",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->images.isEmpty()) {
        QStringList l = item->images;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::IDLs, "Corba IDL",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->idls.isEmpty()) {
        QStringList l = item->idls;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }


//    titem = createGroupItem(GroupItem::Forms, "FORMS",scopeString);
    titem = createGroupItem(GroupItem::Forms, 
				       (m_part->isTMakeProject() ? "INTERFACES" : "FORMS"),
				       scopeString);

    item->groups.append(titem);
    titem->owner = item;
    if (!item->forms.isEmpty()) {
        QStringList l = item->forms;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Sources, "SOURCES",scopeString);
    item->groups.append(titem);
    titem->owner = item;
    if (!item->sources.isEmpty()) {
        QStringList l = item->sources;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            getUiFileLink(item->relpath+"/",*it);
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Headers, "HEADERS",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->headers.isEmpty()) {
        QStringList l = item->headers;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    QStringList childScopes = subBuffer->getChildScopeNames();
    for (unsigned int i=0; i<childScopes.count();i++)
      parseScope(item,scopeString+(!scopeString.isEmpty() ? ":" : "")+childScopes[i],buffer);
}

void TrollProjectWidget::parse(SubprojectItem *item)
{
    QFileInfo fi(item->path);
//    QString proname = item->path + "/" + fi.baseName() + ".pro";
    QDir dir(item->path);
    QStringList l = dir.entryList("*.pro");
    
    item->pro_file = l.count()?l[0]:(fi.baseName() + ".pro");
    QString proname = item->path + "/" + item->pro_file;
    
    kdDebug(9024) << "Parsing " << proname << endl;


    item->m_FileBuffer.bufferFile(proname);
    item->m_FileBuffer.handleScopes();

    parseScope(item,"",&(item->m_FileBuffer));
    QStringList minusListDummy;
    QStringList lst;

    item->configuration.m_subdirName = item->subdir;
    // set defaults
    item->configuration.m_template = QTMP_APPLICATION;
    item->configuration.m_buildMode = QBM_RELEASE;
    item->configuration.m_warnings = QWARN_ON;
    item->configuration.m_requirements = 0;
    item->setPixmap(0,SmallIcon("qmake_app"));

    // retrieve the project configuration
    item->m_FileBuffer.getValues("TEMPLATE",lst,minusListDummy);
    if (lst.count())
    {
      if (lst[0] == "app")
        item->configuration.m_template = QTMP_APPLICATION;
      if (lst[0] == "lib")
      {
        item->setPixmap(0,SmallIcon("qmake_lib"));
        item->configuration.m_template = QTMP_LIBRARY;
      }
      if (lst[0] == "subdirs")
      {
        item->setPixmap(0,SmallIcon("qmake_sub"));
        item->configuration.m_template = QTMP_SUBDIRS;
      }
    }
    item->m_FileBuffer.getValues("VERSION",lst,minusListDummy);
    if(lst.count())
      item->configuration.m_libraryversion = lst[0];

    item->configuration.m_inheritconfig = true;
    QPtrList<FileBuffer::ValueSetMode> configvaluesetmodes;
    item->m_FileBuffer.getVariableValueSetModes("CONFIG",configvaluesetmodes);

    FileBuffer::ValueSetMode* ConfigSetMode = NULL;
    for(ConfigSetMode = configvaluesetmodes.first(); ConfigSetMode; ConfigSetMode = configvaluesetmodes.next())
    {
      if(ConfigSetMode != NULL)
      {
        if(ConfigSetMode[0] == FileBuffer::VSM_RESET)
        {
          item->configuration.m_inheritconfig = false;
	}
      }
    }
      
    item->m_FileBuffer.getValues("CONFIG",lst,minusListDummy);
    if (lst.count())
    {
      // config debug/release
      if (lst.find("debug")!=lst.end())
        item->configuration.m_buildMode = QBM_DEBUG;
      if (lst.find("release")!=lst.end())
        item->configuration.m_buildMode = QBM_RELEASE;
      // config warnings on/off
      if (lst.find("warn_on")!=lst.end())
        item->configuration.m_warnings = QWARN_ON;
      if (lst.find("warn_off")!=lst.end())
        item->configuration.m_warnings = QWARN_OFF;
      // config requerments
      if (lst.find("qt")!=lst.end())
        item->configuration.m_requirements += QD_QT;
      if (lst.find("x11")!=lst.end())
        item->configuration.m_requirements += QD_X11;
      if (lst.find("thread")!=lst.end())
        item->configuration.m_requirements += QD_THREAD;
      if (lst.find("opengl")!=lst.end())
        item->configuration.m_requirements += QD_OPENGL;
      // config lib mode
      if (lst.find("staticlib")!=lst.end())
        item->configuration.m_requirements += QD_STATIC;
      if (lst.find("dll")!=lst.end())
        item->configuration.m_requirements += QD_SHARED;
      if (lst.find("plugin")!=lst.end())
        item->configuration.m_requirements += QD_PLUGIN;
      if (lst.find("exceptions")!=lst.end())
        item->configuration.m_requirements += QD_EXCEPTIONS;
      if (lst.find("stl")!=lst.end())
        item->configuration.m_requirements += QD_STL;
      if (lst.find("rtti")!=lst.end())
        item->configuration.m_requirements += QD_RTTI;
    }
    item->m_FileBuffer.getValues("DESTDIR",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_destdir = lst[0];
    item->m_FileBuffer.getValues("TARGET",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_target = lst[0];
    item->m_FileBuffer.getValues("INCLUDEPATH",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_incadd = lst;
    item->m_FileBuffer.getValues("LIBS",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_libadd = lst;
    item->m_FileBuffer.getValues("DEFINES",lst,minusListDummy);
    item->configuration.m_defines = lst;
    item->m_FileBuffer.getValues("QMAKE_CXXFLAGS_DEBUG",lst,minusListDummy);
    item->configuration.m_cxxflags_debug = lst;
    item->m_FileBuffer.getValues("QMAKE_CXXFLAGS_RELEASE",lst,minusListDummy);
    item->configuration.m_cxxflags_release = lst;
    item->m_FileBuffer.getValues("QMAKE_LFLAGS_DEBUG",lst,minusListDummy);
    item->configuration.m_lflags_debug = lst;
    item->m_FileBuffer.getValues("QMAKE_LFLAGS_RELEASE",lst,minusListDummy);
    item->configuration.m_lflags_release = lst;
    item->m_FileBuffer.getValues("QMAKE_LIBDIR",lst,minusListDummy);
    item->configuration.m_librarypath = lst;
    item->m_FileBuffer.getValues("OBJECTS_DIR",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_objectpath = lst[0];
    item->m_FileBuffer.getValues("UI_DIR",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_uipath = lst[0];
    item->m_FileBuffer.getValues("MOC_DIR",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_mocpath = lst[0];

    // Handle "subdirs" project
    if (item->configuration.m_template == QTMP_SUBDIRS)
    {
      item->m_FileBuffer.getValues("SUBDIRS",lst,minusListDummy);
      item->subdirs = lst;
      QStringList::Iterator it;
      for (it = lst.begin(); it != lst.end(); ++it)
      {
        SubprojectItem *newitem = new SubprojectItem(item, (*it),"");
        newitem->subdir = *it;
        newitem->m_RootBuffer = &(newitem->m_FileBuffer);
        newitem->path = item->path + "/" + (*it);
        parse(newitem);
      }
    }
}
void TrollProjectWidget::slotBuildFile()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(m_part->partController()->activePart());
    if (!part || !part->url().isLocalFile())
        return;

    QString fileName = part->url().path();
    QFileInfo fi(fileName);
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName();
    kdDebug(9020) << "Compiling " << fileName
                  << "in dir " << sourceDir
                  << " with baseName " << baseName << endl;

    QString projectDir = projectDirectory();
    if (!sourceDir.startsWith(projectDir)) {
        KMessageBox::sorry(this, i18n("Can only compile files in directories which belong to the project."));
        return;
    }

    QString buildDir = sourceDir;
    QString target = baseName + ".o";
    kdDebug(9020) << "builddir " << buildDir << ", target " << target << endl;

    m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
    m_part->startMakeCommand(buildDir, target);

  
}


void TrollProjectWidget::slotExecuteProject()
{
    QString program = m_part->mainProgram();

    if ( program.isEmpty() ) {
        KMessageBox::sorry(this, i18n("Please specify the executable name in the "
            "project options dialog first."), i18n("No executable name given"));
        return;
    }

    QDomElement docEl = m_part->projectDom()->documentElement();
    QDomElement trollprojectEl = docEl.namedItem("kdevtrollproject").toElement();
    QDomElement envvarsEl = trollprojectEl.namedItem("envvars").toElement();

    QString environstr;
    QDomElement childEl = envvarsEl.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "envvar") {
            environstr += childEl.attribute("name");
            environstr += "=";
            environstr += childEl.attribute("value");
            environstr += " ";
        }
        childEl = childEl.nextSibling().toElement();
    }
    program.prepend(environstr);

    QString dircmd = "cd "+this->projectDirectory() + " && " ;

    bool inTerminal = DomUtil::readBoolEntry(*(m_part->projectDom()), "/kdevtrollproject/run/terminal");
//    m_part->appFrontend()->startAppCommand(dircmd + program, inTerminal);
//    m_part->execute(this->projectDirectory(), "./"+program );
  m_part->appFrontend()->startAppCommand(this->projectDirectory(), "./"+program,inTerminal );
}


void TrollProjectWidget::slotCleanProject()
{
  QString dir = this->  projectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString rebuildcmd = "make clean";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
  m_part->mainWindow()->lowerView(this);

}

void TrollProjectWidget::slotCleanTarget()
{
  // no subproject selected
  m_part->partController()->saveAllFiles();
  if (!m_shownSubproject)
    return;
  // can't build from scope
  if (m_shownSubproject->isScope)
    return;

  QString dir = subprojectDirectory();
  QFileInfo fi(dir + "/Makefile");
  QFileInfo fi2(dir + "/makefile");
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }
  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+dir + " && " ;
  QString rebuildcmd = "make clean";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
  m_part->mainWindow()->lowerView(this);
}


#include "trollprojectwidget.moc"
