/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "autoprojectwidget.h"

#include <qcheckbox.h>
#include <qdom.h>
#include <qfile.h>
#include <qheader.h>
#include <qpainter.h>
#include <qptrstack.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>
#include <kfile.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "domutil.h"
#include "misc.h"
#include "subprojectoptionsdlg.h"
#include "targetoptionsdlg.h"
#include "addsubprojectdlg.h"
#include "addtargetdlg.h"
#include "addservicedlg.h"
#include "addapplicationdlg.h"
#include "addfiledlg.h"
#include "addicondlg.h"
#include "addexistingfilesdlg.h"
#include "addexistingdirectoriesdlg.h"
#include "removefiledlg.h"
#include "removetargetdlg.h"
#include "choosetargetdialog.h"
#include "autoprojectpart.h"
#include "urlutil.h"
#include "kdevcreatefile.h"

static QString nicePrimary( const QString &primary )
{
	if ( primary == "PROGRAMS" )
		return i18n( "Program" );
	else if ( primary == "LIBRARIES" )
		return i18n( "Library" );
	else if ( primary == "LTLIBRARIES" )
		return i18n( "Libtool Library" );
	else if ( primary == "SCRIPTS" )
		return i18n( "Script" );
	else if ( primary == "HEADERS" )
		return i18n( "Header" );
	else if ( primary == "DATA" )
		return i18n( "Data" );
	else if ( primary == "JAVA" )
		return i18n( "Java" );
	else
		return QString::null;
}


static QString cleanWhitespace( const QString &str )
{
	QString res;

	QStringList l = QStringList::split( QRegExp( "[ \t]" ), str );
	QStringList::ConstIterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	{
		res += *it;
		res += " ";
	}

	return res.left( res.length() - 1 );
}


/**
* Class ProjectItem
*/

ProjectItem::ProjectItem( Type type, QListView *parent, const QString &text )
		: QListViewItem( parent, text ), typ( type )
{
	bld = false;
}


ProjectItem::ProjectItem( Type type, ProjectItem *parent, const QString &text )
		: QListViewItem( parent, text ), typ( type )
{
	bld = false;
}


void ProjectItem::paintCell( QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment )
{
	if ( isBold() )
	{
		QFont font( p->font() );
		font.setBold( true );
		p->setFont( font );
	}
	QListViewItem::paintCell( p, cg, column, width, alignment );
}


/**
* Class SubprojectItem
*/

