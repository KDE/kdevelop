/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2000-2001 by Trolltech AS.                              *
 *   info@trolltech.com                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *   Copyright (C) 2002-2003 by Alexander Dymo                             *
 *   cloudtemple@mksat.net                                                 *
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
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

#include <config.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qdir.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <qwhatsthis.h>
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
#include <kinputdialog.h>
#include <kdeversion.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "trollprojectpart.h"
#include "kdevappfrontend.h"
#include "kdevmakefrontend.h"
#include "kdevlanguagesupport.h"
#include "kdevcreatefile.h"
#include "subclassesdlg.h"
#include "addfilesdialog.h"
#include "urlutil.h"
#include "pathutil.h"
#include "trolllistview.h"

#define VALUES_PER_ROW  1

/*
 * Class qProjectItem
 */

qProjectItem::qProjectItem(Type type, QListView *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


qProjectItem::qProjectItem(Type type, qProjectItem *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}




/*
 * Class SubqmakeprojectItem
 */

SubqmakeprojectItem::SubqmakeprojectItem(QListView *parent, const QString &text, const QString &scopeString)
    : qProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    configuration.m_template = QTMP_APPLICATION;
    init();
}


SubqmakeprojectItem::SubqmakeprojectItem(SubqmakeprojectItem *parent, const QString &text,const QString &scopeString)
    : qProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    init();
}

SubqmakeprojectItem::~SubqmakeprojectItem()
{
}
QString SubqmakeprojectItem::getRelativPath()
{
  if(this->parent()==NULL||this->parent()->parent()==NULL) return("/"+configuration.m_subdirName);
  else return(((SubqmakeprojectItem*)this->parent())->getRelativPath()+"/"+configuration.m_subdirName);
}
QString SubqmakeprojectItem::getDownDirs()
{
    SubqmakeprojectItem* pItem=this;
	while (pItem->parent())
	    pItem=(SubqmakeprojectItem*)pItem->parent();
    return getRelativePath(QDir::cleanDirPath(this->path),QDir::cleanDirPath(pItem->path));
}
QString SubqmakeprojectItem::getSharedLibAddObject(QString downDirs)
{
  if(configuration.m_requirements & QD_SHARED)
  {
    QString tmpPath;
    if(configuration.m_destdir!="")
    {
	  if (QDir::isRelativePath(configuration.m_destdir))
        tmpPath=downDirs+this->getRelativPath()+"/"+configuration.m_destdir;
	  else
	    tmpPath=configuration.m_destdir;
    }else{
      tmpPath=downDirs+this->getRelativPath()+"/";
    }

    tmpPath=QDir::cleanDirPath(tmpPath);

    QString libString;
    if(configuration.m_target!="")
    {
      libString = tmpPath+"/lib"+this->configuration.m_target+".so";

    }else{
      libString = tmpPath+"/lib"+this->configuration.m_subdirName.section('/',-1,-1,QString::SectionSkipEmpty)+".so";

    }
    return(libString);
  }
  return "";
}
QString SubqmakeprojectItem::getApplicationObject( QString downDirs )
{
  QString tmpPath;
  if(configuration.m_destdir!="")
  {
  	  if (QDir::isRelativePath(configuration.m_destdir))
        tmpPath=downDirs+this->getRelativPath()+"/"+configuration.m_destdir;
	  else
	    tmpPath=configuration.m_destdir;
  }else{
    tmpPath=downDirs+this->getRelativPath()+"/";
  }

  tmpPath=QDir::cleanDirPath(tmpPath);

  if (configuration.m_target.isEmpty())
    return tmpPath + "/" + configuration.m_subdirName.section('/',-1,-1,QString::SectionSkipEmpty);
  else
    return tmpPath + "/" + configuration.m_target;
}
QString SubqmakeprojectItem::getLibAddObject(QString downDirs)
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
 	  if (QDir::isRelativePath(configuration.m_destdir))
        tmpPath=downDirs+this->getRelativPath()+"/"+configuration.m_destdir;
	  else
	    tmpPath=configuration.m_destdir;
    }else{
      tmpPath=downDirs+this->getRelativPath()+"/";
    }

    tmpPath=QDir::cleanDirPath(tmpPath);

    QString libString;
    if(configuration.m_target!="")
    {
      libString = tmpPath+"/lib"+this->configuration.m_target+".a";

    }else{
      libString = tmpPath+"/lib"+this->configuration.m_subdirName.section('/',-1,-1,QString::SectionSkipEmpty)+".a";

    }
    return(libString);
  }

  return("");
}
QString SubqmakeprojectItem::getLibAddPath(QString downDirs)
{

  //PATH only add if shared lib
  if(!(configuration.m_requirements & QD_SHARED))return("");

    QString tmpPath;
    if(configuration.m_destdir!="")
    {
	  if (QDir::isRelativePath(configuration.m_destdir))
        tmpPath=downDirs+this->getRelativPath()+"/"+configuration.m_destdir;
	  else
	    tmpPath=configuration.m_destdir;
    }else{
      tmpPath=downDirs+this->getRelativPath()+"/";
    }

    tmpPath=QDir::cleanDirPath(tmpPath);

  return(tmpPath);

}
QString SubqmakeprojectItem::getIncAddPath(QString downDirs)
{
  QString tmpPath=downDirs+this->getRelativPath();
  tmpPath=QDir::cleanDirPath(tmpPath);

  return(tmpPath);
}

void SubqmakeprojectItem::init()
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


/*
 * Class GroupItem
 */

GroupItem::GroupItem(QListView *lv, GroupType type, const QString &text, const QString &scopeString)
    : qProjectItem(Group, lv, text)
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
    else if (ext == "idl")
        return IDLs;
    else if (ext == "l" || ext == "ll" || ext == "lxx" || ext == "l++" )
        return Lexsources;
    else if (ext == "y" || ext == "yy" || ext == "yxx" || ext == "y++" )
        return Yaccsources;
    else if (ext == "ts")
        return Translations;
    else
        return NoType;
}

/*
 * Class FileItem
 */

