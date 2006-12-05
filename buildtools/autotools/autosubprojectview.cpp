/*
   KDevelop Autotools Support
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

/** Qt */
#include <qregexp.h>
#include <qcheckbox.h>
#include <qstringlist.h>
#include <qtable.h>
#include <qlayout.h>

/** KDE Libs */
#include <kxmlguiclient.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kprocess.h>
#include <ksqueezedtextlabel.h>
#include <kdialogbase.h>
#include <klistview.h>

/** KDevelop */
#include <kdevmainwindow.h>
#include <kdevmakefrontend.h>
#include <kdevappfrontend.h>

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
#include "autotoolsaction.h"
#include "removesubprojectdialog.h"
#include "managecustomcommand.h"

namespace AutoProjectPrivate
{

bool isHeader( const QString& fileName )
{
    return QStringList::split( ";", "h;H;hh;hxx;hpp;tcc;h++" ).contains( QFileInfo(fileName).extension(false) );
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
: AutoProjectViewBase(parent, name)
{

	m_widget = widget;
	m_part = part;

	m_listView->setSorting(-1);
	m_listView->header()->hide();
	m_listView->addColumn( QString::null );

	connect( m_listView, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotSelectionChanged( QListViewItem* ) ) );

	initActions();
}


AutoSubprojectView::~AutoSubprojectView()
{
}

void AutoSubprojectView::slotSelectionChanged( QListViewItem* item )
{
	if ( m_listView->selectedItems().count() <= 0 )
	{
		subProjectOptionsAction->setEnabled( false );
		addSubprojectAction->setEnabled( false );
		addTargetAction->setEnabled( false );
		addServiceAction->setEnabled( false );
		addApplicationAction->setEnabled( false );
		buildSubprojectAction->setEnabled( false );
	}
	else
	{
		subProjectOptionsAction->setEnabled( true );
		addSubprojectAction->setEnabled( true );
		addTargetAction->setEnabled( true );
		addServiceAction->setEnabled( true );
		addApplicationAction->setEnabled( true );
		buildSubprojectAction->setEnabled( true );
	}

	emit selectionChanged( item );
}

void AutoSubprojectView::loadMakefileams ( const QString& dir )
{
	SubprojectItem * item = new SubprojectItem( m_listView, m_part->projectName() );
	item->setPixmap ( 0, SmallIcon ( "kdevelop" ) );
	item->subdir = "/";
	item->path = dir;
	parse( item );
	item->setOpen( true );

	//setSelected( item, true );

	expandCollapseFirst( m_listView->firstChild(), false );
}


void AutoSubprojectView::initActions()
{
	KActionCollection * actions = new KActionCollection( this );

	subProjectOptionsAction = new AutoToolsAction( i18n( "Options..." ), "configure", 0,
	                                       this, SLOT( slotSubprojectOptions() ), actions, "subproject options" );
	subProjectOptionsAction->setWhatsThis(i18n("<qt><b>Options</b><p>Shows subproject options dialog "
	                                           "that provides settings for compiler, include paths, "
	                                           "prefixes and build order.</qt>"));
	subProjectOptionsAction->plug( m_optionsButton );

	QToolTip::add( m_button1, tr2i18n( "Add new subproject..."));
	addSubprojectAction = new AutoToolsAction( i18n( "Add new subproject..." ), "folder_new", 0,
	                                   this, SLOT( slotAddSubproject() ), actions, "add subproject" );
	addSubprojectAction->setWhatsThis(i18n("<qt><b>Add new subproject</b><p>Creates a new "
	                                       "subproject in currently selected subproject.</qt>"));
	addSubprojectAction->plug( m_button1 );

	removeSubprojectAction = new KAction( i18n( "Remove Subproject..." ), "remove_subdir", 0,
	                                      this, SLOT( slotRemoveSubproject() ), actions, "remove subproject" );
	removeSubprojectAction->setWhatsThis(i18n("<qt><b>Remove subproject</b><p>Removes the subproject. Asks if the "
	                                          "subproject should be also removed from disk. Only subprojects "
	                                          "which do not hold other subprojects can be removed.</qt>"));
	addExistingSubprojectAction = new KAction( i18n( "Add Existing Subprojects..." ), "fileimport", 0,
	                                           this, SLOT( slotAddExistingSubproject() ), actions, "add existing subproject" );
	addExistingSubprojectAction->setWhatsThis(i18n("<qt><b>Add existing subprojects</b><p>Imports existing "
	                                               "subprojects containing Makefile.am.</qt>"));

	QToolTip::add( m_button2, tr2i18n( "Add Target..."));
	addTargetAction = new AutoToolsAction( i18n( "Add Target..." ), "targetnew_kdevelop", 0,
	                               this, SLOT( slotAddTarget() ), actions, "add target" );
	addTargetAction->setWhatsThis(i18n( "<qt><b>Add target</b><p>Adds a new target to "
	                                    "the currently selected subproject. Target can be a "
	                                    "binary program, library, script, also a collection of "
	                                    "data or header files.</qt>"));
	addTargetAction->plug( m_button2 );

	QToolTip::add( m_button3, tr2i18n( "Add Service..."));
	addServiceAction = new AutoToolsAction( i18n( "Add Service..." ), "servicenew_kdevelop", 0, this,
	                                SLOT( slotAddService() ), actions, "add service" );
	addServiceAction->setWhatsThis(i18n("<qt><b>Add service</b><p>Creates a .desktop file describing the service.</qt>"));
	addServiceAction->plug( m_button3 );

	QToolTip::add( m_button4, tr2i18n( "Add Application..."));
	addApplicationAction = new AutoToolsAction( i18n( "Add Application..." ), "window_new", 0, this,
	                                    SLOT( slotAddApplication() ), actions, "add application" );
	addApplicationAction->setWhatsThis(i18n("<qt><b>Add application</b><p>Creates an application .desktop file.</qt>"));
	addApplicationAction->plug( m_button4 );

	QToolTip::add( m_button5, tr2i18n( "Build"));
	buildSubprojectAction = new AutoToolsAction( i18n( "Build" ), "launch", 0, this,
	                                     SLOT( slotBuildSubproject() ), actions, "build subproject" );
	buildSubprojectAction->setWhatsThis(i18n("<qt><b>Build</b><p>Runs <b>make</b> from the directory of "
	                                         "the selected subproject.<br> Environment variables and "
	                                         "make arguments can be specified in the project settings "
	                                         "dialog, <b>Make Options</b> tab.</qt>"));
	buildSubprojectAction->plug( m_button5 );

	forceReeditSubprojectAction = new KAction( i18n( "Force Reedit" ), 0, 0, this,
	                                           SLOT( slotForceReeditSubproject() ), actions, "force-reedit subproject" );
	forceReeditSubprojectAction->setWhatsThis(i18n("<qt><b>Force Reedit</b><p>Runs <b>make force-reedit</b> "
	                                               "from the directory of the selected subproject.<br>This "
	                                               "recreates makefile (tip: and solves most of .moc related "
	                                               "problems)<br>Environment variables and make arguments can "
	                                               "be specified in the project settings dialog, "
	                                               "<b>Make Options</b> tab.</qt>"));

	if (!m_part->isKDE())
		forceReeditSubprojectAction->setEnabled(false);

	cleanSubprojectAction = new KAction( i18n( "Clean" ), 0, 0, this,
	                                     SLOT( slotCleanSubproject() ), actions, "clean subproject" );
	cleanSubprojectAction->setWhatsThis(i18n("<qt><b>Clean</b><p>Runs <b>make clean</b> from the directory of "
	                                         "the selected subproject.<br> Environment variables and make "
	                                         "arguments can be specified in the project settings dialog, "
	                                         "<b>Make Options</b> tab.</qt>"));

	installSubprojectAction = new KAction( i18n( "Install" ), 0, 0, this,
	                                       SLOT( slotInstallSubproject() ), actions, "install subproject" );
	installSubprojectAction->setWhatsThis(i18n("<qt><b>Install</b><p>Runs <b>make install</b> from the directory "
	                                           "of the selected subproject.<br> Environment variables and "
	                                           "make arguments can be specified in the project settings "
	                                           "dialog, <b>Make Options</b> tab.</qt>"));
	installSuSubprojectAction = new KAction( i18n( "Install (as root user)" ), 0, 0,
	                                         this, SLOT( slotInstallSuSubproject() ), actions, "install subproject as root" );
	installSuSubprojectAction->setWhatsThis(i18n("<qt><b>Install as root user</b><p>Runs <b>make install</b> "
	                                             "command from the directory of the selected subproject "
	                                             "with root privileges.<br> It is executed via kdesu "
	                                             "command.<br> Environment variables and make arguments "
	                                             "can be specified in the project settings dialog, "
	                                             "<b>Make Options</b> tab.</qt>"));

	expandAction = new KAction( i18n( "Expand Subtree" ), 0, 0, this,
	                            SLOT(slotExpandTree()), actions, "expandAction" );
	collapseAction = new KAction( i18n( "Collapse Subtree" ), 0, 0, this,
	                              SLOT(slotCollapseTree()), actions, "collapseAction" );

	otherAction = new KAction( i18n( "Manage Custom Commands..." ), 0, 0, this,
	                           SLOT( slotManageBuildCommands() ), actions, "manage custom commands" );
	otherAction->setWhatsThis(i18n("<qt><b>Manage custom commands</b><p>Allows to create, edit and "
	                               "delete custom build commands which appears in the subproject "
	                               "context menu.<br></qt>"));

	connect( m_listView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( slotContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
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
	popup.insertSeparator();
	forceReeditSubprojectAction->plug( &popup );
	cleanSubprojectAction->plug( &popup );
	popup.insertSeparator();
	installSubprojectAction->plug( &popup );
	installSuSubprojectAction->plug( &popup );
	popup.insertSeparator();
	collapseAction->plug( &popup );
	expandAction->plug( &popup );

	KConfig *config = m_part->instance()->config();
	bool separate = true;
	QMap<QString,QString> customBuildCommands = config->entryMap("CustomCommands");
	for (QMap<QString,QString>::const_iterator it = customBuildCommands.constBegin();
		it != customBuildCommands.constEnd(); ++it)
	{
		if (separate)
		{
			popup.insertSeparator();
			separate = false;
		}
		int id = popup.insertItem(it.key(), this, SLOT(slotCustomBuildCommand(int)));
		m_commandList.append(it.data());
		popup.setItemParameter(id, m_commandList.findIndex(it.data()));
	}

	popup.insertSeparator();
	otherAction->plug( &popup );

	popup.exec( p );
}

void AutoSubprojectView::slotSubprojectOptions()
{
	kdDebug( 9020 ) << "AutoSubprojectView::slotSubprojectOptions()" << endl;

	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	SubprojectOptionsDialog dlg( m_part, m_widget, spitem, this, "subproject options dialog" );
	dlg.exec();
}


void AutoSubprojectView::slotAddSubproject()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	AddSubprojectDialog dlg( m_part, this, spitem, this, "add subproject dialog" );

	dlg.setCaption ( i18n ( "Add New Subproject to '%1'" ).arg ( spitem->subdir ) );
	dlg.exec();
}


void AutoSubprojectView::slotAddExistingSubproject()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	AddExistingDirectoriesDialog dlg ( m_part, m_widget, spitem, this, "add existing subprojects" );

	dlg.setCaption ( i18n ( "Add Existing Subproject to '%1'" ).arg ( spitem->subdir ) );
    dlg.targetLabel->setText("");
    dlg.directoryLabel->setText(spitem->path);

	if ( dlg.exec() )
		emit selectionChanged ( spitem );
}

void AutoSubprojectView::slotAddTarget()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	AddTargetDialog dlg( m_widget, spitem, this, "add target dialog" );