SubprojectItem::SubprojectItem( QListView *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


SubprojectItem::SubprojectItem( SubprojectItem *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


void SubprojectItem::init()
{
	targets.setAutoDelete( true );
	setPixmap( 0, SmallIcon( "folder" ) );
}


/**
* Class TargetItem
*/

TargetItem::TargetItem( QListView *lv, bool group, const QString &text )
		: ProjectItem( Target, lv, text )
{
	sources.setAutoDelete( true );
	setPixmap( 0, group ? SmallIcon( "tar" ) : SmallIcon( "binary" ) );
}


/**
* Class FileItem
*/

FileItem::FileItem( QListView *lv, const QString &text )
		: ProjectItem( File, lv, text )
{
	setPixmap( 0, SmallIcon( "document" ) );
}


AutoProjectWidget::AutoProjectWidget( AutoProjectPart *part, bool kde )
		: QVBox( 0, "auto project widget" )
{
	QSplitter *splitter = new QSplitter(Vertical, this);

	initOverview ( splitter );
	initDetailview ( splitter );

	initActions ();

	m_part = part;
	m_kdeMode = kde;
	m_activeSubproject = 0;
	m_activeTarget = 0;
	m_shownSubproject = 0;
	m_choosenTarget = 0;
}


AutoProjectWidget::~AutoProjectWidget()
{}

void AutoProjectWidget::initOverview ( QWidget* parent )
{
	QVBox * overviewBox = new QVBox( parent, "vertical overview box" );

	QHBox *overviewButtonBox = new QHBox( overviewBox, "subproject button box" );
	overviewButtonBox->setMargin( 2 );
	overviewButtonBox->setSpacing( 2 );

	addSubprojectButton = new QToolButton( overviewButtonBox );
	addSubprojectButton->setPixmap( SmallIcon( "folder_new" ) );
	QToolTip::add
		( addSubprojectButton, i18n( "Add New Subproject" ) );

	/*    addExistingSubprojectButton = new QToolButton(overviewButtonBox);
		addExistingSubprojectButton->setPixmap(SmallIcon("fileimport"));
		QToolTip::add(addExistingSubprojectButton, i18n("Add existing subproject to currently selected subproject..."));
	*/
	addTargetButton = new QToolButton( overviewButtonBox );
	addTargetButton->setPixmap( SmallIcon( "targetnew_kdevelop" ) );
	QToolTip::add
		( addTargetButton, i18n( "Add New Target" ) );

	addServiceButton = new QToolButton( overviewButtonBox );
	addServiceButton->setPixmap( SmallIcon( "servicenew_kdevelop" ) );
	QToolTip::add
		( addServiceButton, i18n( "Add New Service" ) );

	addApplicationButton = new QToolButton( overviewButtonBox );
	addApplicationButton->setPixmap( SmallIcon( "window_new" ) );
	QToolTip::add
		( addApplicationButton, i18n( "Add New Application" ) );

	buildSubprojectButton = new QToolButton( overviewButtonBox );
	buildSubprojectButton->setPixmap( SmallIcon( "launch" ) );
	QToolTip::add
		( buildSubprojectButton, i18n( "Build" ) );

	QWidget *spacer1 = new QWidget( overviewButtonBox );
	overviewButtonBox->setStretchFactor( spacer1, 1 );

	subProjectOptionsButton = new QToolButton( overviewButtonBox );
	subProjectOptionsButton->setPixmap( SmallIcon( "configure" ) );
	QToolTip::add
		( subProjectOptionsButton, i18n( "Show Options" ) );

	overviewButtonBox->setMaximumHeight( subProjectOptionsButton->height() );

	overview = new KListView( overviewBox, "project overview widget" );

	overview->setResizeMode( QListView::LastColumn );
	overview->setSorting( -1 );
	overview->header() ->hide();
	overview->addColumn( QString::null );
}

void AutoProjectWidget::initDetailview ( QWidget* parent )
{
	QVBox * targetBox = new QVBox( parent, "vertical target box" );

	QHBox *targetButtonBox = new QHBox( targetBox, "target button box" );
	targetButtonBox->setMargin( 2 );
	targetButtonBox->setSpacing( 2 );

	addNewFileButton = new QToolButton( targetButtonBox );
	addNewFileButton->setPixmap( SmallIcon( "filenew" ) );
	QToolTip::add
		( addNewFileButton, i18n( "Create New File" ) );

	addExistingFileButton = new QToolButton( targetButtonBox );
	addExistingFileButton->setPixmap( SmallIcon( "fileimport" ) );
	QToolTip::add
		( addExistingFileButton, i18n( "Add Existing Files" ) );

	removeButton = new QToolButton( targetButtonBox );
	removeButton->setPixmap( SmallIcon( "editdelete" ) );
	QToolTip::add
		( removeButton, i18n( "Remove" ) );

	buildTargetButton = new QToolButton( targetButtonBox );
	buildTargetButton->setPixmap( SmallIcon( "launch" ) );
	QToolTip::add
		( buildTargetButton, i18n( "Build" ) );

	QWidget *spacer2 = new QWidget( targetButtonBox );
	targetButtonBox->setStretchFactor( spacer2, 1 );

	targetOptionsButton = new QToolButton( targetButtonBox );
	targetOptionsButton->setPixmap( SmallIcon( "configure" ) );
	QToolTip::add
		( targetOptionsButton, i18n( "Show Options" ) );

	targetButtonBox->setMaximumHeight( addNewFileButton->height() );

	targetOptionsButton->setEnabled( false );
	addNewFileButton->setEnabled( false );
	addExistingFileButton->setEnabled( false );
	removeButton->setEnabled( true );
	buildTargetButton->setEnabled( true );

	details = new KListView( targetBox, "project details widget" );
	details->setRootIsDecorated( true );
	details->setResizeMode( QListView::LastColumn );
	details->setSorting( -1 );
	details->header() ->hide();
	details->addColumn( QString::null );
}

void AutoProjectWidget::initActions()
{
	KActionCollection * actions = new KActionCollection( this );

	subProjectOptionsAction = new KAction( i18n( "Options..." ), "configure", 0,
	                                       this, SLOT( slotSubprojectOptions() ), actions, "subproject options" );
	addSubprojectAction = new KAction( i18n( "Add Subproject..." ), "folder_new", 0,
	                                   this, SLOT( slotAddSubproject() ), actions, "add subproject" );
	addExistingSubprojectAction = new KAction( i18n( "Add Existing Subprojects..." ), "fileimport", 0,
	                                           this, SLOT( slotAddExistingSubproject() ), actions, "add existing subproject" );
	addTargetAction = new KAction( i18n( "Add Target..." ), "targetnew_kdevelop", 0,
	                               this, SLOT( slotAddTarget() ), actions, "add target" );
	addServiceAction = new KAction( i18n( "Add Service..." ), "servicenew_kdevelop", 0,
	                                this, SLOT( slotAddService() ), actions, "add service" );
	addApplicationAction = new KAction( i18n( "Add Application..." ), "window_new", 0,
	                                    this, SLOT( slotAddApplication() ), actions, "add application" );
	buildSubprojectAction = new KAction( i18n( "Build" ), "launch", 0,
	                                     this, SLOT( slotBuildSubproject() ), actions, "add build subproject" );

	targetOptionsAction = new KAction( i18n( "Options..." ), "configure", 0,
	                                   this, SLOT( slotTargetOptions() ), actions, "target options" );
	addNewFileAction = new KAction( i18n( "Create New File..." ), "filenew", 0,
	                                this, SLOT( slotAddNewFile() ), actions, "add new file" );
	addExistingFileAction = new KAction( i18n( "Add Existing Files..." ), "fileimport", 0,
	                                     this, SLOT( slotAddExistingFile() ), actions, "add existing file" );
	addIconAction = new KAction( i18n( "Add Icon..." ), "iconadd_kdevelop", 0,
	                             this, SLOT( slotAddIcon() ), actions, "add icon" );
	buildTargetAction = new KAction( i18n( "Build Target..." ), "launch", 0,
	                                 this, SLOT( slotBuildTarget() ), actions, "build target" );
	setActiveTargetAction = new KAction( i18n( "Make Target Active" ), "", 0,
	                                     this, SLOT( slotSetActiveTarget() ), actions, "set active target" );
	removeDetailAction = new KAction( i18n( "Remove" ), "editdelete", 0,
	                                  this, SLOT( slotRemoveDetail() ), actions, "remove detail" );

	removeSubprojectAction = new KAction ( i18n ( "Remove" ), "editdelete", 0,
	                                       this, SLOT ( slotRemoveSubproject() ), actions, "remove subproject" );

	connect( subProjectOptionsButton, SIGNAL( clicked() ), this, SLOT( slotSubprojectOptions() ) );
	connect( addSubprojectButton, SIGNAL( clicked() ), this, SLOT( slotAddSubproject() ) );
	//    connect( addExistingSubprojectButton, SIGNAL(clicked()), this, SLOT(slotAddExistingSubproject()) );
	connect( addTargetButton, SIGNAL( clicked() ), this, SLOT( slotAddTarget() ) );
	connect( addServiceButton, SIGNAL( clicked() ), this, SLOT( slotAddService() ) );
	connect( addApplicationButton, SIGNAL( clicked() ), this, SLOT( slotAddApplication() ) );
	connect( buildSubprojectButton, SIGNAL( clicked() ), this, SLOT( slotBuildSubproject() ) );

	connect( targetOptionsButton, SIGNAL( clicked() ), this, SLOT( slotTargetOptions() ) );
	connect( addNewFileButton, SIGNAL( clicked() ), this, SLOT( slotAddNewFile() ) );
	connect( addExistingFileButton, SIGNAL( clicked() ), this, SLOT( slotAddExistingFile() ) );
	connect( removeButton , SIGNAL( clicked() ), this, SLOT( slotRemoveDetail() ) );
	connect( buildTargetButton, SIGNAL( clicked() ), this, SLOT( slotBuildTarget() ) );

	connect( overview, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotOverviewSelectionChanged( QListViewItem* ) ) );
	connect( overview, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( slotOverviewContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );

	connect( details, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotDetailsSelectionChanged( QListViewItem* ) ) );
	connect( details, SIGNAL( executed( QListViewItem* ) ),
	         this, SLOT( slotDetailsExecuted( QListViewItem* ) ) );
	connect ( details, SIGNAL ( returnPressed ( QListViewItem* ) ),
	          this, SLOT ( slotDetailsExecuted ( QListViewItem* ) ) );
	connect( details, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( slotDetailsContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );

	// 	connect ( overview, SIGNAL ( selectionChanged ( QListViewItem* ) ),
	// 				chooseTargetDlg, SLOT ( slotOverviewSelectionChanged ( QListViewItem* ) ) );
	// 	connect ( details, SIGNAL ( selectionChanged ( QListViewItem* ) ),
	// 				chooseTargetDlg, SLOT ( slotDetailsSelectionChanged ( QListViewItem* ) ) );

}

void AutoProjectWidget::openProject( const QString &dirName )
{
	SubprojectItem * item = new SubprojectItem( overview, "/" );
	item->setPixmap ( 0, SmallIcon ( "kdevelop" ) );
	item->subdir = "/";
	item->path = dirName;
	parse( item );
	item->setOpen( true );
	item->setText ( 0, m_part->projectDirectory() + m_part->projectName() );
	overview->setSelected( item, true );
}


void AutoProjectWidget::closeProject()
{
	overview->clear();
	details->clear();
}

SubprojectItem* AutoProjectWidget::activeSubproject ()
{
	return m_activeSubproject;
}

TargetItem* AutoProjectWidget::activeTarget ()
{
	return m_activeTarget;
}

QStringList AutoProjectWidget::allSubprojects()
{
	int prefixlen = projectDirectory().length() + 1;
	QStringList res;

	QListViewItemIterator it( overview );
	for ( ; it.current(); ++it )
	{
		// Skip root subproject
		if ( it.current() == overview->firstChild() )
			continue;
		QString path = static_cast<SubprojectItem*>( it.current() ) ->path;
		res.append( path.mid( prefixlen ) );
	}

	return res;
}

QPtrList <SubprojectItem> AutoProjectWidget::allSubprojectItems()
{
	QPtrList <SubprojectItem> res;

	QListViewItemIterator it ( overview );

	for ( ; it.current(); ++it )
	{
		// Skip root subproject
		if ( it.current() == overview->firstChild() )
			continue;

		SubprojectItem* spitem = static_cast <SubprojectItem*> ( it.current() );

		res.append ( spitem );
	}

	return res;
}

SubprojectItem* AutoProjectWidget::subprojectItemForPath(const QString & path, bool pathIsAbsolute) const
{       kdDebug(9020) << "Looking for path " << path << endl;

        int prefixLen = projectDirectory().length() + 1;
        for(QListViewItemIterator it = overview;it.current();++it)
        {
                SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
                QString relpath = (spitem->path).mid(prefixLen);
                relpath = (relpath==QString::null) ? QString("") : relpath;
                kdDebug(9020) << " ... checking -" << spitem->path << "-" << endl;
                kdDebug(9020) << " ... (tailored: -" << relpath << "- against -" << (pathIsAbsolute ? path.mid(prefixLen) : path) << "- )" << endl;
                if ( relpath == (pathIsAbsolute ? path.mid(prefixLen) : path))
                {
                        kdDebug(9020) << "Found it!" << endl;
                        return spitem;
                }
        }
        kdDebug(9020) << "Not found" << endl;
        return NULL;
}

QString AutoProjectWidget::pathForTarget(const TargetItem *titem) const {

  if (!titem) return QString::null;

  kdDebug(9020) << "Looking for target " << titem->name << endl;
  int prefixLen = projectDirectory().length() + 1;
  for(QListViewItemIterator it = overview;it.current();++it)
  {
    SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
    kdDebug(9020) << "Checking: " << spitem->path << endl;
    if (spitem->targets.containsRef(titem)) {
      kdDebug(9020) << "Found it!" << endl;
      QString relpath = (spitem->path).mid(prefixLen);
      return (relpath==QString::null) ? QString("") : relpath;
    }
  }
  kdDebug(9020) << "Not found" << endl;
  return QString::null;
}

QStringList AutoProjectWidget::allLibraries()
{
	int prefixlen = projectDirectory().length() + 1;
	QStringList res;

	QListViewItemIterator it( overview );
	for ( ; it.current(); ++it )
	{
		SubprojectItem *spitem = static_cast<SubprojectItem*>( it.current() );
		QString path = spitem->path;
		QListIterator<TargetItem> tit( spitem->targets );
		for ( ; tit.current(); ++tit )
		{
			QString primary = ( *tit ) ->primary;
			if ( primary == "LIBRARIES" || primary == "LTLIBRARIES" )
			{
				QString fullname = path + "/" + ( *tit ) ->name;
				res.append( fullname.mid( prefixlen ) );
			}
		}
	}

	return res;
}


QStringList AutoProjectWidget::allFiles()
{
	QStack<QListViewItem> s;
	QStringList list;

	for ( QListViewItem * item = overview->firstChild(); item;
	        item = item->nextSibling() ? item->nextSibling() : s.pop() )
	{
		if ( item->firstChild() )
			s.push( item->firstChild() );

		SubprojectItem *spitem = static_cast<SubprojectItem*>( item );
                // use URLUtil so paths in root project dir are worked out correctly
                QString relPath = URLUtil::relativePath(projectDirectory(), spitem->path, URLUtil::SLASH_SUFFIX);
		QListIterator<TargetItem> tit( spitem->targets );
		for ( ; tit.current(); ++tit )
		{
			QListIterator<FileItem> fit( tit.current() ->sources );
			for ( ; fit.current(); ++fit )
				list.append( relPath + ( *fit ) ->name );
		}
	}

	// Files may be in multiple targets, so we have to remove
	// duplicates
	QStringList res;

	QStringList::ConstIterator it;
	for ( it = list.begin(); it != list.end(); ++it )
		if ( !res.contains( *it ) )
                { res.append( *it ); kdDebug(9020) << "***INCLUDING " << (*it) << " in allFiles()" << endl; }

	return res;
}


QString AutoProjectWidget::projectDirectory() const
{
	if ( !overview->firstChild() )
		return QString::null; //confused

	return static_cast<SubprojectItem*>( overview->firstChild() ) ->path;
}


QString AutoProjectWidget::subprojectDirectory()
{
	if ( !selectedSubproject() )
		return QString::null;

	return selectedSubproject() ->path;
}


void AutoProjectWidget::setActiveTarget( const QString &targetPath )
{
	int prefixlen = projectDirectory().length() + 1;

	m_activeSubproject = 0;
	m_activeTarget = 0;

	QListViewItemIterator it( overview );
	for ( ; it.current(); ++it )
	{
		SubprojectItem *spitem = static_cast<SubprojectItem*>( it.current() );
		QString path = spitem->path;
		QListIterator<TargetItem> tit( spitem->targets );
		for ( ; tit.current(); ++tit )
		{
			QString primary = ( *tit ) ->primary;
			if ( primary != "PROGRAMS" && primary != "LIBRARIES"
			        && primary != "LTLIBRARIES" && primary != "JAVA" )
				continue;

			QString currentTargetPath = ( path + "/" + ( *tit ) ->name ).mid( prefixlen );

			bool hasTarget = ( targetPath == currentTargetPath );
			( *tit )->setBold( hasTarget );
			if ( hasTarget )
			{
				spitem->setBold( true );
				m_activeSubproject = spitem;
				m_activeTarget = ( *tit );
				overview->setSelected( m_activeSubproject, true );
				overview->ensureItemVisible ( m_activeSubproject );
				overview->viewport() ->update();
				details->setSelected ( m_activeTarget, true );
				details->ensureItemVisible ( m_activeTarget );
				details->viewport()->update();
			}
			else
			{
				spitem->setBold ( ( m_activeSubproject == spitem ) );
				details->viewport()->update();
			}
		}
	}
}


QString AutoProjectWidget::activeDirectory()
{
	if ( m_activeSubproject )
		return m_activeSubproject->path.mid( projectDirectory().length() + 1 );
	else
	{
/*		if ( selectedSubproject() )
			return selectedSubproject()->path;
		else*/
			return QString::null;
	}
}


void AutoProjectWidget::addFiles( const QStringList &list )
{
	QDomDocument &dom = *m_part->projectDom();
	QStringList fileList = list;

	if ( DomUtil::readBoolEntry( dom, "/kdevautoproject/general/useactivetarget" ) )
	{
		QStringList::iterator it;

		QString fileName;

		for ( it = fileList.begin(); it != fileList.end(); ++it )
		{
			int pos = ( *it ).findRev('/');
			if (pos != -1)
			{
				fileName = ( *it ).mid(pos+1);
			}
			else
			{
				fileName = ( *it );
			}

                        //FileItem * fitem = createFileItem( fileName );
			//m_activeTarget->sources.append( fitem );
			//m_activeTarget->insertItem( fitem );

			// TODO: Merge with code in addfiledlg.cpp
                        addToTarget(fileName, m_activeSubproject, m_activeTarget);
//			QString canontargetname = AutoProjectTool::canonicalize( m_activeTarget->name );
//			QString varname = canontargetname + "_SOURCES";
//			m_activeSubproject->variables[ varname ] += ( " " + fileName );
//
//			QMap<QString, QString> replaceMap;
//			replaceMap.insert( varname, m_activeSubproject->variables[ varname ] );
//
//			AutoProjectTool::modifyMakefileam( m_activeSubproject->path + "/Makefile.am", replaceMap );
		}

		emitAddedFiles ( list );
	}
	else
	{
                // See if we can figure out the target for each file without asking the user
                // I think it's a valid assumption that if a directory contains only one target
                // the file can be added to that target (Julian Rockey linux at jrockey.com)
                QStringList doManually, doneAutomatically;
                for(QStringList::iterator it = fileList.begin();it!=fileList.end();++it)
                {
                        bool autoAdded = false;
                        QString relativeDir = URLUtil::directory(*it);
                        SubprojectItem* spitem = subprojectItemForPath(relativeDir);
                        if (spitem)
                        {
                                QPtrList<TargetItem> titemList = spitem->targets;
                                if (titemList.count()==1) {
                                        addToTarget( URLUtil::filename(*it), spitem, titemList.first() );
                                        doneAutomatically.append(*it);
                                        autoAdded = true;
                                }
                        }
                        // add to manual list if this file wasn't auto-added
                        if (!autoAdded) doManually.append(*it);
                }
                if (doneAutomatically.count()>0) emitAddedFiles(doneAutomatically);

                // raise dialog for any files that weren't added automatically
                if (doManually.count()>0) {
		        ChooseTargetDialog chooseTargetDlg ( this, doManually, this, "choose target dialog" );

		        //chooseTargetDlg = new ChooseTargetDialog ( this, this, "choose target dialog" );

		        if ( chooseTargetDlg.exec() && chooseTargetDlg.neverAskAgainCheckBox->isChecked() )
		        {
		        	DomUtil::writeBoolEntry( dom, "/kdevautoproject/general/useactivetarget", true );
		        }
                }
	}
}

void AutoProjectWidget::addToTarget(const QString & fileName, SubprojectItem* spitem, TargetItem* titem)
{
        FileItem * fitem = createFileItem( fileName );
        titem->sources.append( fitem );
        titem->insertItem( fitem );

	QString canontargetname = AutoProjectTool::canonicalize( titem->name );
	QString varname = canontargetname + "_SOURCES";
        spitem->variables[ varname ] += ( " " + fileName );

	QMap<QString, QString> replaceMap;
	replaceMap.insert( varname, spitem->variables[ varname ] );

        AutoProjectTool::modifyMakefileam( spitem->path + "/Makefile.am", replaceMap );

        slotDetailsSelectionChanged(spitem);
}

void AutoProjectWidget::removeFiles( const QStringList &list )
{
	( void ) list;
}


void AutoProjectWidget::slotOverviewSelectionChanged( QListViewItem *item )
{
	if ( !item )
		return ;

	if ( m_shownSubproject )
	{
		// Remove all TargetItems and all of their children from the view
		QListIterator<TargetItem> it1( m_shownSubproject->targets );
		for ( ; it1.current(); ++it1 )
		{
			// After AddTargetDialog, it can happen that an
			// item is not yet in the list view, so better check...
			if ( it1.current() ->parent() )
				while ( ( *it1 ) ->firstChild() )
					( *it1 ) ->takeItem( ( *it1 ) ->firstChild() );
			details->takeItem( *it1 );
		}
	}

	// We assume here that ALL items in the over list view
	// are SubprojectItem's
	m_shownSubproject = static_cast<SubprojectItem*>( item );


	// Insert all TargetItems and all of their children into the view
	QListIterator<TargetItem> it2( m_shownSubproject->targets );
	for ( ; it2.current(); ++it2 )
	{
		details->insertItem( *it2 );
		QListIterator<FileItem> it3( ( *it2 ) ->sources );
		for ( ; it3.current(); ++it3 )
			( *it2 ) ->insertItem( *it3 );
		QString primary = ( *it2 ) ->primary;
		if ( primary == "PROGRAMS" || primary == "LIBRARIES"
		        || primary == "LTLIBRARIES" || primary == "JAVA" )
			( *it2 ) ->setOpen( true );
	}

	targetOptionsButton->setEnabled ( false );
	addNewFileButton->setEnabled ( false );
	addExistingFileButton->setEnabled ( false );
	removeButton->setEnabled ( false );
	buildTargetButton->setEnabled ( false );
}


void AutoProjectWidget::slotDetailsSelectionChanged( QListViewItem *item )
{
	bool isTarget = false;
	bool isRegularTarget = false;
	bool isFile = false;

	if ( item )
	{
		// We assume here that ALL items in the detail list view
		// are ProjectItem's
		ProjectItem * pvitem = static_cast<ProjectItem*>( item );
		TargetItem* titem = 0;

		if ( pvitem->type() == ProjectItem::File )
		{
			titem = static_cast <TargetItem*> ( pvitem->parent() );

			QString primary = titem->primary;
			if ( primary == "PROGRAMS" || primary == "LIBRARIES"
			        || primary == "LTLIBRARIES" || primary == "JAVA" )
			{
				isRegularTarget = true; // not a data group
				isFile = true;
			}
		}
		else
		{
			titem = static_cast <TargetItem*> ( pvitem );
			isTarget = true;
		}

		QString primary = titem->primary;
		if ( primary == "PROGRAMS" || primary == "LIBRARIES"
		        || primary == "LTLIBRARIES" || primary == "JAVA" )
			isRegularTarget = true; // not a data group

		/*		if ( isRegularTarget && pvitem->type() == ProjectItem::File )
				{
					isRegularFile = true;
				}*/
	}

	targetOptionsButton->setEnabled( isRegularTarget && !isFile );
	addNewFileButton->setEnabled( isTarget );
	addExistingFileButton->setEnabled( isTarget );
	removeButton->setEnabled ( true );

	if ( isRegularTarget && isFile || isRegularTarget )
		buildTargetButton->setEnabled ( true );
	else
		buildTargetButton->setEnabled ( false );
}


void AutoProjectWidget::slotDetailsExecuted( QListViewItem *item )
{
	if ( !item )
		return ;

	ProjectItem *pvitem = static_cast<ProjectItem*>( item );
	if ( pvitem->type() != ProjectItem::File )
		return ;

	QString dirName = m_shownSubproject->path;
	FileItem *fitem = static_cast<FileItem*>( item );
	m_part->partController() ->editDocument( KURL( dirName + "/" + fitem->name ) );
	m_part->topLevel() ->lowerView( this );
}


void AutoProjectWidget::slotSubprojectOptions()
{
	kdDebug( 9020 ) << "AutoProjectWidget::slotSubprojectOptions()" << endl;

	SubprojectOptionsDialog dlg( m_part, this, m_shownSubproject,
	                             this, "subproject options dialog" );
	dlg.exec();
}


void AutoProjectWidget::slotAddSubproject()
{
	AddSubprojectDialog dlg( m_part, this, m_shownSubproject,
	                         this, "add subproject dialog" );

	dlg.setCaption ( i18n ( "Add New Subproject to '%1'" ).arg ( selectedSubproject()->subdir ) );
	dlg.exec();
}


void AutoProjectWidget::slotAddExistingSubproject()
{
	SubprojectItem* spitem = selectedSubproject();
	if ( !spitem )
		return;

	AddExistingDirectoriesDialog dlg ( m_part, this, spitem, this, "add existing subprojects" );

	dlg.setCaption ( i18n ( "Add Existing Subproject to '%1'" ).arg ( spitem->subdir ) );

	if ( dlg.exec() )
		slotOverviewSelectionChanged ( spitem );
}


void AutoProjectWidget::slotAddExistingFile()
{
	TargetItem * titem = selectedTarget();
	if ( !titem )
		return;

	AddExistingFilesDialog dlg( m_part, this, m_shownSubproject, titem,
	                          this, "add existing files" );
	QString caption;
	if ( selectedTarget()->name.isEmpty() )
		caption = i18n ( "%1 in %2" ).arg ( selectedTarget()->primary ).arg ( selectedTarget()->prefix );
	else
		caption = selectedTarget()->name;

	dlg.setCaption( i18n( "Add Existing Files to '%1'" ).arg ( caption ) );

	dlg.exec();
}


void AutoProjectWidget::slotAddTarget()
{
	AddTargetDialog dlg( this, m_shownSubproject, this, "add target dialog" );

	dlg.setCaption ( i18n ( "Add New Target to '%1'" ).arg ( selectedSubproject()->subdir ) );

	// Update the details view if a target was added
	if ( dlg.exec() )
		slotOverviewSelectionChanged( m_shownSubproject );
}


void AutoProjectWidget::slotAddService()
{
	AddServiceDialog dlg( this, m_shownSubproject, this, "add service dialog" );

	dlg.setCaption ( i18n ( "Add New Service to '%1'" ).arg ( selectedSubproject()->subdir ) );

	// Update the details view if a service was added
	if ( dlg.exec() )
		slotOverviewSelectionChanged( m_shownSubproject );
}


void AutoProjectWidget::slotAddApplication()
{
	AddApplicationDialog dlg( this, m_shownSubproject, this, "add application dialog" );

	dlg.setCaption ( i18n ( "Add New Application to '%1'" ).arg ( selectedSubproject()->subdir ) );

	// Update the details view if an application was added
	if ( dlg.exec() )
		slotOverviewSelectionChanged( m_shownSubproject );
}


void AutoProjectWidget::slotBuildSubproject()
{
	QString relpath = m_shownSubproject->path.mid( projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, QString::fromLatin1( "" ) );

	m_part->topLevel() ->lowerView( this );
}


void AutoProjectWidget::slotTargetOptions()
{
	kdDebug( 9020 ) << "AutoProjectWidget::slotTargetOptions()" << endl;
	TargetItem *titem = selectedTarget();
	if ( !titem )
		return ;

	TargetOptionsDialog dlg( this, titem, this, "target options dialog" );

	dlg.setCaption ( i18n ( "Target Options for '%1'" ).arg ( selectedTarget()->name ) );

	dlg.exec();
}


void AutoProjectWidget::slotAddNewFile()
{
	TargetItem * titem = selectedTarget();
	if ( !titem )
          return ;

        KDevCreateFile * createFileSupport = m_part->createFileSupport();
        if (createFileSupport) {
          KDevCreateFile::CreatedFile crFile =
            createFileSupport->createNewFile(QString::null, pathForTarget(titem) );


        } else {
          // No create-file plugin available; use our own code

          AddFileDialog dlg( m_part, this, m_shownSubproject, titem,
                             this, "add file dialog" );

          QString caption;
          if ( selectedTarget()->name.isEmpty() )
            caption = i18n ( "%1 in %2" ).arg ( selectedTarget()->primary ).arg ( selectedTarget()->prefix );
          else
            caption = selectedTarget()->name;

          dlg.setCaption ( i18n ( "Add New File to '%1'" ).arg ( caption ) );

          if ( dlg.exec() )
            slotDetailsSelectionChanged( m_shownSubproject ); // update list view
        }
}


void AutoProjectWidget::slotAddIcon()
{
	TargetItem * titem = selectedTarget();
	if ( !titem )
		return ;

	AddIconDialog dlg( m_part, this, m_shownSubproject, titem,
	                   this, "add icon" );
	dlg.exec();
}


void AutoProjectWidget::slotBuildTarget()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( details->selectedItem() );

	if ( !pvitem )
		return;

	TargetItem* titem = 0;

	if ( pvitem->type() == ProjectItem::File )
		titem = static_cast <TargetItem*> ( pvitem->parent() );
	else
		titem = selectedTarget();

	// 	TargetItem * titem = selectedTarget();
	// 	if ( !titem )
	// 		return ;

	QString name = titem->name;
	if ( titem->primary == "LIBRARIES" )
		name + ".a";
	else if ( titem->primary == "LTLIBRARIES" )
		name + ".la";
	else if ( titem->primary == "KDEDOCS" )
		name = "index.cache.bz2";

	QString relpath = m_shownSubproject->path.mid( projectDirectory().length() );
	m_part->startMakeCommand( m_part->buildDirectory() + relpath, titem->name );

	m_part->topLevel() ->lowerView( this );
}


void AutoProjectWidget::slotRemoveDetail()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( details->selectedItem() );

	if ( pvitem && ( pvitem->type() == ProjectItem::File ) )
	{
		FileItem * fitem = selectedFile();
		QListViewItem* sibling = fitem->nextSibling();

		if ( !fitem )
			return;

		TargetItem *titem = static_cast<TargetItem*>( fitem->parent() );

		RemoveFileDialog dlg( this, m_shownSubproject, titem, fitem->text( 0 ),
		                      this, "remove file dialog" );

		QString caption;
		if ( titem->name.isEmpty() )
			caption = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
		else
			caption = titem->name;

		dlg.setCaption ( i18n ( "Remove File from '%1'" ).arg ( caption ) );

		if ( dlg.exec() )
		{
			slotDetailsSelectionChanged( selectedSubproject() );

			if ( sibling)
			{
				details->setSelected ( sibling, true );
				details->ensureItemVisible ( sibling );
			}
		}

		return;
	}

	if ( pvitem && ( pvitem->type() == ProjectItem::Target ) )
	{
		TargetItem* titem = selectedTarget();
		QListViewItem* sibling = titem->nextSibling();

		if ( !titem )
			return;

		//KMessageBox::information ( this, "Deleting " + titem->name );

		RemoveTargetDialog dlg ( this, selectedSubproject(), titem, this, "remove target dialog" );

		dlg.setCaption ( i18n ( "Remove Target from '%1'" ).arg ( selectedSubproject()->subdir ) );

		if ( dlg.exec() )
		{
			//details->takeItem ( titem );

			slotDetailsSelectionChanged ( selectedSubproject() );

			if ( sibling)
			{
				details->setSelected ( sibling, true );
				details->ensureItemVisible ( sibling );
			}
		}

		return;
	}
}

