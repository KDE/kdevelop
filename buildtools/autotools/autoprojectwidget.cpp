/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Rder                                    *
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
#include <qwhatsthis.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kregexp.h>
#include <kurl.h>
#include <kfile.h>
#include <kaction.h>

#include "kdevcore.h"
#include "domutil.h"
#include "misc.h"
#include "choosetargetdialog.h"

#include "autolistviewitems.h"

#include "autoprojectpart.h"
#include "autosubprojectview.h"
#include "autodetailsview.h"
#include "urlutil.h"

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


AutoProjectWidget::AutoProjectWidget( AutoProjectPart *part, bool kde )
		: QVBox( 0, "auto project widget" )
{
	m_part = part;
	m_kdeMode = kde;
	m_activeSubproject = 0;
	m_activeTarget = 0;
	m_shownSubproject = 0;
	m_choosenTarget = 0;

	QSplitter *splitter = new QSplitter(Vertical, this);

	initOverview ( splitter );
	initDetailview ( splitter );

	initActions ();
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
		( addSubprojectButton, i18n( "Add subproject" ) );
    QWhatsThis::add(addSubprojectButton, i18n("<b>Add subproject</b><p>Creates a new subproject in currently selected subproject."));

	/*    addExistingSubprojectButton = new QToolButton(overviewButtonBox);
		addExistingSubprojectButton->setPixmap(SmallIcon("fileimport"));
		QToolTip::add(addExistingSubprojectButton, i18n("Add existing subproject to currently selected subproject..."));
	*/
	addTargetButton = new QToolButton( overviewButtonBox );
	addTargetButton->setPixmap( SmallIcon( "targetnew_kdevelop" ) );
	QToolTip::add
		( addTargetButton, i18n( "Add target" ) );
    QWhatsThis::add(addTargetButton, i18n("<b>Add target</b><p>Adds a new target to the currently selected subproject. Target can be a binary program, library, script, also a collection of data or header files."));

	addServiceButton = new QToolButton( overviewButtonBox );
	addServiceButton->setPixmap( SmallIcon( "servicenew_kdevelop" ) );
	QToolTip::add
		( addServiceButton, i18n( "Add service" ) );
    QWhatsThis::add(addServiceButton, i18n("<b>Add service</b><p>Creates a .desktop file describing the service."));

	addApplicationButton = new QToolButton( overviewButtonBox );
	addApplicationButton->setPixmap( SmallIcon( "window_new" ) );
	QToolTip::add
		( addApplicationButton, i18n( "Add application" ) );
    QWhatsThis::add(addApplicationButton, i18n("<b>Add application</b><p>Creates an application .desktop file."));

	buildSubprojectButton = new QToolButton( overviewButtonBox );
	buildSubprojectButton->setPixmap( SmallIcon( "launch" ) );
	QToolTip::add
		( buildSubprojectButton, i18n( "Build" ) );
    QWhatsThis::add(buildSubprojectButton, i18n("<b>Build</b><p>Runs <b>make</b> from the directory of the selected subproject.<br>"
                                                "Environment variables and make arguments can be specified "
                                                "in the project settings dialog, <b>Make Options</b> tab."));

	QWidget *spacer1 = new QWidget( overviewButtonBox );
	overviewButtonBox->setStretchFactor( spacer1, 1 );

	subProjectOptionsButton = new QToolButton( overviewButtonBox );
	subProjectOptionsButton->setPixmap( SmallIcon( "configure" ) );
	QToolTip::add
		( subProjectOptionsButton, i18n( "Options" ) );
    QWhatsThis::add(subProjectOptionsButton, i18n("<b>Options</b><p>Shows subproject options dialog that provides settings for compiler, include paths, prefixes and build order."));

	overviewButtonBox->setMaximumHeight( subProjectOptionsButton->height() );

	m_subprojectView = new AutoSubprojectView( this, m_part, overviewBox, "project overview widget" );

	m_subprojectView->setResizeMode( QListView::LastColumn );
	m_subprojectView->setSorting( -1 );
	m_subprojectView->header() ->hide();
	m_subprojectView->addColumn( QString::null );
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
		( addNewFileButton, i18n( "Create new file" ) );
    QWhatsThis::add(addNewFileButton, i18n("<b>Create new file</b><p>Creates a new file and adds it to a currently selected target."));

	addExistingFileButton = new QToolButton( targetButtonBox );
	addExistingFileButton->setPixmap( SmallIcon( "fileimport" ) );
	QToolTip::add
		( addExistingFileButton, i18n( "Add existing files" ) );
    QWhatsThis::add(addExistingFileButton, i18n("<b>Add existing files</b><p>Adds existing file to a currently selected target. Header files will not be included in SOURCES list of a target. "
                                                "They will be added to noinst_HEADERS instead."));

	removeButton = new QToolButton( targetButtonBox );
	removeButton->setPixmap( SmallIcon( "editdelete" ) );
	QToolTip::add
		( removeButton, i18n( "Remove" ) );
    QWhatsThis::add(removeButton, i18n("<b>Remove</b><p>Shows a list of targets dependent on the selected target or file and asks for removal. Also asks if the target or file should be removed from disk."));

	buildTargetButton = new QToolButton( targetButtonBox );
	buildTargetButton->setPixmap( SmallIcon( "launch" ) );
	QToolTip::add
		( buildTargetButton, i18n( "Build" ) );
    QWhatsThis::add(buildTargetButton, i18n("<b>Build target</b><p>Constructs a series of make commands to build the selected target. "
                                            "Also builds dependent targets."));
					    
	executeTargetButton = new QToolButton( targetButtonBox );
	executeTargetButton->setPixmap( SmallIcon( "exec") );
	QToolTip::add
		( executeTargetButton, i18n( "Execute" ) );
    QWhatsThis::add(executeTargetButton, i18n("<b>Execute target</b><p> Executes target, if it's a program. Tries to build it, if that's not yet done."));					    

	QWidget *spacer2 = new QWidget( targetButtonBox );
	targetButtonBox->setStretchFactor( spacer2, 1 );

	targetOptionsButton = new QToolButton( targetButtonBox );
	targetOptionsButton->setPixmap( SmallIcon( "configure" ) );
	QToolTip::add
		( targetOptionsButton, i18n( "Show options" ) );
    QWhatsThis::add(targetOptionsButton, i18n("<b>Options</b><p>Target options dialog that provides settings for linker flags and lists of dependencies and external libraries."));

	targetButtonBox->setMaximumHeight( addNewFileButton->height() );

	targetOptionsButton->setEnabled( false );
	addNewFileButton->setEnabled( false );
	addExistingFileButton->setEnabled( false );
	removeButton->setEnabled( true );
	buildTargetButton->setEnabled( true );
	executeTargetButton->setEnabled( true );	

	m_detailView = new AutoDetailsView( this, m_part, targetBox, "project details widget" );
	m_detailView->setRootIsDecorated( true );
	m_detailView->setResizeMode( QListView::LastColumn );
	m_detailView->setSorting( -1 );
	m_detailView->header() ->hide();
	m_detailView->addColumn( QString::null );
}