	dlg.setCaption ( i18n ( "Add New Target to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if a target was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotAddService()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	AddServiceDialog dlg( m_widget, spitem, this, "add service dialog" );

	dlg.setCaption ( i18n ( "Add New Service to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if a service was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotAddApplication()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	AddApplicationDialog dlg( m_widget, spitem, this, "add application dialog" );

	dlg.setCaption ( i18n ( "Add New Application to '%1'" ).arg ( spitem->subdir ) );

	// Update the details view if an application was added
	if ( dlg.exec() )
		emit selectionChanged( spitem );
}


void AutoSubprojectView::slotBuildSubproject()
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, QString::fromLatin1( "" ) );
}

void AutoSubprojectView::slotRemoveSubproject()
{
    kdDebug(9020) << "AutoSubprojectView::slotRemoveSubproject()" << endl;

	SubprojectItem* spitem = static_cast<SubprojectItem*>( m_listView->selectedItem() );
    if( !spitem )
	return;

    SubprojectItem* parent = static_cast<SubprojectItem*>( spitem->parent() );
    if( !parent || !parent->listView() || spitem->childCount() != 0 ){
	KMessageBox::error( 0, i18n("This item cannot be removed"), i18n("Automake Manager") );
	return;
    }

    QStringList list = QStringList::split( QRegExp("[ \t]"), parent->variables["SUBDIRS"] );
    QStringList::Iterator it = list.find( spitem->subdir );
    QString subdirToRemove = spitem->subdir;
    bool topsubdirs = true;
    if ((parent->variables["SUBDIRS"].find("$(TOPSUBDIRS)") == -1)
        && (parent->variables["SUBDIRS"].find("$(AUTODIRS)") == -1))
    {
        topsubdirs = false;
        if( it == list.end() ){
            KMessageBox::sorry(this, i18n("There is no subproject %1 in SUBDIRS").arg(spitem->subdir));
            return;
        }
    }

    RemoveSubprojectDialog dlg(i18n("Remove Subproject %1").arg(spitem->text(0)),
        i18n("Do you really want to remove subproject %1 with all targets and files?").arg(spitem->text(0)));
    if( dlg.exec() ){

    bool removeSources = dlg.removeFromDisk();

    if (!topsubdirs)
    {
    list.remove( it );
    parent->variables[ "SUBDIRS" ] = list.join( " " );
    }

    parent->listView()->setSelected( parent, true );
    kapp->processEvents( 500 );


    if( removeSources ){
        kdDebug(9020) << "remove dir " << spitem->path << endl;
        AutoProjectPrivate::removeDir( spitem->path );
    }

    if( m_widget->activeSubproject() == spitem ){
        m_widget->setActiveSubproject( 0 );
    }

    // Adjust AC_OUTPUT in configure.in
    if ( !m_part->isKDE() ) {

        QString projroot = m_part->projectDirectory() + "/";
        QString subdirectory = spitem->path;
        QString relpath = subdirectory.replace(0, projroot.length(),"");

        QString configureFile = m_part->getAutoConfFile(projroot);

        QStringList list = AutoProjectTool::configureinLoadMakefiles(configureFile);

        QStringList::iterator it;

        for ( it = list.begin(); it != list.end(); it++ ) {
            QString current = (QString) (*it);
            QRegExp path_regex(relpath);
            if ( path_regex.search(current) >= 0) {
                list.remove(it);
                break;
            }
        }
        AutoProjectTool::configureinSaveMakefiles(configureFile, list);

    }

    // remove all targets
    spitem->targets.setAutoDelete( true );
    spitem->targets.clear();
    delete( spitem );
    spitem = 0;

    // Adjust SUBDIRS variable in containing Makefile.am

    if (parent->variables["SUBDIRS"].find("$(TOPSUBDIRS)") != -1)
    {
        QFile subdirsfile( parent->path + "/subdirs" );
        QStringList topdirs;
        if ( subdirsfile.open( IO_ReadOnly ) )
        {
            QTextStream subdirsstream( &subdirsfile );
            while (!subdirsstream.atEnd())
                topdirs.append(subdirsstream.readLine());
            subdirsfile.close();
        }
        topdirs.remove(subdirToRemove);
        if ( subdirsfile.open( IO_WriteOnly | IO_Truncate ) )
        {
            QTextStream subdirsstream( &subdirsfile );
            for (QStringList::const_iterator it = topdirs.begin(); it != topdirs.end(); ++it)
                subdirsstream << *it << endl;
            subdirsfile.close();
        }
    }

	QMap<QString,QString> replaceMap;
	    replaceMap.insert( "SUBDIRS", subdirToRemove );
	AutoProjectTool::removeFromMakefileam( parent->path + "/Makefile.am", replaceMap );

	QString relmakefile = ( parent->path + "/Makefile" ).mid( m_part->projectDirectory().length()+1 );
	kdDebug(9020) << "Relative makefile path: " << relmakefile << endl;

    // check for config.status
	if( !QFileInfo(m_part->buildDirectory(), "config.status").exists() ){
		return;
	}

	QString cmdline = "cd ";
	cmdline += KProcess::quote(m_part->projectDirectory());
	cmdline += " && automake ";
	cmdline += KProcess::quote(relmakefile);
	cmdline += " && cd ";
	cmdline += KProcess::quote(m_part->buildDirectory());
	cmdline += " && CONFIG_HEADERS=config.h CONFIG_FILES=";
	cmdline += KProcess::quote(relmakefile);
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
        if( subdirsfile.exists() )
        {
		    if ( subdirsfile.open( IO_ReadOnly ) )
		    {
			    QTextStream subdirsstream( &subdirsfile );
			    while ( !subdirsstream.atEnd() )
				    dirs.append( subdirsstream.readLine() );
			    subdirsfile.close();
		    }
        } else
        {
            QDir d( item->path );
            QStringList l = d.entryList( QDir::Dirs );
            for( QStringList::const_iterator it = l.begin(); it != l.end(); ++it )
            {
                if( (*it) != "CVS" && (*it) != "admin" && (*it) != ".svn" && (*it) != "." && (*it) != ".." )
                {
                    QDir subdir = d;
                    subdir.cd( *it, false );
                    if( subdir.exists( "Makefile.am" ) )
                        dirs.append( *it );
                }
            }

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

	//search for AC_SUBST variables and try to replace them with variables
	//that have been already defined e.g. in a "kdevelop hint"
	varre = QRegExp( "\\@(.*)\\@" );
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
		subdirs.replace( QRegExp( "\\@" + varname + "\\@" ), varvalue );
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

	/// @todo only if in a c++ project
	TargetItem* noinst_HEADERS_item = findNoinstHeaders(item);

	QDir dir( item->path );
	QStringList headersList = QStringList::split( QRegExp("[ \t]"), item->variables[ "noinst_HEADERS" ] );

	headersList += dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tcc", QDir::Files );
	headersList.sort();
    headersList = QStringList::split(QRegExp("[ \t]"), headersList.join( " " ));

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

void AutoSubprojectView::slotForceReeditSubproject( )
{
	SubprojectItem* spitem = dynamic_cast <SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, "force-reedit" );
}

void AutoSubprojectView::slotInstallSubproject( )
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, "install" );
}

void AutoSubprojectView::slotInstallSuSubproject( )
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, "install", true );
}