void AutoProjectWidget::slotRemoveSubproject()
{
	ProjectItem* pvitem = static_cast <ProjectItem*> ( overview->selectedItem() );

	if ( pvitem && ( pvitem->type() == ProjectItem::Subproject ) )
	{
		SubprojectItem* spitem = selectedSubproject();

		if ( !spitem )
			return;

		KMessageBox::information ( this, "Deleting Subproject " + spitem->path );

		return;
	}
}

void AutoProjectWidget::slotSetActiveTarget()
{
	TargetItem * titem = selectedTarget();
	if ( !titem )
		return ;

	QString targetPath = m_shownSubproject->path + "/" + titem->name;
	targetPath = targetPath.mid( projectDirectory().length() + 1 );
	kdDebug( 9020 ) << "Setting active " << targetPath << endl;
	setActiveTarget( targetPath );
	QDomDocument &dom = *m_part->projectDom();
	DomUtil::writeEntry( dom, "/kdevautoproject/general/activetarget", targetPath );
}


void AutoProjectWidget::slotOverviewContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	if ( !item )
		return ;

	KPopupMenu popup( i18n( "Subproject: %1" ).arg( item->text( 0 ) ), this );

	subProjectOptionsAction->plug( &popup );
	popup.insertSeparator();
	addSubprojectAction->plug( &popup );
	addTargetAction->plug( &popup );
	addServiceAction->plug( &popup );
	addApplicationAction->plug( &popup );
	popup.insertSeparator();
	addExistingSubprojectAction->plug( &popup );
	popup.insertSeparator();
	removeSubprojectAction->plug( &popup );
	popup.insertSeparator();
	buildSubprojectAction->plug( &popup );

	popup.exec( p );
}