void AutoProjectWidget::initActions()
{
	connect( subProjectOptionsButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotSubprojectOptions() ) );
	connect( addSubprojectButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotAddSubproject() ) );
	//    connect( addExistingSubprojectButton, SIGNAL(clicked()), this, SLOT(slotAddExistingSubproject()) );
	connect( addTargetButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotAddTarget() ) );
	connect( addServiceButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotAddService() ) );
	connect( addApplicationButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotAddApplication() ) );
	connect( buildSubprojectButton, SIGNAL( clicked() ), m_subprojectView, SLOT( slotBuildSubproject() ) );

	connect( targetOptionsButton, SIGNAL( clicked() ), m_detailView, SLOT( slotTargetOptions() ) );
	connect( addNewFileButton, SIGNAL( clicked() ), m_detailView, SLOT( slotAddNewFile() ) );
	connect( addExistingFileButton, SIGNAL( clicked() ), m_detailView, SLOT( slotAddExistingFile() ) );
	connect( removeButton , SIGNAL( clicked() ), m_detailView, SLOT( slotRemoveDetail() ) );
	connect( buildTargetButton, SIGNAL( clicked() ), m_detailView, SLOT( slotBuildTarget() ) );
	connect( executeTargetButton, SIGNAL( clicked() ), m_detailView, SLOT( slotExecuteTarget() ) );	

	connect( m_subprojectView, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotOverviewSelectionChanged( QListViewItem* ) ) );

	connect( m_detailView, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotDetailsSelectionChanged( QListViewItem* ) ) );
}

