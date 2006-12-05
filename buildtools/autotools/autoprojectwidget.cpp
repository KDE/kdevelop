/*
   KDevelop Autotools Support
   Copyright (c) 2001-2002 by Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (c) 2002 by Victor Roeder <victor_roeder@gmx.de>
   Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/


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
#include <kxmlguiclient.h>
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
#include "makefilehandler.h"

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
        m_makefileHandler = new MakefileHandler();

	QSplitter *splitter = new QSplitter(Vertical, this);

	initOverview ( splitter );
	initDetailview ( splitter );

	initActions ();
}


AutoProjectWidget::~AutoProjectWidget()
{
    delete m_makefileHandler;
}

void AutoProjectWidget::initOverview ( QWidget* parent )
{
	m_subprojectView = new AutoSubprojectView( this, m_part, parent, "project overview widget" );
}

void AutoProjectWidget::initDetailview ( QWidget* parent )
{
	m_detailView = new AutoDetailsView( this, m_part, parent, "project details widget" );
}

void AutoProjectWidget::initActions()
{
	connect( m_subprojectView, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotOverviewSelectionChanged( QListViewItem* ) ) );
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
	MakefileHandler mfh;
	mfh.parse( m_part->projectDirectory(), true );

}


void AutoProjectWidget::closeProject()
{
	m_shownSubproject = 0;
	m_subprojectView->listView()->clear();
	m_detailView->listView()->clear();
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

	QListViewItemIterator it( m_subprojectView->listView() );
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

	QListViewItemIterator it ( m_subprojectView->listView() );

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
	QListViewItemIterator it( m_subprojectView->listView() );
	for(; it.current(); ++it)
	{
		SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
		QString relpath = (spitem->path).mid(prefixLen);
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

QString AutoProjectWidget::pathForTarget(const TargetItem *titem) const
{

	if (!titem)
		return QString::null;

	kdDebug(9020) << "Looking for target " << titem->name << endl;
	int prefixLen = m_part->projectDirectory().length() + 1;
	QListViewItemIterator it( m_subprojectView->listView() );
	for(; it.current(); ++it)
	{
		SubprojectItem* spitem = static_cast<SubprojectItem*>(it.current() );
		kdDebug(9020) << "Checking: " << spitem->path << endl;
		if (spitem->targets.containsRef(titem))
		{
			kdDebug(9020) << "Found it!" << endl;
			QString relpath = (spitem->path).mid(prefixLen);
			return relpath;
		}
	}
	kdDebug(9020) << "Not found" << endl;
	return QString::null;
}

QStringList AutoProjectWidget::allLibraries()
{
	int prefixlen = m_part->projectDirectory().length() + 1;
	QStringList res;

	QListViewItemIterator it( m_subprojectView->listView() );
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

	for ( QListViewItem * item = m_subprojectView->listView()->firstChild(); item;
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
			for ( ; fit.current(); ++fit )
			{

				if((*fit)->is_subst)
					continue;

				QFileInfo fileInfo( (*fit)->name );
				if( fileInfo.extension() == "ui" )
				{
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

	return selectedSubproject()->path;
}


void AutoProjectWidget::setActiveTarget( const QString &targetPath )
{
	int prefixlen = m_part->projectDirectory().length() + 1;

	m_activeSubproject = 0;
	m_activeTarget = 0;

	QListViewItemIterator it( m_subprojectView->listView() );
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
				m_subprojectView->listView()->setSelected( m_activeSubproject, true );
				m_subprojectView->listView()->ensureItemVisible ( m_activeSubproject );
				m_subprojectView->listView()->viewport()->update();
				m_detailView->listView()->setSelected ( m_activeTarget, true );
				m_detailView->listView()->ensureItemVisible ( m_activeTarget );
				m_detailView->listView()->viewport()->update();
			}
			else
			{
				// to avoid a setBold ( false ) if there's another target in the current Subproject (i.e. spitem) ...
				spitem->setBold ( ( m_activeSubproject == spitem ) );
				m_detailView->listView()->viewport()->update();
			}
		}
	}

	if ( m_activeSubproject == 0 && m_activeTarget == 0 )
	{
		m_subprojectView->listView()->setSelected ( m_subprojectView->listView()->firstChild(), true );
		m_subprojectView->listView()->ensureItemVisible ( m_subprojectView->listView()->firstChild() );
		m_subprojectView->listView()->viewport()->update();
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
				fileName = ( *it ).mid(pos+1);
			else
				fileName = ( *it );

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
//			AutoProjectTool::addToMakefileam( m_activeSubproject->path + "/Makefile.am", replaceMap );
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

			if ( chooseTargetDlg.exec() && chooseTargetDlg.alwaysUseActiveTarget() )
				DomUtil::writeBoolEntry( dom, "/kdevautoproject/general/useactivetarget", true );
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

	AutoProjectTool::addToMakefileam( spitem->path + "/Makefile.am", replaceMap );

	m_detailView->slotSelectionChanged( spitem );
}

void AutoProjectWidget::removeFiles( const QStringList &list )
{
	Q_UNUSED( list )
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
			m_detailView->listView()->takeItem( *it1 );
		}
	}

	// We assume here that ALL items in the over list view
	// are SubprojectItem's
	m_shownSubproject = dynamic_cast<SubprojectItem*>( item );
	if ( !m_shownSubproject) return;
	kdDebug ( 9020 ) << "m_shownSubproject (after takeItem()):  " << selectedSubproject()->subdir << endl;

	// Insert all TargetItems and all of their children into the view
	QPtrListIterator<TargetItem> it2( selectedSubproject()->targets );
	for ( ; it2.current(); ++it2 )
	{
		kdDebug ( 9020 ) << "insertItem in detail " << ( *it2 )->name << endl;
		m_detailView->listView()->insertItem( *it2 );
		QPtrListIterator<FileItem> it3( ( *it2 ) ->sources );
		for ( ; it3.current(); ++it3 )
			( *it2 )->insertItem( *it3 );
		QString primary = ( *it2 ) ->primary;
		if ( primary == "PROGRAMS" || primary == "LIBRARIES" ||
		     primary == "LTLIBRARIES" || primary == "JAVA" )
			( *it2 ) ->setOpen( true );
	}
}

TargetItem *AutoProjectWidget::selectedTarget()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( m_detailView->listView()->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::Target ) )
		return 0;

	return static_cast<TargetItem*>( pvitem );
}


FileItem *AutoProjectWidget::selectedFile()
{
	ProjectItem * pvitem = static_cast<ProjectItem*>( m_detailView->listView()->selectedItem() );
	if ( !pvitem || ( pvitem->type() != ProjectItem::File ) )
		return 0;

	return static_cast<FileItem*>( pvitem );
}

SubprojectItem* AutoProjectWidget::selectedSubproject()
{
	ProjectItem * pvitem = static_cast <SubprojectItem*> ( m_subprojectView->listView()->selectedItem() );

	if ( !pvitem || ( pvitem->type() != ProjectItem::Subproject ) )
		return 0;

	return static_cast <SubprojectItem*> ( pvitem );
}

TargetItem *AutoProjectWidget::createTargetItem( const QString &name,
                                                 const QString &prefix, const QString &primary,
                                                 bool take )
{
	bool docgroup = ( primary == "KDEDOCS" );
	bool icongroup = ( primary == "KDEICON" );
	bool group = !(docgroup || icongroup);

	QString text;
	if ( docgroup )
		text = i18n( "Documentation data" );
	else if ( icongroup )
		text = i18n( "KDE Icon data" ).arg( prefix );
	else
		text = i18n( "%1 (%2 in %3)" ).arg( name ).arg( nicePrimary( primary ) ).arg( prefix );

	// Workaround because of QListView not being able to create
	// items without actually inserting them
	TargetItem *titem = new TargetItem( m_detailView->listView(), group, text );
	titem->name = name;
	titem->prefix = prefix;
	titem->primary = primary;
	if( take )
		m_detailView->listView()->takeItem( titem );

	return titem;
}


FileItem *AutoProjectWidget::createFileItem( const QString &name, SubprojectItem *subproject )
{
	bool is_subst;
	if(name.find("$(") == 0 || name.find("${") == 0)
		is_subst = true;
	else
		is_subst = false;

	FileItem * fitem = new FileItem( m_subprojectView->listView(), name, is_subst );
	fitem->uiFileLink = m_detailView->getUiFileLink(subproject->relativePath()+"/", name );
	m_subprojectView->listView()->takeItem( fitem );
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
	if ( m_activeTarget && m_activeSubproject )
	{
		QDomDocument domDoc = el->ownerDocument();

		QString activeTargetPath = m_activeSubproject->path.mid ( m_part->project()->projectDirectory().length() + 1 );
		activeTargetPath = activeTargetPath + "/" + m_activeTarget->name;

		QDomElement generalEl = domDoc.createElement("general");

		kdDebug ( 9020 ) << k_funcinfo << "Saving session data of AutoProjectWidget: " << activeTargetPath << endl;

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
		m_detailView->listView()->setFocus();
		break;
	case SubprojectView:
	default:
		m_subprojectView->listView()->setFocus();
	}
}

void AutoProjectWidget::setLastFocusedView( AutoProjectView view )
{
	m_lastFocusedView = view;
}

#include "autoprojectwidget.moc"

MakefileHandler* AutoProjectWidget::makefileHandler()
{
	return m_makefileHandler;
}
//kate: indent-mode csands; tab-width 4; space-indent off;

