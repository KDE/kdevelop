/***************************************************************************
                             -------------------
    begin                : 19.01.2003
    copyright            : (C) 2002 by Victor Rder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** Qt */
#include <qregexp.h>
#include <qcheckbox.h>

/** KDE Libs */
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kapplication.h>

/** Gideon */
#include <kdevmainwindow.h>
#include <kdevmakefrontend.h>

/** AutoProject */
#include "subprojectoptionsdlg.h"
#include "addsubprojectdlg.h"
#include "addtargetdlg.h"
#include "addservicedlg.h"
#include "addapplicationdlg.h"
#include "addexistingdirectoriesdlg.h"
#include "autolistviewitems.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"
#include "autosubprojectview.h"
#include "removesubprojectdialog.h"

namespace AutoProjectPrivate
{

static bool isHeader( const QString& fileName )
{
    return QStringList::split( ";", "h;H;hh;hxx;hpp;tcc" ).contains( QFileInfo(fileName).extension() );
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

static void removeDir( const QString& dirName )
{
    QDir d( dirName );
    const QFileInfoList* fileList = d.entryInfoList();
    if( !fileList )
	return;
    
    QFileInfoListIterator it( *fileList );
    while( it.current() ){
	const QFileInfo* fileInfo = it.current();
	++it;
	
	if( fileInfo->fileName() == "." || fileInfo->fileName() == ".." )
	    continue;
	
	if( fileInfo->isDir() && !fileInfo->isSymLink() )
	    removeDir( fileInfo->absFilePath() );
	
	kdDebug(9020) << "remove " << fileInfo->absFilePath() << endl;
	d.remove( fileInfo->fileName(), false );
    }
    
    kdDebug(9020) << "remove dir " << dirName << endl;
    d.rmdir( d.absPath(), true );
}

}


AutoSubprojectView::AutoSubprojectView(AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name)
  : KListView(parent, name)
{
	m_widget = widget;
	m_part = part;

	initActions();
}


AutoSubprojectView::~AutoSubprojectView()
{
}

void AutoSubprojectView::loadMakefileams ( const QString& dir )
{
	SubprojectItem * item = new SubprojectItem( this, m_part->projectDirectory() + "/" + m_part->projectName() + ".kdevelop" );
	item->setPixmap ( 0, SmallIcon ( "kdevelop" ) );
	item->subdir = "/";
	item->path = dir;
	parse( item );
	item->setOpen( true );

	setSelected( item, true );
}


void AutoSubprojectView::initActions()
{
	KActionCollection * actions = new KActionCollection( this );

	subProjectOptionsAction = new KAction( i18n( "Options..." ), "configure", 0,
	                                       this, SLOT( slotSubprojectOptions() ), actions, "subproject options" );
	addSubprojectAction = new KAction( i18n( "Add Subproject..." ), "folder_new", 0,
	                                   this, SLOT( slotAddSubproject() ), actions, "add subproject" );
	removeSubprojectAction = new KAction( i18n( "Remove Subproject..." ), "folder_remove", 0,
	                                   this, SLOT( slotRemoveSubproject() ), actions, "remove subproject" );
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

	connect( this, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( slotContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
/*	connect ( this, SIGNAL ( executed ( QListViewItem* ),
			this, SLOT ( slotSubprojectExecuted ( QListViewItem* ) ) );
	connect ( this, SIGNAL ( returnPressed ( QListViewItem* ),
			this, SLOT ( slotSubprojectExecuted ( QListViewItem* ) ) );*/
}

void AutoSubprojectView::slotContextMenu( KListView *, QListViewItem *item, const QPoint &p )
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
/*
void AutoSubprojectView::slotSubprojectExecuted ( QListViewItem* item )
{
	emit selectionChanged ( item );
}*/

void AutoSubprojectView::slotSubprojectOptions()
{
	kdDebug( 9020 ) << "AutoSubprojectView::slotSubprojectOptions()" << endl;

	SubprojectItem* spitem = static_cast <SubprojectItem*> ( selectedItem() );
	if ( !spitem )	return;

	SubprojectOptionsDialog dlg( m_part, m_widget, spitem, this, "subproject options dialog" );
	dlg.exec();
}


void AutoSubprojectView::slotAddSubproject()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	AddSubprojectDialog dlg( m_part, this, spitem, this, "add subproject dialog" );

	dlg.setCaption ( i18n ( "Add New Subproject to '%1'" ).arg ( spitem->subdir ) );
	dlg.exec();
}


void AutoSubprojectView::slotAddExistingSubproject()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	AddExistingDirectoriesDialog dlg ( m_part, m_widget, spitem, this, "add existing subprojects" );

	dlg.setCaption ( i18n ( "Add Existing Subproject to '%1'" ).arg ( spitem->subdir ) );

	if ( dlg.exec() )
		emit selectionChanged ( spitem );
}

void AutoSubprojectView::slotAddTarget()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	AddTargetDialog dlg( m_widget, spitem, this, "add target dialog" );