AutoSubprojectView* AutoProjectWidget::getSubprojectView ()
{
	return m_subprojectView;
}

AutoDetailsView* AutoProjectWidget::getDetailsView ()
{
	return m_detailView;
}

void AutoProjectWidget::openProject( const QString &dirName )
{
	m_subprojectView->loadMakefileams ( dirName );
}


void AutoProjectWidget::closeProject()
{
	m_subprojectView->clear();
	m_detailView->clear();
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
	int prefixlen = m_part->projectDirectory().length() + 1;
	QStringList res;

	QListViewItemIterator it( m_subprojectView );
	for ( ; it.current(); ++it )
	{
		// Skip root subproject
		// if ( it.current() == m_subprojectView->firstChild() )
		//	continue;
		QString path = static_cast<SubprojectItem*>( it.current() ) ->path;
		res.append( path.mid( prefixlen ) );
	}

	return res;
}

QPtrList <SubprojectItem> AutoProjectWidget::allSubprojectItems()
{
	QPtrList <SubprojectItem> res;

	QListViewItemIterator it ( m_subprojectView );

	for ( ; it.current(); ++it )
	{
		// Skip root subproject
		// if ( it.current() == m_subprojectView->firstChild() )
		//	continue;

		SubprojectItem* spitem = static_cast <SubprojectItem*> ( it.current() );

		res.append ( spitem );
	}

	return res;
}

SubprojectItem* AutoProjectWidget::subprojectItemForPath(const QString & path, bool pathIsAbsolute)
{
       kdDebug(9020) << "Looking for path " << path << endl;

        int prefixLen = m_part->projectDirectory().length() + 1;
        for(QListViewItemIterator it = m_subprojectView;it.current();++it)
        {
                SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
                QString relpath = (spitem->path).mid(prefixLen);
                relpath = (relpath.isNull()) ? QString("") : relpath;
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
  int prefixLen = m_part->projectDirectory().length() + 1;
  for(QListViewItemIterator it = m_subprojectView;it.current();++it)
  {
    SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
    kdDebug(9020) << "Checking: " << spitem->path << endl;
    if (spitem->targets.containsRef(titem)) {
      kdDebug(9020) << "Found it!" << endl;
      QString relpath = (spitem->path).mid(prefixLen);
      return (relpath.isNull()) ? QString("") : relpath;
    }
  }
  kdDebug(9020) << "Not found" << endl;
  return QString::null;
}

QStringList AutoProjectWidget::allLibraries()
{
	int prefixlen = m_part->projectDirectory().length() + 1;
	QStringList res;

	QListViewItemIterator it( m_subprojectView );
	for ( ; it.current(); ++it )
	{
		SubprojectItem *spitem = static_cast<SubprojectItem*>( it.current() );
		QString path = spitem->path;
		QPtrListIterator<TargetItem> tit( spitem->targets );
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
	QPtrStack<QListViewItem> s;
	QMap<QString, bool> dict;

	for ( QListViewItem * item = m_subprojectView->firstChild(); item;
	        item = item->nextSibling() ? item->nextSibling() : s.pop() )
	{
		if ( item->firstChild() )
			s.push( item->firstChild() );

		SubprojectItem *spitem = static_cast<SubprojectItem*>( item );
		// use URLUtil so paths in root project dir are worked out correctly
		QString relPath = URLUtil::relativePath(m_part->projectDirectory(), spitem->path, URLUtil::SLASH_SUFFIX);
		QPtrListIterator<TargetItem> tit( spitem->targets );
		for ( ; tit.current(); ++tit )
		{
			QPtrListIterator<FileItem> fit( tit.current() ->sources );
			for ( ; fit.current(); ++fit ){

				if((*fit)->is_subst)
					continue;
				
				QFileInfo fileInfo( (*fit)->name );
				if( fileInfo.extension() == "ui" ){
				    dict.insert( relPath + fileInfo.baseName() + ".h", true );
				    dict.insert( relPath + fileInfo.baseName() + ".cpp", true );
				}

				dict.insert( relPath + ( *fit ) ->name, true );
			}
		}
	}

	// Files may be in multiple targets, so we have to remove
	// duplicates
	QStringList res;
	QMap<QString, bool>::Iterator it = dict.begin();
	while( it != dict.end() ){
	    res << it.key();
	    ++it;
	}

	return res;
}


QString AutoProjectWidget::subprojectDirectory()
{
	if ( !selectedSubproject() )
		return QString::null;

	return selectedSubproject() ->path;
}


void AutoProjectWidget::setActiveTarget( const QString &targetPath )
{
	int prefixlen = m_part->projectDirectory().length() + 1;

	m_activeSubproject = 0;
	m_activeTarget = 0;

	QListViewItemIterator it( m_subprojectView );
	for ( ; it.current(); ++it )
	{
		SubprojectItem *spitem = static_cast<SubprojectItem*>( it.current() );
		QString path = spitem->path;
		QPtrListIterator<TargetItem> tit( spitem->targets );
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
				m_subprojectView->setSelected( m_activeSubproject, true );
				m_subprojectView->ensureItemVisible ( m_activeSubproject );
				m_subprojectView->viewport() ->update();
				m_detailView->setSelected ( m_activeTarget, true );
				m_detailView->ensureItemVisible ( m_activeTarget );
				m_detailView->viewport()->update();
			}
			else
			{
				// to avoid a setBold ( false ) if there's another target in the current Subproject (i.e. spitem) ...
				spitem->setBold ( ( m_activeSubproject == spitem ) );
				m_detailView->viewport()->update();
			}
		}
	}

	if ( m_activeSubproject == 0 &&
		m_activeTarget == 0 )
	{
		m_subprojectView->setSelected ( m_subprojectView->firstChild(), true );
		m_subprojectView->ensureItemVisible ( m_subprojectView->firstChild() );
		m_subprojectView->viewport()->update();
	}
}