TargetItem * AutoSubprojectView::findNoinstHeaders( SubprojectItem *item )
{
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

    return noinst_HEADERS_item;
}

void AutoSubprojectView::slotCleanSubproject( )
{
	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );

	m_part->startMakeCommand( m_part->buildDirectory() + relpath, "clean" );
}

void AutoSubprojectView::focusOutEvent( QFocusEvent */* e*/ )
{
    m_widget->setLastFocusedView(AutoProjectWidget::SubprojectView);
}

void AutoSubprojectView::slotManageBuildCommands( )
{
	KConfig *config = m_part->instance()->config();
	//menu item name <-> command
	QMap<QString, QString> customBuildCommands = config->entryMap("CustomCommands");

	KDialogBase dlg(KDialogBase::Plain, i18n("Manage Custom Commands"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok);
	dlg.plainPage()->setMargin(0);
	(new QVBoxLayout(dlg.plainPage(), 0, 0))->setAutoAdd(true);

	ManageCustomCommand *widget = new ManageCustomCommand(dlg.plainPage());

	for (QMap<QString,QString>::const_iterator it = customBuildCommands.constBegin();
		it != customBuildCommands.constEnd(); ++it)
	{
		widget->commandsTable->insertRows(widget->commandsTable->numRows());
		widget->setRowProperties(widget->commandsTable->numRows()-1);
		widget->commandsTable->setText(widget->commandsTable->numRows() - 1, 0, it.key());
		widget->commandsTable->setText(widget->commandsTable->numRows() - 1, 1,
			it.data().section(":::", 0, 0));
		static_cast<QComboTableItem*>(widget->commandsTable->
			item(widget->commandsTable->numRows() - 1, 2))->
			setCurrentItem(it.data().section(":::", 1, 1).toInt());
	}

	widget->commandsTable->setFocus();
	if (dlg.exec() == QDialog::Accepted)
	{
		config->deleteGroup("CustomCommands");
		config->setGroup("CustomCommands");
		for (int i = 0; i < widget->commandsTable->numRows(); ++i)
		{
			config->writeEntry(widget->commandsTable->text(i, 0),
				widget->commandsTable->text(i, 1)+":::"+
				QString("%1").arg(static_cast<QComboTableItem*>(widget->
				commandsTable->item(i, 2))->currentItem()));
		}
		config->sync();
	}

}

void AutoSubprojectView::slotCustomBuildCommand(int val)
{
	QString cmd = m_commandList[val].section(":::", 0, 0);
	int type = m_commandList[val].section(":::", 1, 1).toInt();

	SubprojectItem* spitem = dynamic_cast<SubprojectItem*>( m_listView->selectedItem() );
	if ( !spitem )	return;

	QString relpath = spitem->path.mid( m_part->projectDirectory().length() );
	switch (type)
	{
		case 0: //make target
			m_part->startMakeCommand( m_part->buildDirectory() + relpath, cmd );
			break;
		case 1: //make target as root
			m_part->startMakeCommand( m_part->buildDirectory() + relpath, cmd, true );
			break;
		case 2: //make command
			m_part->startSimpleMakeCommand( m_part->buildDirectory() + relpath, cmd );
			break;
		case 3: //make command as root
			m_part->startSimpleMakeCommand( m_part->buildDirectory() + relpath, cmd, true );
			break;
		case 4: //command
			m_part->appFrontend()->startAppCommand(m_part->buildDirectory() + relpath,
				cmd, false);
			break;
		case 5: //command as root
			m_part->appFrontend()->startAppCommand(m_part->buildDirectory() + relpath,
				"kdesu -t -c ' cd " +
				KProcess::quote(m_part->buildDirectory() + relpath) + " && "
				 + cmd + "'", false);
			break;
	}
}

void AutoSubprojectView::slotExpandTree()
{
	expandCollapseFirst( m_listView->currentItem(), true );
}

void AutoSubprojectView::slotCollapseTree()
{
	expandCollapseFirst( m_listView->currentItem(), false );
}

void AutoSubprojectView::expandCollapseFirst( QListViewItem * item, bool expand )
{
	if ( !item ) return;

	if ( item == m_listView->firstChild() )	// special case for root node
	{
		item = item->firstChild();
		while ( item )
		{
			expandCollapse( item, expand );
			item = item->nextSibling();
		}
	}
	else
	{
		expandCollapse( item, expand );
	}
}

void AutoSubprojectView::expandCollapse( QListViewItem * item, bool expand )
{
	if ( !item ) return;

	item->setOpen( expand );

	item = item->firstChild();
	while ( item )
	{
		expandCollapse( item, expand );
		item = item->nextSibling();
	}
}

#include "autosubprojectview.moc"

// kate: indent-mode csands; tab-width 4; space-indent off;