void AutoProjectWidget::slotDetailsContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	if ( !item )
		return ;

	ProjectItem *pvitem = static_cast<ProjectItem*>( item );

	if ( pvitem->type() == ProjectItem::Target )
	{

		TargetItem * titem = static_cast<TargetItem*>( pvitem );

		KPopupMenu popup( i18n( "Target: %1" ).arg( titem->name ), this );

		if ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES"
		        || titem->primary == "LTLIBRARIES" || titem->primary == "JAVA" )
		{
			targetOptionsAction->plug( &popup );
			popup.insertSeparator();
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
			popup.insertSeparator();
			setActiveTargetAction->plug( &popup );
			popup.insertSeparator();
			buildTargetAction->plug( &popup );
		}
		else if ( titem->primary == "KDEDOCS" )
		{
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
			popup.insertSeparator();
			buildTargetAction->plug( &popup );
		}
		else if ( titem->primary == "KDEICON" )
		{
			addIconAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
		}
		else
		{
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
		}

		popup.exec( p );

	}
	else if ( pvitem->type() == ProjectItem::File )
	{

		FileItem * fitem = static_cast<FileItem*>( pvitem );

		KPopupMenu popup( i18n( "File: %1" ).arg( fitem->name ), this );

		removeDetailAction->plug( &popup );
		FileContext context( m_shownSubproject->path + "/" + fitem->name, false );
		m_part->core() ->fillContextMenu( &popup, &context );

		popup.exec( p );

	}
}