FileItem::FileItem(QListView *lv, const QString &text, bool exclude/*=false*/)
    : qProjectItem(File, lv, text)
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
    addSubdirButton = new QToolButton ( projectTools, "Add subproject button" );
    addSubdirButton->setPixmap ( SmallIcon ( "folder_new" ) );
    addSubdirButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, addSubdirButton->sizePolicy().hasHeightForWidth() ) );
    addSubdirButton->setEnabled ( true );
    QToolTip::add( addSubdirButton, i18n( "Add subproject" ) );
    QWhatsThis::add(addSubdirButton, i18n("<b>Add subproject</b><p>Creates a <i>new</i> or adds an <i>existing</i> subproject to a currently selected subproject. "
        "This action is allowed only if a type of the subproject is 'subdirectories'. The type of the subproject can be "
        "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)."));
    // Create scope
    createScopeButton = new QToolButton ( projectTools, "Create scope button" );
    createScopeButton->setPixmap ( SmallIcon ( "qmake_scopenew" ) );
    createScopeButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, createScopeButton->sizePolicy().hasHeightForWidth() ) );
    createScopeButton->setEnabled ( true );
    QToolTip::add( createScopeButton, i18n( "Create scope" ) );
    QWhatsThis::add(createScopeButton, i18n("<b>Create scope</b><p>Creates QMake scope in the project file in case the subproject is selected or creates nested scope in case the scope is selected."));
    // build selected file
    buildFileButton = new QToolButton ( projectTools, "Make file button" );
    buildFileButton->setPixmap ( SmallIcon ( "compfile" ) );
    buildFileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildFileButton->sizePolicy().hasHeightForWidth() ) );
    buildFileButton->setEnabled ( true );
    QToolTip::add( buildFileButton, i18n( "Compile file" ) );
    QWhatsThis::add(buildFileButton, i18n("<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    // build
    buildProjectButton = new QToolButton ( projectTools, "Make button" );
    buildProjectButton->setPixmap ( SmallIcon ( "make_kdevelop" ) );
    buildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildProjectButton->sizePolicy().hasHeightForWidth() ) );
    buildProjectButton->setEnabled ( true );
    QToolTip::add( buildProjectButton, i18n( "Build project" ) );
    QWhatsThis::add(buildProjectButton, i18n("<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    // rebuild
    rebuildProjectButton = new QToolButton ( projectTools, "Rebuild button" );
    rebuildProjectButton->setPixmap ( SmallIcon ( "rebuild" ) );
    rebuildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, rebuildProjectButton->sizePolicy().hasHeightForWidth() ) );
    rebuildProjectButton->setEnabled ( true );
    QToolTip::add( rebuildProjectButton, i18n( "Rebuild project" ) );
    QWhatsThis::add(rebuildProjectButton, i18n("<b>Rebuild project</b><p>Runs <b>make clean</b> and then <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    // run
    executeProjectButton = new QToolButton ( projectTools, "Run button" );
    executeProjectButton->setPixmap ( SmallIcon ( "exec" ) );
    executeProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, executeProjectButton->sizePolicy().hasHeightForWidth() ) );
    executeProjectButton->setEnabled ( true );
    QToolTip::add( executeProjectButton, i18n( "Execute main program" ) );
    QWhatsThis::add(executeProjectButton, i18n("<b>Execute main program</b><p>Executes the main program specified in project settings, <b>Run Options</b> tab."));
    // spacer
    QWidget *spacer = new QWidget(projectTools);
    projectTools->setStretchFactor(spacer, 1);
    // Project configuration
    projectconfButton = new QToolButton ( projectTools, "Project configuration button" );
    projectconfButton->setPixmap ( SmallIcon ( "configure" ) );
    projectconfButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, projectconfButton->sizePolicy().hasHeightForWidth() ) );
    projectconfButton->setEnabled ( true );
    QToolTip::add( projectconfButton, i18n( "Subproject settings" ) );
    QWhatsThis::add(projectconfButton, i18n("<b>Subproject settings</b><p>Opens <b>QMake Subproject Configuration</b> dialog for the currently selected subproject. "
        "It provides settings for:<br>subproject type and configuration,<br>include and library paths,<br>lists of dependencies and "
        "external libraries,<br>build order,<br>intermediate files locations,<br>compiler options."));

    // Project button connections
    connect ( addSubdirButton, SIGNAL ( clicked () ), this, SLOT ( slotAddSubdir () ) );
    connect ( createScopeButton, SIGNAL ( clicked () ), this, SLOT ( slotCreateScope () ) );
    connect ( buildFileButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildFile () ) );

    connect ( buildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildProject () ) );
    connect ( rebuildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotRebuildProject () ) );
    connect ( executeProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotExecuteProject () ) );



    connect ( projectconfButton, SIGNAL ( clicked () ), this, SLOT ( slotConfigureProject () ) );

    // Project tree
    overview = new TrollListView(this, overviewContainer, SubprojectView, "project overview widget");
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

    // Add new file button
    newfileButton = new QToolButton ( fileTools, "Create new file" );
    newfileButton->setPixmap ( SmallIcon ( "filenew" ) );
    newfileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, newfileButton->sizePolicy().hasHeightForWidth() ) );
    newfileButton->setEnabled ( true );
    QToolTip::add( newfileButton, i18n( "Create new file" ) );
    QWhatsThis::add(newfileButton, i18n("<b>Create new file</b><p>Creates a new file and adds it to a currently selected group."));

    // Add existing files button
    addfilesButton = new QToolButton ( fileTools, "Add existing files" );
    addfilesButton->setPixmap ( SmallIcon ( "fileimport" ) );
    addfilesButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, addfilesButton->sizePolicy().hasHeightForWidth() ) );
    addfilesButton->setEnabled ( true );
    QToolTip::add( addfilesButton, i18n( "Add existing files" ) );
    QWhatsThis::add(addfilesButton, i18n("<b>Add existing files</b><p>Adds existing files to a currently selected group. It is "
            "possible to copy files to a current subproject directory, create symbolic links or "
            "add them with the relative path."));

    // remove file button
    removefileButton = new QToolButton ( fileTools, "Remove file" );
    removefileButton->setPixmap ( SmallIcon ( "button_cancel" ) );
    removefileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, removefileButton->sizePolicy().hasHeightForWidth() ) );
    removefileButton->setEnabled ( true );
    QToolTip::add( removefileButton, i18n( "Remove file" ) );
    QWhatsThis::add(removefileButton, i18n("<b>Remove file</b><p>Removes file from a current group. Does not remove file from disk."));

    // build
    buildTargetButton = new QToolButton ( fileTools, "Make sp button" );
    buildTargetButton->setPixmap ( SmallIcon ( "make_kdevelop" ) );
    buildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, buildTargetButton->sizePolicy().hasHeightForWidth() ) );
    buildTargetButton->setEnabled ( true );
    QToolTip::add( buildTargetButton, i18n( "Build subproject" ) );
    QWhatsThis::add(buildTargetButton, i18n("<b>Build subproject</b><p>Runs <b>make</b> from the current subproject directory. "
                              "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    // rebuild
    rebuildTargetButton = new QToolButton ( fileTools, "Rebuild sp button" );
    rebuildTargetButton->setPixmap ( SmallIcon ( "rebuild" ) );
    rebuildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, rebuildTargetButton->sizePolicy().hasHeightForWidth() ) );
    rebuildTargetButton->setEnabled ( true );
    QToolTip::add( rebuildTargetButton, i18n( "Rebuild subproject" ) );
    QWhatsThis::add(rebuildTargetButton, i18n("<b>Rebuild subproject</b><p>Runs <b>make clean</b> and then <b>make</b> from the current subproject directory. "
                              "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    // run
    executeTargetButton = new QToolButton ( fileTools, "Run sp button" );
    executeTargetButton->setPixmap ( SmallIcon ( "exec" ) );
    executeTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, executeTargetButton->sizePolicy().hasHeightForWidth() ) );
    executeTargetButton->setEnabled ( true );
    QToolTip::add( executeTargetButton, i18n( "Execute subproject" ) );
    QWhatsThis::add(executeTargetButton, i18n("<b>Execute subproject</b><p>Executes the target program for the currently selected subproject. "
        "This action is allowed only if a type of the subproject is 'application'. The type of the subproject can be "
        "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)."));


    // spacer
    spacer = new QWidget(fileTools);
    projectTools->setStretchFactor(spacer, 1);

    // Configure file button
    configurefileButton = new QToolButton ( fileTools, "Configure file" );
    configurefileButton->setPixmap ( SmallIcon ( "configure_file" ) );
    configurefileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType) 0, 0, 0, configurefileButton->sizePolicy().hasHeightForWidth() ) );
    configurefileButton->setEnabled ( true );
    QToolTip::add( configurefileButton, i18n( "Configure file" ) );
    QWhatsThis::add(configurefileButton, i18n("<b>Configure file</b><p>Opens <b>File Properties</b> dialog that allows a file to be excluded from specified scopes."));

    // detail tree
    details = new TrollListView(this, detailContainer, DetailsView, "details widget");
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
    m_rootSubproject = 0;
}


TrollProjectWidget::~TrollProjectWidget()
{}


void TrollProjectWidget::openProject(const QString &dirName)
{
    QDomDocument &dom = *(m_part->projectDom());
    m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevtrollproject/subclassing" ,
                                                    "subclass","sourcefile", "uifile");
    SubqmakeprojectItem *item = new SubqmakeprojectItem(overview, "/","");
    item->subdir = dirName.right(dirName.length()-dirName.findRev('/')-1);
    item->path = dirName;
    item->m_RootBuffer = &(item->m_FileBuffer);
    parse(item);
    item->setOpen(true);
    m_rootSubproject = item;
    overview->setSelected(item, true);
}