	dlg.setCaption ( i18n ( "Add New Target to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if a target was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotAddService()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	AddServiceDialog dlg( m_widget, spitem, this, "add service dialog" );

	dlg.setCaption ( i18n ( "Add New Service to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if a service was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotAddApplication()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	AddApplicationDialog dlg( m_widget, spitem, this, "add application dialog" );

	dlg.setCaption ( i18n ( "Add New Application to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if an application was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotBuildSubproject()
{
	SubprojectItem* spitem = static_cast <SubprojectItem*>  ( selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, QString::fromLatin1( "" ) );

	m_part->mainWindow() ->lowerView( m_widget );
}

void AutoSubprojectView::slotRemoveSubproject()
{
    kdDebug(9020) << "AutoSubprojectView::slotRemoveSubproject()" << endl;
    
    SubprojectItem* spitem = static_cast<SubprojectItem*>( selectedItem() );
    if( !spitem )
	return;
    
    SubprojectItem* parent = static_cast<SubprojectItem*>( spitem->parent() );
    if( !parent || !parent->listView() || spitem->childCount() != 0 ){
	KMessageBox::error( 0, i18n("This item can't be removed"), i18n("Automake Manager") );
	return;
    }
    
    // check for config.status
    if( !QFileInfo(m_part->buildDirectory(), "config.status").exists() ){
	KMessageBox::sorry(this, i18n("There is no config.status in the project root build directory. Run 'Configure' first"));
	return;
    }
    
    QStringList list = QStringList::split( QRegExp("[ \t]"), parent->variables["SUBDIRS"] );
    QStringList::Iterator it = list.find( spitem->subdir );
    if( it == list.end() ){
	KMessageBox::sorry(this, i18n("There is no subproject %1 in SUBDIRS").arg(spitem->subdir));
	return;
    }
    
    RemoveSubprojectDialog dlg;
    if( dlg.exec() ){
	
	bool removeSources = dlg.removeCheckBox->isChecked();
	
	list.remove( it );
	parent->variables[ "SUBDIRS" ] = list.join( " " );
	
	parent->listView()->setSelected( parent, true );
	kapp->processEvents( 500 );
	
	
	if( removeSources ){
	    kdDebug(9020) << "remove dir " << spitem->path << endl;
	    AutoProjectPrivate::removeDir( spitem->path );
	}
	
	if( m_widget->activeSubproject() == spitem ){
	    m_widget->setActiveSubproject( 0 );
	}
	// remove all targets
	spitem->targets.setAutoDelete( true );
	spitem->targets.clear();
	delete( spitem );
	spitem = 0;
	
	// Adjust SUBDIRS variable in containing Makefile.am
	QMap<QString,QString> replaceMap;
	replaceMap.insert( "SUBDIRS", parent->variables["SUBDIRS"] );
	AutoProjectTool::modifyMakefileam( parent->path + "/Makefile.am", replaceMap );
	
	QString relmakefile = ( parent->path + "/Makefile" ).mid( m_part->projectDirectory().length()+1 );
	kdDebug(9020) << "Relative makefile path: " << relmakefile << endl;
	
	QString cmdline = "cd ";
	cmdline += m_part->projectDirectory();
	cmdline += " && automake ";
	cmdline += relmakefile;
	cmdline += " && cd ";
	cmdline += m_part->buildDirectory();
	cmdline += " && CONFIG_HEADERS=config.h CONFIG_FILES=";
	cmdline += relmakefile;
	cmdline += " ./config.status";
	m_part->makeFrontend()->queueCommand( m_part->projectDirectory(), cmdline );
    }
}


void AutoSubprojectView::parsePrimary( SubprojectItem *item,
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
			TargetItem *titem = m_widget->createTargetItem( *it1, prefix, primary );
			item->targets.append( titem );

			QString canonname = AutoProjectTool::canonicalize( *it1 );
			titem->ldflags = AutoProjectPrivate::cleanWhitespace( item->variables[ canonname + "_LDFLAGS" ] );
			titem->ldadd = AutoProjectPrivate::cleanWhitespace( item->variables[ canonname + "_LDADD" ] );
			titem->libadd = AutoProjectPrivate::cleanWhitespace( item->variables[ canonname + "_LIBADD" ] );
			titem->dependencies = AutoProjectPrivate::cleanWhitespace( item->variables[ canonname + "_DEPENDENCIES" ] );

			QString sources = item->variables[ canonname + "_SOURCES" ];
			QStringList sourceList = QStringList::split( QRegExp( "[ \t\n]" ), sources );
			QMap<QString, bool> dict;
			QStringList::Iterator it = sourceList.begin();
			while( it != sourceList.end() ){
			    dict.insert( *it, true );
			    ++it;
			}

			QMap<QString, bool>::Iterator dictIt = dict.begin();
			while( dictIt != dict.end() ){
				QString fname = dictIt.key();
				++dictIt;

				FileItem *fitem = m_widget->createFileItem( fname, item );
				titem->sources.append( fitem );

				if( AutoProjectPrivate::isHeader(fname) )
					headers += fname;
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
		TargetItem *titem = m_widget->createTargetItem( "", prefix, primary );
		item->targets.append( titem );

		QStringList l = QStringList::split( QRegExp( "[ \t]" ), rhs );
		QStringList::Iterator it3;
		for ( it3 = l.begin(); it3 != l.end(); ++it3 )
		{
			QString fname = *it3;
			FileItem *fitem = m_widget->createFileItem( fname, item );
			titem->sources.append( fitem );

			if( AutoProjectPrivate::isHeader(fname) )
				headers += fname;

		}
	}
	else if ( primary == "JAVA" )
	{
		QStringList l = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
		QStringList::Iterator it1;
		TargetItem *titem = m_widget->createTargetItem( "", prefix, primary );
		item->targets.append( titem );

		for ( it1 = l.begin(); it1 != l.end(); ++it1 )
		{
			FileItem *fitem = m_widget->createFileItem( *it1, item );
			titem->sources.append( fitem );
		}
	}
}


void AutoSubprojectView::parseKDEDOCS( SubprojectItem *item,
                                      const QString & /*lhs*/, const QString & /*rhs*/ )
{
	// Handle the line KDE_ICON =
	// (actually, no parsing is involved here)

	QString prefix = "kde_docs";
	QString primary = "KDEDOCS";

	TargetItem *titem = m_widget->createTargetItem( "", prefix, primary );
	item->targets.append( titem );

	QDir d( item->path );
	QStringList l = d.entryList( QDir::Files );

	QRegExp re( "Makefile.*|\\..*|.*~|index.cache.bz2" );

	QStringList::ConstIterator it;
	for ( it = l.begin(); it != l.end(); ++it )
	{
		if ( !re.exactMatch( *it ) )
		{
			QString fname = *it;
			FileItem * fitem = m_widget->createFileItem( fname, item );
			titem->sources.append( fitem );
		}
	}
}


void AutoSubprojectView::parseKDEICON( SubprojectItem *item,
                                      const QString &lhs, const QString &rhs )
{
	// Parse a line foo_ICON = bla bla

	int pos = lhs.find( "_ICON" );
	QString prefix = lhs.left( pos );
	if ( prefix == "KDE" )
		prefix = "kde_icon";

	QString primary = "KDEICON";

	TargetItem *titem = m_widget->createTargetItem( "", prefix, primary );
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
			FileItem * fitem = m_widget->createFileItem( *it, item );
			titem->sources.append( fitem );
		}
	}
}


void AutoSubprojectView::parsePrefix( SubprojectItem *item,
                                     const QString &lhs, const QString &rhs )
{
	// Parse a line foodir = bla bla
	QString name = lhs.left( lhs.length() - 3 );
	QString dir = rhs;
	item->prefixes.insert( name, dir );
}


void AutoSubprojectView::parseSUBDIRS( SubprojectItem *item,
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

void AutoSubprojectView::parse( SubprojectItem *item )
{
	headers.clear();
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

	// TODO: only if in a c++ project
	TargetItem* noinst_HEADERS_item = 0;
	QPtrListIterator<TargetItem> itemIt( item->targets );
	while( itemIt.current() ){
	    TargetItem* titem = itemIt.current();
	    ++itemIt;

	    if( titem->prefix == "noinst" && titem->primary == "HEADERS" ){
	        noinst_HEADERS_item = titem;
		break;
	    }
	}

	if( !noinst_HEADERS_item ){
	    noinst_HEADERS_item = m_widget->createTargetItem( "", "noinst", "HEADERS" );
	    item->targets.append( noinst_HEADERS_item );
	}

	QDir dir( item->path );
	QStringList headersList = QStringList::split( "[ \t]", item->variables[ "noinst_HEADERS" ] );
	headersList += dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tcc", QDir::Files );
	headersList.sort();

	item->variables[ "noinst_HEADERS" ] = headersList.join( " " );

	QStringList::Iterator fileIt = headersList.begin();
	while( fileIt != headersList.end() ){
	    QString fname = *fileIt;
	    ++fileIt;

	    if( AutoProjectPrivate::isHeader(fname) && !headers.contains(fname) ){
	        FileItem *fitem = m_widget->createFileItem( fname, item );
	        noinst_HEADERS_item->sources.append( fitem );
	    }
	}
}


#include "autosubprojectview.moc"