TargetItem *AutoProjectWidget::selectedTarget()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( details->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::Target ) )
		return 0;

	return static_cast<TargetItem*>( pvitem );
}


FileItem *AutoProjectWidget::selectedFile()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( details->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::File ) )
		return 0;

	return static_cast<FileItem*>( pvitem );
}

SubprojectItem* AutoProjectWidget::selectedSubproject()
{
	ProjectItem * pvitem = static_cast <SubprojectItem*> ( overview->selectedItem() );

	if ( !pvitem || ( pvitem->type() != ProjectItem::Subproject ) )
		return 0;

	return static_cast <SubprojectItem*> ( pvitem );
}

TargetItem *AutoProjectWidget::createTargetItem( const QString &name,
                                                 const QString &prefix, const QString &primary )
{
	bool group = !( primary == "PROGRAMS" || primary == "LIBRARIES"
	                || primary == "LTLIBRARIES" || primary == "JAVA" );
	bool docgroup = ( primary == "KDEDOCS" );
	bool icongroup = ( primary == "KDEICON" );

	QString text;
	if ( docgroup )
		text = i18n( "Documentation data" );
	else if ( icongroup )
		text = i18n( "Icon data in %1" ).arg( prefix );
	else if ( group )
		text = i18n( "%1 in %2" ).arg( nicePrimary( primary ) ).arg( prefix );
	else
		text = i18n( "%1 (%2 in %3)" ).arg( name ).arg( nicePrimary( primary ) ).arg( prefix );

	// Workaround because of QListView not being able to create
	// items without actually inserting them
	TargetItem *titem = new TargetItem( overview, group, text );
	overview->takeItem( titem );
	titem->name = name;
	titem->prefix = prefix;
	titem->primary = primary;

	return titem;
}