void TrollProjectWidget::closeProject()
{
    m_rootSubproject = 0;
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
        QString path = static_cast<SubqmakeprojectItem*>(it.current())->path;
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

        SubqmakeprojectItem *spitem = static_cast<SubqmakeprojectItem*>(item);
        QString path = spitem->path;

        for (QPtrListIterator<GroupItem> tit(spitem->groups); tit.current(); ++tit) {
            GroupItem::GroupType type = (*tit)->groupType;

            if (type == GroupItem::Sources || type == GroupItem::Headers || type == GroupItem::Forms || type == GroupItem::Images ||
	    	type ==  GroupItem::Lexsources || type ==  GroupItem::Yaccsources || type == GroupItem::Distfiles ||
		type ==  GroupItem::Translations || type ==  GroupItem::IDLs || type ==  GroupItem::InstallObject  ) {
		for (QPtrListIterator<FileItem> fit(tit.current()->files); fit.current(); ++fit){
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

    return static_cast<SubqmakeprojectItem*>(overview->firstChild())->path;
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
    else if (m_shownSubproject->configuration.m_template==QTMP_SUBDIRS)
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

    configurefileButton->setEnabled(false);
    newfileButton->setEnabled(true);
    removefileButton->setEnabled(false);
    addfilesButton->setEnabled(true);

    details->setEnabled(hasSourceFiles);
}

void TrollProjectWidget::slotOverviewSelectionChanged(QListViewItem *item)
{
    if (!item)
        return;
    cleanDetailView(m_shownSubproject);
    m_shownSubproject = static_cast<SubqmakeprojectItem*>(item);
    setupContext();
    buildProjectDetailTree(m_shownSubproject,details);

    QString subProjPath = m_shownSubproject->path;
    QString relpath = subProjPath.remove(0,projectDirectory().length()+1);
    QDomDocument &dom = *(m_part->projectDom());
    DomUtil::writeEntry(dom, "/kdevtrollproject/general/activedir",relpath);

}

QString TrollProjectWidget::getCurrentTarget()
{
  if (!m_shownSubproject)
    return "";
  if (m_shownSubproject->configuration.m_destdir.isEmpty() ||
      m_shownSubproject->configuration.m_destdir[ m_shownSubproject->configuration.m_destdir.length()-1 ] == '/' )
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
  if (m_shownSubproject->configuration.m_target.isEmpty())
  {
    QString exe = m_shownSubproject->pro_file;
    return exe.replace(QRegExp("\\.pro$"), "");
  }
  else
    return m_shownSubproject->configuration.m_target;
}

void TrollProjectWidget::cleanDetailView(SubqmakeprojectItem *item)
{
  // If no children in detailview
  // it is a subdir template
  if (item && details->childCount())
  {
//    if (item->configuration.m_template == QTMP_SUBDIRS)
//      return;
    // Remove all GroupItems and all of their children from the view
//    QPtrListIterator<SubqmakeprojectItem> it(item->scopes);
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

void TrollProjectWidget::buildProjectDetailTree(SubqmakeprojectItem *item,KListView *listviewControl)
{
//  if (item->configuration.m_template == QTMP_SUBDIRS)
//    return;

  // Insert all GroupItems and all of their children into the view
  if (listviewControl)
  {
//    QPtrListIterator<SubqmakeprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      listviewControl->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QPtrListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        listviewControl->insertItem(*it2);
        if ((*it2)->groupType==GroupItem::InstallRoot)
        {
          QPtrListIterator<GroupItem> it3((*it2)->installs);
          for (; it3.current(); ++it3)
          {
              (*it2)->insertItem(*it3);
              QPtrListIterator<FileItem> it4((*it3)->files);
              for (; it4.current(); ++it4)
                  (*it3)->insertItem(*it4);
              (*it3)->setOpen(true);
              (*it3)->sortChildItems(0,true);
          }
          (*it2)->setOpen(true);
          (*it2)->sortChildItems(0,true);
        }
        else
        {
          QPtrListIterator<FileItem> it3((*it2)->files);
          for (; it3.current(); ++it3)
              (*it2)->insertItem(*it3);
          (*it2)->setOpen(true);
          (*it2)->sortChildItems(0,true);
        }
    }
  }
  else
  {
//    QPtrListIterator<SubqmakeprojectItem> it1(item->scopes);
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
        (*it2)->sortChildItems(0,true);
    }
  }
}

void TrollProjectWidget::slotDetailsExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are qProjectItem's
    qProjectItem *pvitem = static_cast<qProjectItem*>(item);
    if (pvitem->type() != qProjectItem::File)
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
}


void TrollProjectWidget::slotConfigureProject()
{
//  ProjectOptionsDlg *d = new ProjectOptionsDlg(m_part,this);
//  d->exec();

  ProjectConfigurationDlg *dlg = new ProjectConfigurationDlg(m_shownSubproject,overview);
  if (dlg->exec() == QDialog::Accepted)
  {
    updateProjectConfiguration(m_shownSubproject);
    setupContext();
  }
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


  // Only run application projects
  if (m_shownSubproject->configuration.m_template!=QTMP_APPLICATION)
    return;

  QString dircmd = "cd "+KProcess::quote(subprojectDirectory() + "/" + getCurrentDestDir()) + " && ";
  QString program = KProcess::quote("./" + getCurrentOutputFilename());

    // Build environment variables to prepend to the executable path
    QString runEnvVars = QString::null;
    DomUtil::PairList list =
        DomUtil::readPairListEntry( *(m_part->projectDom()), "/kdevtrollproject/run/envvars", "envvar", "name", "value" );

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        const DomUtil::Pair &pair = (*it);
        if ( (!pair.first.isEmpty()) && (!pair.second.isEmpty()) )
            runEnvVars += pair.first + "=" + pair.second + " ";
    }
    program.prepend(runEnvVars);

    program.append(" " + DomUtil::readEntry( *(m_part->projectDom()), "/kdevtrollproject/run/programargs" ) + " ");
//  std::cerr<<dircmd + "./"+program<<std::endl;
//  m_part->execute(dircmd + "./"+program);
//  m_part->appFrontend()->startAppCommand(dircmd +"./"+program,true);

  bool inTerminal = DomUtil::readBoolEntry(*m_part->projectDom(), "/kdevtrollproject/run/terminal");
  m_part->appFrontend()->startAppCommand(subprojectDirectory() + "/" + getCurrentDestDir(), program, inTerminal );

}

void TrollProjectWidget::slotBuildProject()
{
  m_part->partController()->saveAllFiles();
  QString dir = projectDirectory();

  if (!m_rootSubproject)
    return;

  createMakefileIfMissing(dir, m_rootSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString buildcmd = constructMakeCommandLine(m_rootSubproject->configuration.m_makefile);
  m_part->queueCmd(dir,dircmd + buildcmd);
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
  createMakefileIfMissing(dir, m_shownSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString buildcmd = constructMakeCommandLine(m_shownSubproject->configuration.m_makefile);
  m_part->queueCmd(dir,dircmd + buildcmd);
}

void TrollProjectWidget::slotRebuildProject()
{
  m_part->partController()->saveAllFiles();
  QString dir = this->  projectDirectory();

  if (!m_rootSubproject)
    return;

  createMakefileIfMissing(dir, m_rootSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString rebuildcmd = constructMakeCommandLine(m_rootSubproject->configuration.m_makefile) + " clean && " + constructMakeCommandLine(m_rootSubproject->configuration.m_makefile);
  m_part->queueCmd(dir,dircmd + rebuildcmd);
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
  createMakefileIfMissing(dir, m_shownSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString rebuildcmd = constructMakeCommandLine(m_shownSubproject->configuration.m_makefile) + " clean && " + constructMakeCommandLine(m_shownSubproject->configuration.m_makefile);
  m_part->queueCmd(dir,dircmd + rebuildcmd);
}

void TrollProjectWidget::slotCreateScope(SubqmakeprojectItem *spitem)
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
    spitem = m_shownSubproject;
  bool ok = FALSE;
  QString scopename = KInputDialog::getText(
                      i18n( "Create Scope" ),
                      i18n( "Please enter a name for the new scope:" ),
                      QString::null, &ok, this );
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

void TrollProjectWidget::slotAddSubdir(SubqmakeprojectItem *spitem)
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
    spitem = m_shownSubproject;
  QString relpath = spitem->path.mid(projectDirectory().length());

  KURLRequesterDlg dialog(i18n( "Add Subdirectory" ), i18n( "Please enter a name for the subdirectory: " ), this, 0);
  dialog.urlRequester()->setMode(KFile::Directory);
  dialog.urlRequester()->setURL(QString::null);

  if ( dialog.exec() == QDialog::Accepted && !dialog.urlRequester()->url().isEmpty() )
  {
    QString subdirname;
    if ( !QDir::isRelativePath(dialog.urlRequester()->url()) )
    subdirname = getRelativePath( m_shownSubproject->path, dialog.urlRequester()->url() );
    else
    subdirname = dialog.urlRequester()->url();

    QDir dir(projectDirectory()+relpath);
    if (!dir.exists(subdirname))
    {
     if (!dir.mkdir(subdirname))
     {
       KMessageBox::error(this,i18n("Failed to create subdirectory. "
                                    "Do you have write permission "
                                    "in the project folder?" ));
       return;
     }
    }
    spitem->subdirs.append(subdirname);
    updateProjectFile(spitem);
    SubqmakeprojectItem *newitem = new SubqmakeprojectItem(spitem, subdirname,"");
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

void TrollProjectWidget::slotRemoveSubproject(SubqmakeprojectItem *spitem)
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
  {
    if ( ( spitem = dynamic_cast<SubqmakeprojectItem *>(m_shownSubproject->parent()) ) != NULL  )
    {
    QString subdirname = m_shownSubproject->subdir;
    spitem->subdirs.remove(subdirname);
    delete m_shownSubproject;
    m_shownSubproject = spitem;
    updateProjectFile(spitem);
    overview->setCurrentItem(m_shownSubproject);
    overview->setSelected(m_shownSubproject, true);
    }
  }
}

void TrollProjectWidget::slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    SubqmakeprojectItem *spitem = static_cast<SubqmakeprojectItem*>(item);

    KPopupMenu popup(this);
    popup.insertTitle(i18n("Subproject %1").arg(item->text(0)));

    int idBuild = -2;
    int idQmake = -2;
    int idProjectConfiguration = -2;
    int idAddSubproject = -2;
    int idRemoveSubproject = -2;
    int idRemoveScope = -2;
    int idAddScope = -2;


    if (!spitem->isScope)
    {
      idBuild = popup.insertItem(SmallIcon("make_kdevelop"),i18n("Build"));
      popup.setWhatsThis(idBuild, i18n("<b>Build</b><p>Runs <b>make</b> from the selected subproject directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
      idQmake = popup.insertItem(SmallIcon("qmakerun"),i18n("Run qmake"));
      popup.setWhatsThis(idQmake, i18n("<b>Run qmake</b><p>Runs <b>qmake</b> from the selected subproject directory. This creates or regenerates Makefile."));
      popup.insertSeparator();
      idAddSubproject = popup.insertItem(SmallIcon("folder_new"),i18n("Add Subproject..."));
      popup.setWhatsThis(idAddSubproject, i18n("<b>Add subproject</b><p>Creates a <i>new</i> or adds an <i>existing</i> subproject to a currently selected subproject. "
        "This action is allowed only if a type of the subproject is 'subdirectories'. The type of the subproject can be "
        "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)."));
      if (spitem->configuration.m_template != QTMP_SUBDIRS)
        popup.setItemEnabled(idAddSubproject, false);
      idRemoveSubproject = popup.insertItem(SmallIcon("remove_subdir"),i18n("Remove Subproject..."));
      popup.setWhatsThis(idRemoveSubproject, i18n("<b>Remove subproject</b><p>Removes currently selected subproject. Does not delete any file from disk. Deleted subproject can be later added by calling 'Add Subproject' action."));
      if (spitem->parent() == NULL)
        popup.setItemEnabled(idRemoveSubproject, false);
      idAddScope = popup.insertItem(SmallIcon("qmake_scopenew"),i18n("Create Scope..."));
      popup.setWhatsThis(idAddScope, i18n("<b>Create scope</b><p>Creates QMake scope in the project file of the currently selected subproject."));
      popup.insertSeparator();
      idProjectConfiguration = popup.insertItem(SmallIcon("configure"),i18n("Subproject Settings"));
      popup.setWhatsThis(idProjectConfiguration, i18n("<b>Subproject settings</b><p>Opens <b>QMake Subproject Configuration</b> dialog. "
        "It provides settings for:<br>subproject type and configuration,<br>include and library paths,<br>lists of dependencies and "
        "external libraries,<br>build order,<br>intermediate files locations,<br>compiler options."));
    }
    else
    {
      idAddScope = popup.insertItem(SmallIcon("qmake_scopenew"),i18n("Create Scope..."));
      popup.setWhatsThis(idAddScope, i18n("<b>Create Scope</b><p>Creates QMake scope in the currently selected scope."));
      idRemoveScope = popup.insertItem(SmallIcon("editdelete"),i18n("Remove Scope"));
      popup.setWhatsThis(idRemoveScope, i18n("<b>Remove Scope</b><p>Removes currently selected scope."));
    }

    int r = popup.exec(p);

    QString relpath = spitem->path.mid(projectDirectory().length());
    if (r == idAddSubproject)
    {
      slotAddSubdir(spitem);
    }
    if (r == idRemoveSubproject)
    {
      slotRemoveSubproject(spitem);
    }
    if (r == idAddScope)
    {
      slotCreateScope(spitem);
    }
    else if (r == idRemoveScope)
    {
      slotRemoveScope(spitem);
    }
    else if (r == idBuild)
    {
        slotBuildTarget();
//        m_part->startMakeCommand(projectDirectory() + relpath, QString::fromLatin1(""));
    }
    else if (r == idQmake)
    {
        m_part->startQMakeCommand(projectDirectory() + relpath);
    }
    else if (r == idProjectConfiguration)
    {
      ProjectConfigurationDlg *dlg = new ProjectConfigurationDlg(spitem,overview);
      if (dlg->exec() == QDialog::Accepted)
        updateProjectConfiguration(spitem);
    }
}

void TrollProjectWidget::updateProjectConfiguration(SubqmakeprojectItem *item)
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
  if (item->configuration.m_requirements & QD_ORDERED)
    configList.append("ordered");
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
  Buffer->removeValues("MAKEFILE");
  if (!item->configuration.m_makefile.simplifyWhiteSpace().isEmpty())
    Buffer->setValues("MAKEFILE", item->configuration.m_makefile,FileBuffer::VSM_RESET,VALUES_PER_ROW);



  Buffer->removeValues("INCLUDEPATH");
  if (item->configuration.m_incadd.count()>0)
    Buffer->setValues("INCLUDEPATH",item->configuration.m_incadd,FileBuffer::VSM_RESET,VALUES_PER_ROW);

  Buffer->removeValues("LIBS");
  if (item->configuration.m_libadd.count()>0)
    Buffer->setValues("LIBS",item->configuration.m_libadd,FileBuffer::VSM_APPEND,VALUES_PER_ROW);

  Buffer->removeValues("TARGETDEPS");
//  if (item->configuration.m_libadd.count()>0)
  Buffer->setValues("TARGETDEPS",item->configuration.m_prjdeps,FileBuffer::VSM_APPEND,VALUES_PER_ROW);

  updateInstallObjects(item,Buffer);


  // Write to .pro file
//  Buffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->subdir+".pro",getHeader());
  Buffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->pro_file,getHeader());
}

SubqmakeprojectItem* TrollProjectWidget::getScope(SubqmakeprojectItem *baseItem,const QString &scopeString)
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
  QPtrListIterator<SubqmakeprojectItem> spit(baseItem->scopes);
  for (; spit.current(); ++spit)
  {
    SubqmakeprojectItem *spitem = spit;
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
  SubqmakeprojectItem *spitem = static_cast<SubqmakeprojectItem*>(item);
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
  subBuffer->setValues("IDLS",spitem->idls_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("IMAGES");
  subBuffer->setValues("IMAGES",spitem->images,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("IMAGES",spitem->images_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("DISTFILES");
  subBuffer->setValues("DISTFILES",spitem->distfiles,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("DISTFILES",spitem->distfiles_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("LEXSOURCES");
  subBuffer->setValues("LEXSOURCES",spitem->lexsources,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("LEXSOURCES",spitem->lexsources_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);

  subBuffer->removeValues("YACCSOURCES");
  subBuffer->setValues("YACCSOURCES",spitem->yaccsources,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("YACCSOURCES",spitem->yaccsources_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);


  subBuffer->removeValues("TRANSLATIONS");
  subBuffer->setValues("TRANSLATIONS",spitem->translations,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  subBuffer->setValues("TRANSLATIONS",spitem->translations_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);


  if( m_part->isTMakeProject() ) {
      subBuffer->removeValues("INTERFACES");
      subBuffer->setValues("INTERFACES",spitem->forms,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
      subBuffer->setValues("INTERFACES",spitem->forms_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
  } else {
      subBuffer->removeValues("FORMS");
      subBuffer->setValues("FORMS",spitem->forms,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
      subBuffer->setValues("FORMS",spitem->forms_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
  }

  updateInstallObjects(spitem,subBuffer);
//  m_shownSubproject->m_RootBuffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->subdir+".pro",getHeader());
  m_shownSubproject->m_RootBuffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->pro_file,getHeader());
}

void TrollProjectWidget::updateInstallObjects(SubqmakeprojectItem* item, FileBuffer* subBuffer)
{
  // Install objects
  GroupItem* instroot = getInstallRoot(item);
  QPtrListIterator<GroupItem> it(instroot->installs);
  QStringList instobjects;

  for (;it.current();++it)
  {
    GroupItem* iobj = *it;
    subBuffer->removeValues(iobj->install_objectname+".path");
    subBuffer->removeValues(iobj->install_objectname+".files");

    if (!iobj->str_files.isEmpty() ||
        !iobj->str_files_exclude.isEmpty())
    {
      instobjects.append(iobj->install_objectname);
      subBuffer->setValues(iobj->install_objectname+".path",iobj->install_path,FileBuffer::VSM_RESET,VALUES_PER_ROW);
      subBuffer->setValues(iobj->install_objectname+".files",iobj->str_files,FileBuffer::VSM_APPEND,VALUES_PER_ROW);
      subBuffer->setValues(iobj->install_objectname+".files",iobj->str_files_exclude,FileBuffer::VSM_EXCLUDE,VALUES_PER_ROW);
    }
  }

  if (!item->configuration.m_target_install_path.isEmpty() &&
      item->configuration.m_target_install)
  {
    instobjects.append("target");
    subBuffer->removeValues("target.path");
    subBuffer->setValues("target.path",item->configuration.m_target_install_path,FileBuffer::VSM_RESET,VALUES_PER_ROW);
    subBuffer->setValues("INSTALLS",QString("target"),FileBuffer::VSM_APPEND,VALUES_PER_ROW);
  }

  subBuffer->removeValues("INSTALLS");
  subBuffer->setValues("INSTALLS",instobjects,FileBuffer::VSM_APPEND,VALUES_PER_ROW);


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
  header.sprintf(m_part->getQMakeHeader().latin1(),
                 relpath.ascii(),
                 templateString.ascii(),
                 targetString.ascii());
  return header;
}


void TrollProjectWidget::addFileToCurrentSubProject(GroupItem *titem,const QString &filename)
{
  QPtrListIterator<FileItem> it( titem->files );
  while ( it.current() != 0 )
  {
     if(it.current()->name == filename) //File already exists in this subproject
       return;
     ++it;
  }

  FileItem *fitem = createFileItem(filename);

  fitem->uiFileLink = getUiFileLink(titem->owner->relpath+"/",filename);
  if (titem->groupType != GroupItem::InstallObject)
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
    case GroupItem::Lexsources:
      titem->owner->lexsources.append(filename);
      break;
    case GroupItem::Yaccsources:
      titem->owner->yaccsources.append(filename);
      break;
    case GroupItem::Images:
      titem->owner->images.append(filename);
      break;
    case GroupItem::Distfiles:
      titem->owner->distfiles.append(filename);
      break;
    case GroupItem::Translations:
      titem->owner->translations.append(filename);
      break;
    case GroupItem::InstallObject:
      titem->str_files.append(filename);
      titem->files.append(fitem);
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
       QPtrListIterator<FileItem> it( gitem->files );
       while ( it.current() != 0 )
       {
        if(it.current()->name == filename) //File already exists in this subproject
          return;
        ++it;
       }
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
    case GroupItem::Lexsources:
      m_shownSubproject->lexsources.append(filename);
      break;
    case GroupItem::Yaccsources:
      m_shownSubproject->yaccsources.append(filename);
      break;
    case GroupItem::Translations:
      m_shownSubproject->translations.append(filename);
      break;
    case GroupItem::Images:
      m_shownSubproject->images.append(filename);
    case GroupItem::Distfiles:
      m_shownSubproject->distfiles.append(filename);
      break;
    /*
    case GroupItem::InstallObject:
      GroupItem *gitem = 0;

      QPtrListIterator<GroupItem> it(m_shownSubproject->groups);
      for (; it.current(); ++it)
      {
        if ((*it)->groupType == GroupItem::InstallRoot)
        {
          gitem = *it;
          break;
        }
      }
      QPtrListIterator<GroupItem> it2(gitem->installs);
      for (; it2.current(); ++it2)
      {
        if ((*it2)->install_objectname == )
        {
          if ();
        }
      }

      m_shownSubproject->files.append(fitem);
      break;
    */
    default:
      break;
  }
}

/**
* Method adds a file to the current project by grouped
* by file extension
*/
void TrollProjectWidget::addFiles( QStringList &files, bool noPathTruncate)
{
  if (!m_shownSubproject)
    return;

  QString newPath;

  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
  {
    QString fileName = *it;

    QString origFileName = noPathTruncate ? fileName : QFileInfo(fileName).fileName();
    QString origFilePath = noPathTruncate ? QDir::cleanDirPath(m_shownSubproject->path + "/" + fileName) : fileName;
    if (m_shownSubproject->configuration.m_template == QTMP_SUBDIRS)
    {
        ChooseSubprojectDlg dlg(this);
        if (dlg.exec() == QDialog::Accepted)
        {
            if (dlg.selectedSubproject())
            {
                overview->setCurrentItem(dlg.selectedSubproject());
                newPath = dlg.selectedSubproject()->path;
            }
        }
        else
            return;
    }

    if (!newPath.isEmpty())
    {
        //move file to it's new location
        KURL source;
        KURL dest;
        kdDebug() << "  orig: " << origFilePath << "  dest: " << QDir::cleanDirPath(newPath + "/" + origFileName) << endl;
        source.setPath(origFilePath);
        dest.setPath(QDir::cleanDirPath(newPath + "/" + origFileName));
        if (KIO::NetAccess::copy(source, dest, (QWidget*)0))
            KIO::NetAccess::del(source, (QWidget*)0);
        *it = QDir::cleanDirPath(newPath + "/" + origFileName);
        fileName = *it;
    }

    QFileInfo info(fileName);
    QString ext = info.extension(false).simplifyWhiteSpace();
    QString noPathFileName;
    if (noPathTruncate)
        noPathFileName = fileName;
    else
        noPathFileName = info.fileName();

    GroupItem *gitem = 0;
    QPtrListIterator<GroupItem> it(m_shownSubproject->groups);
    for (; it.current(); ++it)
    {
        if ((*it)->groupType == GroupItem::groupTypeForExtension(ext))
        {
            gitem = *it;
            break;
        }
    }
    if (gitem && !noPathTruncate)
    {
        QString addName;
        if (fileName.startsWith("/"))
            addName = URLUtil::relativePath(gitem->owner->path, fileName);
        else
            addName = URLUtil::relativePath(gitem->owner->relpath, "/" + fileName);
        if (!addName.isEmpty())
        {
            if (addName[0] == '/')
                addName = addName.mid(1);
            noPathFileName = addName;
        }
    }

    addFileToCurrentSubProject(GroupItem::groupTypeForExtension(ext), noPathFileName);
    updateProjectFile(m_shownSubproject);
    slotOverviewSelectionChanged(m_shownSubproject);
    emitAddedFile ( fileName.mid(m_part->projectDirectory().length()+1) );
  }


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
  else if (QString("ts").find(ext)>-1)
    addFileToCurrentSubProject(GroupItem::Translations,fileWithNoSlash);
  else
    addFileToCurrentSubProject(GroupItem::NoType,fileWithNoSlash);*/

}


void TrollProjectWidget::slotAddFiles()
{
  QString cleanSubprojectDir = QDir::cleanDirPath(m_shownSubproject->path);
  QString  filter = "*.cpp *.cc *.c *.hpp *.h *.ui|" + i18n("Source Files");
  filter += "\n*|" + i18n("All Files");
#if KDE_VERSION >= 310
  AddFilesDialog *dialog = new AddFilesDialog(cleanSubprojectDir,
                                        filter,
                                        this,
                                        "Insert existing files",
                                        true, new QComboBox(false));
#else
  AddFilesDialog *dialog = new AddFilesDialog(cleanSubprojectDir,
                                        filter,
                                        this,
                                        i18n("Insert Existing Files").ascii(),
                                        true);
#endif
  dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
  dialog->exec();
  QStringList files = dialog->selectedFiles();
  for (unsigned int i=0;i<files.count();i++)
  {
    switch (dialog->mode())
    {
      case AddFilesDialog::Copy:
        {
        // Copy selected files to current subproject folder
        QProcess *proc = new QProcess( this );
        proc->addArgument( "cp" );
        proc->addArgument( "-f" );
        proc->addArgument( files[i] );
        proc->addArgument( cleanSubprojectDir );
        proc->start();
        QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
        // and add them to the filelist
        QFile testExist(cleanSubprojectDir+"/"+filename);
        if (testExist.exists())
        {
          QStringList files(filename);
          addFiles(files);
        }
        }
        break;

      case AddFilesDialog::Link:
        {
        // Link selected files to current subproject folder
        QProcess *proc = new QProcess( this );
        proc->addArgument( "ln" );
        proc->addArgument( "-s" );
        proc->addArgument( files[i] );
        proc->addArgument( cleanSubprojectDir );
        proc->start();
        QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
        // and add them to the filelist
        QFile testExist(cleanSubprojectDir+"/"+filename);
        if (testExist.exists())
        {
          QStringList files(filename);
          addFiles(files);
        }
        }
        break;

      case AddFilesDialog::Relative:
        // Form relative path to current subproject folder
        QStringList files(URLUtil::relativePathToFile(cleanSubprojectDir , files[i]));
        addFiles(files, true);
        break;
    }
  }
}

GroupItem* TrollProjectWidget::getInstallRoot(SubqmakeprojectItem* item)
{
  QPtrListIterator<GroupItem> it(item->groups);
  for (;it.current();++it)
  {
    if ((*it)->groupType == GroupItem::InstallRoot)
      return *it;
  }
  return 0;
}

GroupItem* TrollProjectWidget::getInstallObject(SubqmakeprojectItem* item, const QString& objectname)
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

void TrollProjectWidget::slotNewFile()
{
    GroupItem *gitem = static_cast<GroupItem*>(details->currentItem());
    if (gitem)
    {
      if (gitem->groupType == GroupItem::InstallObject)
      {
          // QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
          bool ok = FALSE;
          QString filepattern = KInputDialog::getText(
                              i18n( "Insert New Filepattern" ),
                              i18n( "Please enter a filepattern relative the current "
                                    "subproject (example docs/*.html):" ),
                              QString::null, &ok, this );
          if ( ok && !filepattern.isEmpty() )
          {
            addFileToCurrentSubProject(gitem,filepattern);
            updateProjectFile(gitem->owner);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
          return;
      }
      if (gitem->groupType == GroupItem::InstallRoot)
      {
//          QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
          bool ok = FALSE;
          QString install_obj = KInputDialog::getText(
                              i18n( "Insert New Install Object" ),
                              i18n( "Please enter a name for the new object:" ),
                              QString::null, &ok, this );
          if ( ok && !install_obj.isEmpty() )
          {
            GroupItem* institem = createGroupItem(GroupItem::InstallObject, install_obj ,gitem->scopeString);
            institem->owner = m_shownSubproject;
            institem->install_objectname = install_obj;
            gitem->installs.append(institem);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
          return;
      }

    }
    KDevCreateFile * createFileSupport = m_part->createFileSupport();
    if (createFileSupport)
    {
        KDevCreateFile::CreatedFile crFile =
            createFileSupport->createNewFile(QString::null, projectDirectory()+m_shownSubproject->path.mid(projectDirectory().length()));
    } else {
        bool ok = FALSE;
        QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
        QString filename = KInputDialog::getText(
                            i18n( "Insert New File"),
                            i18n( "Please enter a name for the new file:" ),
                            QString::null, &ok, this );
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
            QStringList files(relpath+'/'+filename);
            addFiles(files);
        }
    }
}

void TrollProjectWidget::slotRemoveFile()
{
  QListViewItem *selectedItem = details->currentItem();
  if (!selectedItem)
    return;
  qProjectItem *pvitem = static_cast<qProjectItem*>(selectedItem);
  // Check that it is a file (just in case)
  if (pvitem->type() != qProjectItem::File)
    return;
  FileItem *fitem = static_cast<FileItem*>(pvitem);
  removeFile(m_shownSubproject, fitem);
}

void TrollProjectWidget::slotConfigureFile()
{
  QListViewItem *selectedItem = details->currentItem();
  if (!selectedItem)
    return;
  qProjectItem *pvitem = static_cast<qProjectItem*>(selectedItem);
  // Check that it is a file (just in case)
  if (pvitem->type() != qProjectItem::File)
    return;
  FileItem *fitem = static_cast<FileItem*>(pvitem);

  GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());
  if (!gitem)
    return;
  QStringList dirtyScopes;
  FilePropertyDlg *propdlg = new FilePropertyDlg(m_shownSubproject,gitem->groupType,fitem,dirtyScopes);
  SubqmakeprojectItem *scope;
  propdlg->exec();

  for (uint i=0; i<dirtyScopes.count();i++)
  {
    scope = getScope(m_shownSubproject,dirtyScopes[i]);
    if (gitem->groupType == GroupItem::InstallObject)
    {
      GroupItem* instroot = getInstallRoot(scope);
      GroupItem* instobj = getInstallObject(scope,gitem->install_objectname);
      if (!instobj)
      {
        GroupItem* institem = createGroupItem(GroupItem::InstallObject, gitem->install_objectname ,scope->scopeString);
        institem->owner = scope;
        institem->install_objectname = gitem->install_objectname;
        instroot->installs.append(institem);
        instobj = institem;
      }
      // Using the boolean nature of this operation I can append or remove the file from the excludelist
      if (instobj->str_files_exclude.join(":").find(fitem->name) >= 0)
      {
        instobj->str_files_exclude.remove(fitem->name);
      }
      else
      {
        instobj->str_files_exclude.append(fitem->name);
      }
    }
    if (scope)
      updateProjectFile(scope);
  }
}

void TrollProjectWidget::slotDetailsSelectionChanged(QListViewItem *item)
{
    if (!item)
    {
//        addfilesButton->setEnabled(false);
//        newfileButton->setEnabled(false);
        removefileButton->setEnabled(false);
        configurefileButton->setEnabled(false);
        return;
    }
//    addfilesButton->setEnabled(false);
//    newfileButton->setEnabled(false);
    removefileButton->setEnabled(false);
    configurefileButton->setEnabled(false);
/*    buildTargetButton->setEnabled(false);
    rebuildTargetButton->setEnabled(false);
    executeTargetButton->setEnabled(false);*/

    qProjectItem *pvitem = static_cast<qProjectItem*>(item);
    if (pvitem->type() == qProjectItem::Group)
    {
      GroupItem* gitem = static_cast<GroupItem*>(item);
      if (gitem->groupType == GroupItem::InstallObject)
      {
        configurefileButton->setEnabled(true);
        newfileButton->setEnabled(true);
      }
      else if (gitem->groupType == GroupItem::InstallRoot)
      {
        newfileButton->setEnabled(true);
      }
      else
      {
        addfilesButton->setEnabled(true);
        newfileButton->setEnabled(true);
      }


    }
    else if (pvitem->type() == qProjectItem::File)
    {
        removefileButton->setEnabled(true);
        configurefileButton->setEnabled(true);
/*        buildTargetButton->setEnabled(true);
        rebuildTargetButton->setEnabled(true);
        executeTargetButton->setEnabled(true);*/
    }
}

void TrollProjectWidget::slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    qProjectItem *pvitem = static_cast<qProjectItem*>(item);
    if (pvitem->type() == qProjectItem::Group) {
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
        case GroupItem::Lexsources:
            title = i18n("Lexsources");
            ext = "*.l *.ll *.lxx *.l++";
            break;
        case GroupItem::Yaccsources:
            title = i18n("Yaccsources");
            ext = "*.y *.yy *.yxx *.y++";
            break;
        case GroupItem::Images:
            title = i18n("Images");
            ext = "*.jpg *.png *.xpm *.gif";
            break;
        case GroupItem::Distfiles:
            title = i18n("Distfiles");
            ext = "*";
            break;
        case GroupItem::Translations:
            title = i18n("Translations");
            ext = "*.ts";
            break;
        case GroupItem::InstallRoot:
            title = i18n("Installs");
            break;
        case GroupItem::InstallObject:
            title = i18n("Install object");
            break;

        default: ;
        }

        KPopupMenu popup(this);
	popup.insertTitle(title);

        int idInsExistingFile = -2;
	int idInsNewFile = -2;
	int idInsInstallObject = -2;
	int idInsNewFilepatternItem = -2;
	int idSetInstObjPath = -2;
	int idLUpdate = -2;
	int idLRelease = -2;

 //       int idFileProperties = popup.insertItem(SmallIconSet("filenew"),i18n("Properties..."));
        if (titem->groupType == GroupItem::InstallRoot)
        {
	  idInsInstallObject = popup.insertItem(SmallIconSet("fileopen"),i18n("Add Install Object..."));
            popup.setWhatsThis(idInsInstallObject, i18n("<b>Add install object</b><p>Creates QMake install object. "
                "It is possible to define a list of files to install and installation locations for each object. Warning! "
                "Install objects without path specified will not be saved to a project file."));
        }
        else if (titem->groupType == GroupItem::InstallObject)
        {
	  idSetInstObjPath = popup.insertItem(SmallIconSet("fileopen"),i18n("Install Path..."));
            popup.setWhatsThis(idSetInstObjPath, i18n("<b>Install path</b><p>Allows to choose the installation path for the current install object."));
	  idInsNewFilepatternItem = popup.insertItem(SmallIconSet("fileopen"),i18n("Add Pattern of Files to Install..."));
            popup.setWhatsThis(idInsNewFilepatternItem, i18n("<b>Add pattern of files to install</b><p>Defines the pattern to match files which will be installed. "
                "It is possible to use wildcards and relative paths like <i>docs/*</i>."));
        }
        else if(titem->groupType == GroupItem::Translations)
        {
	  idInsNewFile = popup.insertItem(SmallIconSet("filenew"),i18n("Create New File..."));
            popup.setWhatsThis(idInsNewFile, i18n("<b>Create new file</b><p>Creates a new translation file and adds it to a currently selected TRANSLATIONS group."));
	  idInsExistingFile = popup.insertItem(SmallIconSet("fileopen"),i18n("Add Existing Files..."));
            popup.setWhatsThis(idInsExistingFile, i18n("<b>Add existing files</b><p>Adds existing translation (*.ts) files to a currently selected TRANSLATIONS group. It is "
            "possible to copy files to a current subproject directory, create symbolic links or "
            "add them with the relative path."));
	  idLUpdate = popup.insertItem(SmallIconSet("konsole"),i18n("Update Translation Files"));
            popup.setWhatsThis(idLUpdate, i18n("<b>Update Translation Files</b><p>Runs <b>lupdate</b> command from the current subproject directory. It collects translatable "
                "messages and saves them into translation files."));
	  idLRelease = popup.insertItem(SmallIconSet("konsole"),i18n("Release Binary Translations"));
            popup.setWhatsThis(idLRelease, i18n("<b>Release Binary Translations</b><p>Runs <b>lrelease</b> command from the current subproject directory. It creates binary "
                "translation files that are ready to be loaded at program execution."));
        }
        else // File group containing files
        {
 	  idInsNewFile = popup.insertItem(SmallIconSet("filenew"),i18n("Create New File..."));
            popup.setWhatsThis(idInsNewFile, i18n("<b>Create new file</b><p>Creates a new file and adds it to a currently selected group."));
	  idInsExistingFile = popup.insertItem(SmallIconSet("fileopen"),i18n("Add Existing Files..."));
            popup.setWhatsThis(idInsExistingFile, i18n("<b>Add existing files</b><p>Adds existing files to a currently selected group. It is "
            "possible to copy files to a current subproject directory, create symbolic links or "
            "add them with the relative path."));
        }
        int r = popup.exec(p);
        QString cleanSubprojectPath = QDir::cleanDirPath(m_shownSubproject->path);

        if (r == idSetInstObjPath)
        {
          KURLRequesterDlg dialog(i18n( "Choose Install Path" ), i18n( "Enter a path "
                                    "(example /usr/local/share/... ):" ), this, 0);
          dialog.urlRequester()->setMode(KFile::Directory);
          dialog.urlRequester()->setURL(titem->install_path);
          if (dialog.exec() == QDialog::Accepted)
          {
            titem->install_path = dialog.urlRequester()->url();
            updateProjectFile(titem->owner);
          }
        }
	    else if (r == idInsNewFilepatternItem)
        {
          bool ok = FALSE;
          QString filepattern = KInputDialog::getText(
                              i18n( "Add Pattern of Files to Install" ),
                              i18n( "Enter a pattern relative to the current "
                                    "subproject (example docs/*.html):" ),
                              QString::null, &ok, this );
          if ( ok && !filepattern.isEmpty() )
          {
            addFileToCurrentSubProject(titem,filepattern);
            updateProjectFile(titem->owner);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
        }
        else if (r == idInsExistingFile)
        {
#if KDE_VERSION >= 310
          AddFilesDialog *dialog = new AddFilesDialog(cleanSubprojectPath,
                                                ext + "|" + title + " (" + ext + ")",
                                                this,
                                                "Add existing files",
                                                true, new QComboBox(false));
#else
          AddFilesDialog *dialog = new AddFilesDialog(cleanSubprojectPath,
                                                ext + "|" + title + " (" + ext + ")",
                                                this,
                                                "Add existing files",
                                                true);
#endif
          dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
          if ( dialog->exec() == QDialog::Rejected )
            return;
          QStringList files = dialog->selectedFiles();
          for (unsigned int i=0;i<files.count();++i)
          {
            switch (dialog->mode())
            {
            case AddFilesDialog::Copy:
                {
                // Copy selected files to current subproject folder
                QProcess *proc = new QProcess( this );
                proc->addArgument( "cp" );
                proc->addArgument( "-f" );
                proc->addArgument( files[i] );
                proc->addArgument( cleanSubprojectPath );
                proc->start();
                QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
                // and add them to the filelist
                addFileToCurrentSubProject(titem,filename);

                QString fileNameToAdd = m_shownSubproject->relpath + "/" + filename;
                if (fileNameToAdd.startsWith("/"))
                    fileNameToAdd = fileNameToAdd.mid(1);
                QStringList fileList(fileNameToAdd);
                emit m_part->addedFilesToProject(fileList);
                }
                break;

            case AddFilesDialog::Link:
                {
                // Link selected files to current subproject folder
                QProcess *proc = new QProcess( this );
                proc->addArgument( "ln" );
                proc->addArgument( "-s" );
                proc->addArgument( files[i] );
                proc->addArgument( cleanSubprojectPath );
                proc->start();
                QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
                // and add them to the filelist
                addFileToCurrentSubProject(titem,filename);

                QString fileNameToAdd = m_shownSubproject->relpath + "/" + filename;
                if (fileNameToAdd.startsWith("/"))
                    fileNameToAdd = fileNameToAdd.mid(1);
                QStringList fileList(fileNameToAdd);
                emit m_part->addedFilesToProject(fileList);
                }
                break;

            case AddFilesDialog::Relative:
                // Form relative path to current subproject folder
                addFileToCurrentSubProject(titem,URLUtil::relativePathToFile(cleanSubprojectPath , files[i]));

                QString fileNameToAdd = URLUtil::canonicalPath(m_shownSubproject->path + "/" + URLUtil::relativePathToFile(cleanSubprojectPath , files[i]));
                fileNameToAdd = fileNameToAdd.mid(m_part->projectDirectory().length() + 1);
                if (fileNameToAdd.startsWith("/"))
                    fileNameToAdd = fileNameToAdd.mid(1);
                QStringList fileList(fileNameToAdd);
                emit m_part->addedFilesToProject(fileList);
                break;
            }
          }
          // Update project file
          if ( titem && titem->owner )
            updateProjectFile(titem->owner);
          // Update subprojectview
          slotOverviewSelectionChanged(m_shownSubproject);
        }
        else if (r == idInsNewFile)
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
                case GroupItem::Translations:
                    fcext = "ts";
                    break;
                case GroupItem::Lexsources:
                    fcext = "l";
                    break;
                case GroupItem::Yaccsources:
                    fcext = "y";
                    break;
                default:
                    fcext = QString::null;
                }
                KDevCreateFile::CreatedFile crFile =
                    createFileSupport->createNewFile(fcext, cleanSubprojectPath);
            } else {
                bool ok = FALSE;
                QString filename = KInputDialog::getText(
                                    i18n( "Create New File"),
                                    i18n( "Enter a name for the new file:" ),
                                    QString::null, &ok, this );
                if ( ok && !filename.isEmpty() )
                {
                    QFile newfile(cleanSubprojectPath+'/'+filename);
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

                    QStringList files(m_shownSubproject->path + "/" + filename);
                    emit m_part->addedFilesToProject(files);
                }
            }
        }
        else if (r == idInsInstallObject)
        {
          bool ok = FALSE;
          QString install_obj = KInputDialog::getText(
                              i18n( "Add Install Object" ),
                              i18n( "Enter a name for the new object:" ),
                              QString::null, &ok, this );
          if ( ok && !install_obj.isEmpty() )
          {
            GroupItem* institem = createGroupItem(GroupItem::InstallObject, install_obj ,titem->scopeString);
            institem->owner = m_shownSubproject;
            institem->install_objectname = install_obj;
            titem->installs.append(institem);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
        }
	else if (r == idLUpdate)
        {
	   QString cmd = "lupdate ";
           cmd += m_shownSubproject->pro_file;
           m_part->appFrontend()->startAppCommand(m_shownSubproject->path,cmd,false);
        }
        else if (r == idLRelease)
        {
	   QString cmd = "lrelease ";
           cmd += m_shownSubproject->pro_file;
           m_part->appFrontend()->startAppCommand(m_shownSubproject->path,cmd,false);
        }
    } else if (pvitem->type() == qProjectItem::File) {

        removefileButton->setEnabled(true);
        FileItem *fitem = static_cast<FileItem*>(pvitem);
	GroupItem::GroupType gtype = static_cast<GroupItem*>(item->parent())->groupType;

	KPopupMenu popup(this);
	if (!(gtype == GroupItem::InstallObject))
        popup.insertTitle(i18n("File: %1").arg(fitem->name));
	else
        popup.insertTitle(i18n("Pattern: %1").arg(fitem->name));

        int idRemoveFile = -2;
	int idSubclassWidget = -2;
	int idUpdateWidgetclass = -2;
	int idUISubclasses = -2;
	int idViewUIH = -2;
	int idFileProperties = -2;
	int idEditInstallPattern = -2;

        if (!fitem->uiFileLink.isEmpty())
        {
          idUpdateWidgetclass = popup.insertItem(SmallIconSet("qmake_subclass"),i18n("Edit ui-Subclass..."));
          popup.setWhatsThis(idUpdateWidgetclass, i18n("<b>Edit ui-subclass</b><p>Launches <b>Subclassing</b> wizard "
                           "and prompts to implement missing in childclass slots and functions."));
        }
        if(fitem->name.contains(".ui"))
        {
	  idSubclassWidget = popup.insertItem(SmallIconSet("qmake_subclass"),i18n("Subclassing Wizard..."));
          popup.setWhatsThis(idSubclassWidget, i18n("<b>Subclass widget</b><p>Launches <b>Subclassing</b> wizard. "
                           "It allows to create a subclass from the class defined in .ui file. "
                           "There is also possibility to implement slots and functions defined in the base class."));
	  idViewUIH = popup.insertItem(SmallIconSet("qmake_ui_h"),i18n("Open ui.h File"));
          popup.setWhatsThis(idViewUIH, i18n("<b>Open ui.h file</b><p>Opens .ui.h file associated with the selected .ui."));
	  idUISubclasses = popup.insertItem(SmallIconSet("qmake_subclass"),i18n("List of Subclasses..."));
          popup.setWhatsThis(idUISubclasses, i18n("<b>List of subclasses</b><p>Shows subclasses list editor. "
                           "There is possibility to add or remove subclasses from the list."));
        }
	if(!(gtype == GroupItem::InstallObject))
	{
	  idRemoveFile = popup.insertItem(SmallIconSet("editdelete"),i18n("Remove File"));
            popup.setWhatsThis(idRemoveFile, i18n("<b>Remove file</b><p>Removes file from a current group. Does not remove file from disk."));
	  idFileProperties = popup.insertItem(SmallIconSet("configure_file"),i18n("Properties..."));
            popup.setWhatsThis(idFileProperties, i18n("<b>Properties</b><p>Opens <b>File Properties</b> dialog that allows to exclude file from specified scopes."));
	}
	else
	{
	  idEditInstallPattern = popup.insertItem(SmallIconSet("configure_file"),i18n("Edit Pattern"));
            popup.setWhatsThis(idEditInstallPattern, i18n("<b>Edit pattern</b><p>Allows to edit install files pattern."));
	  idRemoveFile = popup.insertItem(SmallIconSet("editdelete"),i18n("Remove Pattern"));
            popup.setWhatsThis(idRemoveFile, i18n("<b>Remove pattern</b><p>Removes install files pattern from the current install object."));
	}
	if(!(gtype == GroupItem::InstallObject))
	{
        FileContext context(m_shownSubproject->path + "/" + fitem->name, false);
        m_part->core()->fillContextMenu(&popup, &context);
	}

        int r = popup.exec(p);
        if (r == idRemoveFile)
            removeFile(m_shownSubproject, fitem);
        // Fileproperties
        else if (r == idFileProperties)
        {
        /*
          GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());
          if (!gitem)
            return;
          QStringList dirtyScopes;
          FilePropertyDlg *propdlg = new FilePropertyDlg(m_shownSubproject,gitem->groupType,fitem,dirtyScopes);
          SubqmakeprojectItem *scope;
          propdlg->exec();
          for (uint i=0; i<dirtyScopes.count();i++)
          {
            scope = getScope(m_shownSubproject,dirtyScopes[i]);
            if (scope)
               updateProjectFile(scope);
          }
        */
          slotConfigureFile();
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
                for (uint i=0; i<newFileNames.count(); ++i)
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
        else if (r == idEditInstallPattern)
        {
	GroupItem *titem = static_cast<GroupItem*>(item->parent());

          bool ok = FALSE;
          QString filepattern = KInputDialog::getText(
                              i18n( "Edit Pattern" ),
                              i18n( "Enter a pattern relative to the current "
                                    "subproject (example docs/*.html):" ),
                              fitem->name , &ok, this );
          if ( ok && !filepattern.isEmpty() )
          {
	    removeFile(m_shownSubproject, fitem);
            addFileToCurrentSubProject(titem,filepattern);
            updateProjectFile(titem->owner);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
        }
    }
}


void TrollProjectWidget::removeFile(SubqmakeprojectItem *spitem, FileItem *fitem)
{
    GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());

    if(gitem->groupType != GroupItem::InstallObject)
    {
        QString removedFileName = spitem->relpath + "/" + fitem->text(0);
        if (removedFileName.startsWith("/"))
            removedFileName = removedFileName.mid(1);
        emitRemovedFile(removedFileName);
    }


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
      case GroupItem::Lexsources:
        spitem->lexsources.remove(fitem->text(0));
        break;
      case GroupItem::Yaccsources:
        spitem->yaccsources.remove(fitem->text(0));
        break;
      case GroupItem::Images:
        spitem->images.remove(fitem->text(0));
        break;
      case GroupItem::Distfiles:
        spitem->distfiles.remove(fitem->text(0));
        break;
      case GroupItem::Translations:
        spitem->translations.remove(fitem->text(0));
        break;
      case GroupItem::IDLs:
        spitem->idls.remove(fitem->text(0));
        break;
      case GroupItem::InstallObject:
	gitem->str_files.remove(fitem->text(0));
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

void TrollProjectWidget::parseScope(SubqmakeprojectItem *item, QString scopeString, FileBuffer *buffer)
{
    if (!scopeString.isEmpty())
    {
      QStringList scopeNames = QStringList::split(':',scopeString);
      SubqmakeprojectItem *sitem;
      sitem = new SubqmakeprojectItem(item, scopeNames[scopeNames.count()-1],scopeString);
      sitem->path = item->path;
      sitem->m_RootBuffer = buffer;
      sitem->subdir = item->subdir;
      sitem->pro_file = item->pro_file;
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
    subBuffer->getValues("LEXSOURCES",item->lexsources,item->lexsources_exclude);
    subBuffer->getValues("YACCSOURCES",item->yaccsources,item->yaccsources_exclude);
    subBuffer->getValues("IMAGES",item->images,item->images_exclude);
    subBuffer->getValues("DISTFILES",item->distfiles,item->distfiles_exclude);
    subBuffer->getValues("TRANSLATIONS",item->translations,item->translations_exclude);
    subBuffer->getValues("IDLS",item->idls,item->idls_exclude);
    QStringList installs,installs_exclude;
    subBuffer->getValues("INSTALLS",installs,installs_exclude);

    // Create list view items

    GroupItem * titem = createGroupItem(GroupItem::InstallRoot, "INSTALLS",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!installs.isEmpty())
    {
      QStringList::iterator it = installs.begin();
      for (;it!=installs.end();it++)
      {
        if ((*it)=="target")
          continue;
        QStringList path,path_excl;
        QString path_str;
        subBuffer->getValues((*it)+".path",path,path_excl);
        if (!path.isEmpty())
          path_str = path[0];

        GroupItem* institem = createGroupItem(GroupItem::InstallObject, (*it)  ,scopeString);
        subBuffer->getValues((*it)+".files",institem->str_files,institem->str_files_exclude);
        institem->install_path = path_str;
        institem->install_objectname = *it;
        institem->owner = item;
        titem->installs.append(institem);

        if (!institem->str_files.isEmpty())
        {
          QStringList::iterator it2 = institem->str_files.begin();
          for (;it2!=institem->str_files.end();it2++)
          {
            FileItem *fitem = createFileItem(*it2);
            institem->files.append(fitem);
          }
        }


      }
    }


    titem = createGroupItem(GroupItem::Lexsources, "LEXSOURCES",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->lexsources.isEmpty()) {
        QStringList l = item->lexsources;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Yaccsources, "YACCSOURCES",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->yaccsources.isEmpty()) {
        QStringList l = item->yaccsources;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Distfiles, "DISTFILES",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->distfiles.isEmpty()) {
        QStringList l = item->distfiles;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            fitem->uiFileLink = getUiFileLink(item->relpath+"/",*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Images, "IMAGES",scopeString);
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
    titem = createGroupItem(GroupItem::Translations, "TRANSLATIONS",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->translations.isEmpty()) {
        QStringList l = item->translations;
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

void TrollProjectWidget::parse(SubqmakeprojectItem *item)
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
	  break;
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
      if (lst.find("ordered")!=lst.end())
        item->configuration.m_requirements += QD_ORDERED;
    }
    item->m_FileBuffer.getValues("DESTDIR",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_destdir = lst[0];
    item->m_FileBuffer.getValues("INCLUDEPATH",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_incadd = lst;
    item->m_FileBuffer.getValues("LIBS",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_libadd = lst;

    item->m_FileBuffer.getValues("TARGET",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_target = lst[0];

    item->m_FileBuffer.getValues("TARGETDEPS",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_prjdeps = lst;
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
    item->m_FileBuffer.getValues("MAKEFILE",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_makefile = lst[0];

    // Install path
    item->m_FileBuffer.getValues("target.path",lst,minusListDummy);
    if (lst.count())
      item->configuration.m_target_install_path = lst[0];
    item->m_FileBuffer.getValues("INSTALLS",lst,minusListDummy);
    if (lst.count())
    {
      QStringList::iterator it = lst.begin();
      for (;it!=lst.end();it++)
      {
        if (*it == "target")
        {
          item->configuration.m_target_install = true;
          break;
        }
      }
    }



    // Handle "subdirs" project
    if (item->configuration.m_template == QTMP_SUBDIRS)
    {
      item->m_FileBuffer.getValues("SUBDIRS",lst,minusListDummy);
      item->subdirs = lst;
      QStringList::Iterator it;
      for (it = lst.begin(); it != lst.end(); ++it)
      {
        SubqmakeprojectItem *newitem = new SubqmakeprojectItem(item, (*it),"");
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
    QString baseName = fi.baseName(true);
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
//    m_part->startMakeCommand(buildDir, target);

    kdDebug(9020) << "searching for the subproject" << endl;
    QPtrList<SubqmakeprojectItem> list = findSubprojectForFile(fi);
    kdDebug(9020) << "searching for the subproject: success" << endl;

    SubqmakeprojectItem *spitem;
    for ( spitem = list.first(); spitem; spitem = list.next() )
    {
        QString buildcmd = constructMakeCommandLine(spitem->configuration.m_makefile);
        QString dircmd = "cd " + KProcess::quote(spitem->path) + " && " ;
        kdDebug(9020) << "builddir " << spitem->path << ", cmd " << dircmd + buildcmd + " " + target << endl;
        m_part->queueCmd(spitem->path, dircmd + buildcmd + " " + target);
    }

//    startMakeCommand(buildDir, target);

}


void TrollProjectWidget::slotExecuteProject()
{
    QString program = m_part->mainProgram();
    if (!program.startsWith("/"))
        program.prepend("./");

    if ( program.isEmpty() ) {
        KMessageBox::sorry(this, i18n("Please specify the executable name in the "
            "project options dialog first."), i18n("No Executable Name Given"));
        return;
    }

    // Build environment variables to prepend to the executable path
    QString runEnvVars = QString::null;
    DomUtil::PairList list =
        DomUtil::readPairListEntry( *(m_part->projectDom()), "/kdevtrollproject/run/envvars", "envvar", "name", "value" );

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        const DomUtil::Pair &pair = (*it);
        if ( (!pair.first.isEmpty()) && (!pair.second.isEmpty()) )
            runEnvVars += pair.first + "=" + pair.second + " ";
    }
    program.prepend(runEnvVars);
    program.append(" " + DomUtil::readEntry( *(m_part->projectDom()), "/kdevtrollproject/run/programargs" ) + " ");

    QString dircmd = "cd "+KProcess::quote(this->projectDirectory()) + " && " ;

    bool inTerminal = DomUtil::readBoolEntry(*(m_part->projectDom()), "/kdevtrollproject/run/terminal");
//    m_part->appFrontend()->startAppCommand(dircmd + program, inTerminal);
//    m_part->execute(this->projectDirectory(), "./"+program );
    m_part->appFrontend()->startAppCommand(this->projectDirectory(), program,inTerminal );
}


void TrollProjectWidget::slotCleanProject()
{
  QString dir = this->  projectDirectory();
  if (!m_rootSubproject)
    return;

  createMakefileIfMissing(dir, m_rootSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString rebuildcmd = constructMakeCommandLine(m_rootSubproject->configuration.m_makefile) + " clean";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
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
  createMakefileIfMissing(dir, m_shownSubproject);

  m_part->mainWindow()->raiseView(m_part->makeFrontend()->widget());
  QString dircmd = "cd "+KProcess::quote(dir) + " && " ;
  QString rebuildcmd = constructMakeCommandLine(m_shownSubproject->configuration.m_makefile) + " clean";
  m_part->queueCmd(dir,dircmd + rebuildcmd);
}

QString TrollProjectWidget::constructMakeCommandLine(const QString makeFileName )
{
    QDomDocument &dom = *(m_part->projectDom());

    QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!makeFileName.isEmpty())
    {
        cmdline += " -f " + makeFileName;
    }
    if (!DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, "/kdevtrollproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline.prepend(m_part->makeEnvironment());

    return cmdline;
}
/*
QString TrollProjectWidget::makeEnvironment()
{
    // Get the make environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*(m_part->projectDom()), "/kdevtrollproject/make/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
#if (KDE_VERSION > 305)
        environstr += KProcess::quote((*it).second);
#else
        environstr += KShellProcess::quote((*it).second);
#endif
        environstr += " ";
    }
    return environstr;
}
*/
void TrollProjectWidget::startMakeCommand( const QString & dir, const QString & target )
{
    m_part->partController()->saveAllFiles();

/*    QFileInfo fi;
    QFileInfo fi2;
    if (m_rootSubproject->configuration.m_makefile.isEmpty())
    {
        fi.setFile(dir + "/Makefile");
        fi2.setFile(dir + "/makefile");
    }
    else
    {
        fi.setFile(m_rootSubproject->configuration.m_makefile);
        fi2.setFile(dir + "/" + m_rootSubproject->configuration.m_makefile);
    }
    if (!fi.exists() && !fi2.exists()) {
        int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
        if (r == KMessageBox::No)
            return;
        m_part->startQMakeCommand(dir);
    }
*/
    QDomDocument &dom = *(m_part->projectDom());

    if (target=="clean")
    {
      QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
      if (cmdline.isEmpty())
          cmdline = MAKE_COMMAND;
      cmdline += " clean";
      QString dircmd = "cd " + KProcess::quote(dir) + " && ";
      cmdline.prepend(m_part->makeEnvironment());
      m_part->makeFrontend()->queueCommand(dir, dircmd + cmdline);
    }

    QString cmdline = constructMakeCommandLine() + " " + target;

    QString dircmd = "cd " + KProcess::quote(dir) + " && ";

    cmdline.prepend(m_part->makeEnvironment());
    m_part->makeFrontend()->queueCommand(dir, dircmd + cmdline);
}

void TrollProjectWidget::createMakefileIfMissing(const QString &dir, SubqmakeprojectItem *item)
{
  QFileInfo fi;
  QFileInfo fi2;
  if (item->configuration.m_makefile.isEmpty())
  {
    fi.setFile(dir + "/Makefile");
    fi2.setFile(dir + "/makefile");
  }
  else
  {
    fi.setFile(item->configuration.m_makefile);
    fi2.setFile(dir + "/" + item->configuration.m_makefile);
  }
  if (!fi.exists() && !fi2.exists()) {
      int r = KMessageBox::questionYesNo(this, i18n("There is no Makefile in this directory. Run qmake first?"));
      if (r == KMessageBox::No)
          return;
      m_part->startQMakeCommand(dir);
  }
}

QPtrList<SubqmakeprojectItem> TrollProjectWidget::findSubprojectForFile( QFileInfo fi )
{
    QPtrList<SubqmakeprojectItem> list;
    findSubprojectForFile(list, m_rootSubproject, fi.absFilePath());
    return list;
}

void TrollProjectWidget::findSubprojectForFile( QPtrList<SubqmakeprojectItem> &list, SubqmakeprojectItem * item, QString absFilePath )
{
    QDir d(item->path);
    kdDebug(9020) << "searching withing subproject: " << item->path << endl;

    for (QStringList::Iterator it = item->sources.begin(); it != item->sources.end(); ++it )
    {
        QFileInfo fi2(d, *it);
        kdDebug(9020) << "subproject item: key: " << absFilePath << " value:" << fi2.absFilePath() << endl;
        if (absFilePath == fi2.absFilePath())
            list.append(item);
    }

    for (QStringList::Iterator it = item->headers.begin(); it != item->headers.end(); ++it )
    {
        QFileInfo fi2(d, *it);
        kdDebug(9020) << "subproject item: key: " << absFilePath << " value:" << fi2.absFilePath() << endl;
        if (absFilePath == fi2.absFilePath())
            list.append(item);
    }

    QListViewItem * child = item->firstChild();
    while( child )
    {
        SubqmakeprojectItem *spitem = dynamic_cast<SubqmakeprojectItem*>(child);

        if (spitem)
        {
            kdDebug(9020) << "next subproject item with profile = " << spitem->pro_file << endl;
            findSubprojectForFile(list, spitem, absFilePath);
        }

        child = child->nextSibling();
    }

/*    QListViewItemIterator it( item );
    while ( it.current() )
    {
        SubqmakeprojectItem *spitem = dynamic_cast<SubqmakeprojectItem*>(it.current());

        if (spitem)
        {
            kdDebug(9020) << "next subproject item with profile = " << spitem->pro_file << endl;
            findSubprojectForFile(list, spitem, absFilePath);
        }

        ++it;
    }*/
}

//check or uncheck dependency to currently checked or unchecked library
void InsideCheckListItem::stateChange( bool state )
{
    if (listView() == m_config->insidelib_listview)
    {
        QListViewItemIterator it( m_config->intDeps_view );
        while ( it.current() ) {
            InsideCheckListItem *chi = dynamic_cast<InsideCheckListItem*>(it.current());
            if (chi)
                if ( chi->prjItem == prjItem )
                    chi->setOn(state);
            ++it;
        }
    }
}

void TrollProjectWidget::slotRemoveScope( SubqmakeprojectItem * spitem )
{
  if (spitem==0 && m_shownSubproject==0)
    return;
  else
  {
    if ( ( spitem = dynamic_cast<SubqmakeprojectItem *>(m_shownSubproject->parent()) ) != NULL  )
    {
    spitem->m_RootBuffer->removeScope(m_shownSubproject->scopeString, m_shownSubproject->scopeString);
    spitem->scopes.remove(m_shownSubproject);
    delete m_shownSubproject;
    m_shownSubproject = spitem;
    updateProjectFile(spitem);
    overview->setCurrentItem(m_shownSubproject);
    overview->setSelected(m_shownSubproject, true);
    }
  }
}

SubqmakeprojectItem * TrollProjectWidget::findSubprojectForScope( SubqmakeprojectItem * scope )
{
    if ((scope == 0) || (scope->parent() == 0))
        return 0;
    if (!scope->isScope)
        return scope;
    return findSubprojectForScope(dynamic_cast<SubqmakeprojectItem *>(scope->parent()));
}

void TrollProjectWidget::focusInEvent( QFocusEvent */*e*/ )
{
    switch (m_lastFocusedView)
    {
        case DetailsView:
            details->setFocus();
            break;
        case SubprojectView:
        default:
            overview->setFocus();
    }
}

void TrollProjectWidget::setLastFocusedView( TrollProjectView view )
{
    m_lastFocusedView = view;
}


#include "trollprojectwidget.moc"