QString AutoProjectWidget::activeDirectory()
{
	if ( m_activeSubproject )
		return m_activeSubproject->path.mid( m_part->projectDirectory().length() + 1 );
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

                        //FileItem * fitem = createFileItem( fileName,m_activeSubproject );
			//m_activeTarget->sources.append( fitem );
			//m_activeTarget->insertItem( fitem );

			/// @todo Merge with code in addfiledlg.cpp
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
		        ChooseTargetDialog chooseTargetDlg ( this, m_part, doManually, this, "choose target dialog" );

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
    QString varname;
    /// \FIXME a quick hack to prevent adding header files to _SOURCES and display them in noinst_HEADERS
    if (AutoProjectPrivate::isHeader(fileName) &&
        ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES" ||  titem->primary == "LTLIBRARIES" ) )
    {
        kdDebug ( 9020 ) << "Ignoring header file and adding it to noinst_HEADERS: " << fileName << endl;
        TargetItem* noinst_HEADERS_item = getSubprojectView()->findNoinstHeaders(spitem);
        FileItem *fitem = createFileItem( fileName, spitem );
        noinst_HEADERS_item->sources.append( fitem );
        noinst_HEADERS_item->insertItem( fitem );
        varname = "noinst_HEADERS";
    }
    else
    {
        FileItem * fitem = createFileItem( fileName, spitem );
        titem->sources.append( fitem );
        titem->insertItem( fitem );

        QString canontargetname = AutoProjectTool::canonicalize( titem->name );
        varname = canontargetname + "_SOURCES";
    }
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
		return;

	// Delete the items from the details view first.
	if ( m_shownSubproject )
	{
		// Remove all TargetItems and all of their children from the view
		kdDebug ( 9020 ) << "m_shownSubproject (before takeItem()): " << m_shownSubproject->subdir << endl;

		QPtrListIterator<TargetItem> it1( m_shownSubproject->targets );
		for ( ; it1.current(); ++it1 )
		{
			// After AddTargetDialog, it can happen that an
			// item is not yet in the list view, so better check...
			kdDebug ( 9020 ) << "take child items of  " << ( *it1 )->name << endl;
			if ( it1.current() ->parent() )
			{
				while ( ( *it1 ) ->firstChild() )
				{
					kdDebug ( 9020 ) << "take FileItem " << ( *it1 ) ->firstChild()->text(0) << endl;
					( *it1 ) ->takeItem( ( *it1 ) ->firstChild() );
				}
			}
			m_detailView->takeItem( *it1 );
		}
	}

	// We assume here that ALL items in the over list view
	// are SubprojectItem's
	m_shownSubproject = static_cast<SubprojectItem*>( item );

	kdDebug ( 9020 ) << "m_shownSubproject (after takeItem()):  " << selectedSubproject()->subdir << endl;

	// Insert all TargetItems and all of their children into the view
	QPtrListIterator<TargetItem> it2( selectedSubproject()->targets );
	for ( ; it2.current(); ++it2 )
	{
		kdDebug ( 9020 ) << "insertItem in detail " << ( *it2 )->name << endl;
		m_detailView->insertItem( *it2 );
		QPtrListIterator<FileItem> it3( ( *it2 ) ->sources );
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
	executeTargetButton->setEnabled ( false );	
}


void AutoProjectWidget::slotDetailsSelectionChanged( QListViewItem *item )
{
	bool isTarget = false;
	bool isRegularTarget = false;
	bool isFile = false;
	bool isProgram = false;

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

		if ( primary == "PROGRAMS" )
			isProgram = true;
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
	{
		buildTargetButton->setEnabled ( true );
		if( isProgram )
			executeTargetButton->setEnabled ( true );
	}
	else
	{
		buildTargetButton->setEnabled ( false );
		executeTargetButton->setEnabled ( false );		
	}
			
	
}

TargetItem *AutoProjectWidget::selectedTarget()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( m_detailView->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::Target ) )
		return 0;

	return static_cast<TargetItem*>( pvitem );
}