FileItem *AutoProjectWidget::createFileItem( const QString &name )
{
	FileItem * fitem = new FileItem( overview, name );
	overview->takeItem( fitem );
	fitem->name = name;

	return fitem;
}


void AutoProjectWidget::emitAddedFiles( const QStringList &fileList )
{
	emit m_part->addedFilesToProject( fileList );
}

void AutoProjectWidget::emitAddedFile( const QString &name )
{
	QStringList fileList;
	fileList.append ( name );
	emit m_part->addedFilesToProject( fileList );
}

void AutoProjectWidget::emitRemovedFiles( const QStringList &fileList )
{
	emit m_part->removedFilesFromProject( fileList );
}

void AutoProjectWidget::emitRemovedFile( const QString &name )
{
	QStringList fileList;
	fileList.append ( name );
	emit m_part->removedFilesFromProject( fileList );
}

void AutoProjectWidget::restoreSession ( const QDomElement* el )
{

}

void AutoProjectWidget::saveSession ( QDomElement* el )
{
	kdDebug ( 9020 ) << "************** Saving session data of AutoProjectWidget: " << endl;

	if ( m_activeTarget && m_activeSubproject )
	{
		QDomDocument domDoc = el->ownerDocument();

		QString activeTargetPath = m_activeSubproject->path.mid ( m_part->project()->projectDirectory().length() + 1 );
		activeTargetPath = activeTargetPath + "/" + m_activeTarget->name;

		QDomElement generalEl = domDoc.createElement("general");

		kdDebug ( 9020 ) << "************** Saving session data of AutoProjectWidget: " << activeTargetPath << endl;

		generalEl.setAttribute("activetarget", activeTargetPath);
		el->appendChild(generalEl);
	}
}
void AutoProjectWidget::parsePrimary( SubprojectItem *item,
                                      const QString &lhs, const QString &rhs )
{
	// Parse line foo_bar = bla bla

	int pos = lhs.findRev( '_' );
	QString prefix = lhs.left( pos );
	QString primary = lhs.right( lhs.length() - pos - 1 );
	//    kdDebug(9020) << "Prefix:" << prefix << ",Primary:" << primary << endl;

#if 0

	QStrList prefixes;
	prefixes.append( "bin" );
	prefixes.append( "pkglib" );
	prefixes.append( "pkgdata" );
	prefixes.append( "noinst" );
	prefixes.append( "check" );
	prefixes.append( "sbin" );
	QStrList primaries;
	primaries.append( "PROGRAMS" );
	primaries.append( "LIBRARIES" );
	primaries.append( "LTLIBRARIES" );
	primaries.append( "SCRIPTS" );
	primaries.append( "HEADERS" );
	primaries.append( "DATA" );
#endif

	// Not all combinations prefix/primary are possible, so this
	// could also be checked... not trivial because the list of
	// possible prefixes can be extended dynamically (see below)
	if ( primary == "PROGRAMS" || primary == "LIBRARIES" || primary == "LTLIBRARIES" )
	{
		QStringList l = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
		QStringList::Iterator it1;
		for ( it1 = l.begin(); it1 != l.end(); ++it1 )
		{
			TargetItem *titem = createTargetItem( *it1, prefix, primary );
			item->targets.append( titem );

			QString canonname = AutoProjectTool::canonicalize( *it1 );
			titem->ldflags = cleanWhitespace( item->variables[ canonname + "_LDFLAGS" ] );
			titem->ldadd = cleanWhitespace( item->variables[ canonname + "_LDADD" ] );
			titem->libadd = cleanWhitespace( item->variables[ canonname + "_LIBADD" ] );
			titem->dependencies = cleanWhitespace( item->variables[ canonname + "_DEPENDENCIES" ] );

			QString sources = item->variables[ canonname + "_SOURCES" ];
			QStringList l2 = QStringList::split( QRegExp( "[ \t\n]" ), sources );
			QStringList::Iterator it2;
			for ( it2 = l2.begin(); it2 != l2.end(); ++it2 )
			{
				FileItem *fitem = createFileItem( *it2 );
				titem->sources.append( fitem );
				if ( !kdeMode() || !( *it2 ).endsWith( ".cpp" ) )
					continue;
				QString header = ( *it2 ).left( ( *it2 ).length() - 4 ) + ".h";
				if ( sources.contains( header ) )
					continue;
				fitem = createFileItem( header );
				titem->sources.append( fitem );
			}
		}
	}
	else if ( primary == "SCRIPTS" || primary == "HEADERS" || primary == "DATA" )
	{
		// See if we have already such a group
		for ( uint i = 0; i < item->targets.count(); ++i )
		{
			TargetItem *titem = item->targets.at( i );
			if ( primary == titem->primary && prefix == titem->prefix )
			{
				item->targets.remove( i );
				break;
			}
		}
		// Create a new one
		TargetItem *titem = createTargetItem( "", prefix, primary );
		item->targets.append( titem );

		QStringList l = QStringList::split( QRegExp( "[ \t]" ), rhs );
		QStringList::Iterator it3;
		for ( it3 = l.begin(); it3 != l.end(); ++it3 )
		{
			FileItem *fitem = createFileItem( *it3 );
			titem->sources.append( fitem );
		}
	}
	else if ( primary == "JAVA" )
	{
		QStringList l = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
		QStringList::Iterator it1;
		TargetItem *titem = createTargetItem( "", prefix, primary );
		item->targets.append( titem );

		for ( it1 = l.begin(); it1 != l.end(); ++it1 )
		{
			FileItem *fitem = createFileItem( *it1 );
			titem->sources.append( fitem );
		}
	}
}


void AutoProjectWidget::parseKDEDOCS( SubprojectItem *item,
                                      const QString & /*lhs*/, const QString & /*rhs*/ )
{
	// Handle the line KDE_ICON =
	// (actually, no parsing is involved here)

	QString prefix = "kde_docs";
	QString primary = "KDEDOCS";

	TargetItem *titem = createTargetItem( "", prefix, primary );
	item->targets.append( titem );

	QDir d( item->path );
	QStringList l = d.entryList( QDir::Files );

	QRegExp re( "Makefile.*|\\..*|.*~|index.cache.bz2" );

	QStringList::ConstIterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	{
		if ( !re.exactMatch( *it ) )
		{
			FileItem * fitem = createFileItem( *it );
			titem->sources.append( fitem );
		}
	}
}


void AutoProjectWidget::parseKDEICON( SubprojectItem *item,
                                      const QString &lhs, const QString &rhs )
{
	// Parse a line foo_ICON = bla bla

	int pos = lhs.find( "_ICON" );
	QString prefix = lhs.left( pos );
	if ( prefix == "KDE" )
		prefix = "kde_icon";

	QString primary = "KDEICON";

	TargetItem *titem = createTargetItem( "", prefix, primary );
	item->targets.append( titem );

	QDir d( item->path );
	QStringList l = d.entryList( QDir::Files );

	QString regexp;

	if ( rhs == "AUTO" )
	{
		regexp = ".*\\.(png|mng|xpm)";
	}
	else
	{
		QStringList appNames = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
		regexp = ".*(-" + appNames.join( "|-" ) + ")\\.(png|mng|xpm)";
	}

	QRegExp re( regexp );

	QStringList::ConstIterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	{
		if ( re.exactMatch( *it ) )
		{
			FileItem * fitem = createFileItem( *it );
			titem->sources.append( fitem );
		}
	}
}