FileItem *AutoProjectWidget::selectedFile()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( m_detailView->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::File ) )
		return 0;

	return static_cast<FileItem*>( pvitem );
}

SubprojectItem* AutoProjectWidget::selectedSubproject()
{
	ProjectItem * pvitem = static_cast <SubprojectItem*> ( m_subprojectView->selectedItem() );

	if ( !pvitem || ( pvitem->type() != ProjectItem::Subproject ) )
		return 0;

	return static_cast <SubprojectItem*> ( pvitem );
}

TargetItem *AutoProjectWidget::createTargetItem( const QString &name,
                                                 const QString &prefix, const QString &primary,
						 bool take )
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
	TargetItem *titem = new TargetItem( m_detailView, group, text );
	titem->name = name;
	titem->prefix = prefix;
	titem->primary = primary;
	if( take )
	    m_detailView->takeItem( titem );

	return titem;
}


FileItem *AutoProjectWidget::createFileItem( const QString &name, SubprojectItem *subproject )
{
        bool is_subst;
	if(name.find("$(") == 0 || name.find("${") == 0)
	is_subst = true;
	else
	is_subst = false;
	
	FileItem * fitem = new FileItem( m_subprojectView, name, is_subst );
	fitem->uiFileLink = m_detailView->getUiFileLink(subproject->relativePath()+"/", name );
	m_subprojectView->takeItem( fitem );
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
    Q_UNUSED( el );
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

void AutoProjectWidget::setActiveSubproject( SubprojectItem * spitem )
{
    m_activeSubproject = spitem;
}

void AutoProjectWidget::focusInEvent( QFocusEvent */*e*/ )
{
    switch (m_lastFocusedView)
    {
        case DetailsView:
            m_detailView->setFocus();
            break;
        case SubprojectView:
        default:
            m_subprojectView->setFocus();
    }
}

void AutoProjectWidget::setLastFocusedView( AutoProjectView view )
{
    m_lastFocusedView = view;
}

#include "autoprojectwidget.moc"