void AutoProjectWidget::parsePrefix( SubprojectItem *item,
                                     const QString &lhs, const QString &rhs )
{
	// Parse a line foodir = bla bla
	QString name = lhs.left( lhs.length() - 3 );
	QString dir = rhs;
	item->prefixes.insert( name, dir );
}


void AutoProjectWidget::parseSUBDIRS( SubprojectItem *item,
                                      const QString & /*lhs*/, const QString &rhs )
{
	// Parse a line SUBDIRS = bla bla
	QString subdirs = rhs;
	kdDebug( 9020 ) << "subdirs are " << subdirs << endl;

	// Take care of KDE hacks:
	// TOPSUBDIRS is an alias for all directories
	// listed in the subdirs file
	if ( subdirs.find( "$(TOPSUBDIRS)" ) != -1 )
	{
		QStringList dirs;
		QFile subdirsfile( item->path + "/subdirs" );
		if ( subdirsfile.open( IO_ReadOnly ) )
		{
			QTextStream subdirsstream( &subdirsfile );
			while ( !subdirsstream.atEnd() )
				dirs.append( subdirsstream.readLine() );
			subdirsfile.close();
		}
		subdirs.replace( QRegExp( "\\$\\(TOPSUBDIRS\\)" ), dirs.join( " " ) );
	}

	// AUTODIRS is an alias for all subdirectories
	if ( subdirs.find( "$(AUTODIRS)" ) != -1 )
	{
		QDir d( item->path );
		QStringList dirs = d.entryList( QDir::Dirs );
		dirs.remove( "." );
		dirs.remove( ".." );
		dirs.remove( "CVS" );
		subdirs.replace( QRegExp( "\\$\\(AUTODIRS\\)" ), dirs.join( " " ) );
	}

	// If there are any variables in the subdirs line then search
	// the Makefile(.am?) for its definition. Unfortunately, it may be
	// defined outside this file in which case those dirs won't be added.
	QRegExp varre( "\\$\\(\\s*(.*)\\s*\\)" );
	varre.setMinimal( true );
	while ( varre.search( subdirs ) != -1 )
	{
		QString varname = varre.cap( 1 );
		QString varvalue;

		// Search the whole Makefile(.am?)
		// Note that if the variable isn't found it just disappears
		// (Perhaps we should add it back in this case?)
		QMap<QString, QString>::ConstIterator varit = item->variables.find( varname );
		if ( varit != item->variables.end() )
		{
			kdDebug( 9020 ) << "Found Makefile var " << varname << ", adding dirs <" << varit.data() << ">" << endl;
			varvalue = varit.data();
		}
		else
		{
			kdDebug( 9020 ) << "Not found Makefile var " << varname << endl;
		}
		subdirs.replace( QRegExp( "\\$\\(\\s*" + varname + "\\s*\\)" ), varvalue );
	}

	QStringList l = QStringList::split( QRegExp( "[ \t]" ), subdirs );
	l.sort();
	QStringList::Iterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	{
		if ( *it == "." )
			continue;
		SubprojectItem *newitem = new SubprojectItem( item, ( *it ) );
		newitem->subdir = ( *it );
		newitem->path = item->path + "/" + ( *it );
		parse( newitem );
		// Experience tells me this :-)
		bool open = true;
		if ( newitem->subdir == "doc" )
			open = false;
		if ( newitem->subdir == "po" )
			open = false;
		if ( newitem->subdir == "pics" )
			open = false;
		if ( newitem && static_cast<SubprojectItem*>( newitem->parent() )
		        ->subdir == "doc" )
			open = false;
		if ( newitem && static_cast<SubprojectItem*>
		        ( newitem->parent() ) ->subdir == "po" )
			open = false;
		if ( newitem && static_cast<SubprojectItem*>
		        ( newitem->parent() ) ->subdir == "pics" )
			open = false;
		newitem->setOpen( open );

		// Move to the bottom of the list
		QListViewItem *lastItem = item->firstChild();
		while ( lastItem->nextSibling()
		      )
			lastItem = lastItem->nextSibling();
		if ( lastItem != newitem )
			newitem->moveItem( lastItem );
	}
}


void AutoProjectWidget::parse( SubprojectItem *item )
{
	AutoProjectTool::parseMakefileam( item->path + "/Makefile.am", &item->variables );

	QMap<QString, QString>::ConstIterator it;
	for ( it = item->variables.begin(); it != item->variables.end(); ++it )
	{
		QString lhs = it.key();
		QString rhs = it.data();
		if ( lhs == "KDE_DOCS" )
			parseKDEDOCS( item, lhs, rhs );
		else if ( lhs.right( 5 ) == "_ICON" )
			parseKDEICON( item, lhs, rhs );
		else if ( lhs.find( '_' ) > 0 )
			parsePrimary( item, lhs, rhs );
		else if ( lhs.right( 3 ) == "dir" )
			parsePrefix( item, lhs, rhs );
		else if ( lhs == "SUBDIRS" )
			parseSUBDIRS( item, lhs, rhs );
	}
}

#include "autoprojectwidget.moc"
