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
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
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
#include <kprocess.h>
#include <qtimer.h>
#include <qdir.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <qwhatsthis.h>
#include <qvaluestack.h>
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
#include <kdirwatch.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <kurlcompletion.h>

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
#include "projectconfigurationdlg.h"
#include "qmakescopeitem.h"
#include "scope.h"
#include "createscopedlg.h"
#include "disablesubprojectdlg.h"

TrollProjectWidget::TrollProjectWidget( TrollProjectPart *part )
    : QVBox( 0, "troll project widget" ), m_shownSubproject( 0 ), m_rootSubproject( 0 ),
        m_rootScope ( 0 ), m_part ( part ), m_configDlg( 0 ), m_filesCached(false)
{
    QSplitter * splitter = new QSplitter( Vertical, this );

    //////////////////
    // PROJECT VIEW //
    //////////////////

    overviewContainer = new QVBox( splitter, "Projects" );
    overviewContainer->setMargin ( 2 );
    overviewContainer->setSpacing ( 2 );
    //    overviewContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    //    splitter->setResizeMode(overviewContainer, QSplitter::FollowSizeHint);

    projectTools = new QHBox( overviewContainer, "Project buttons" );
    projectTools->setMargin ( 2 );
    projectTools->setSpacing ( 2 );
    // Add subdir
    addSubdirButton = new QToolButton ( projectTools, "Add subproject button" );
    addSubdirButton->setPixmap ( SmallIcon ( "folder_new" ) );
    addSubdirButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, addSubdirButton->sizePolicy().hasHeightForWidth() ) );
    addSubdirButton->setEnabled ( true );
    QToolTip::add( addSubdirButton, i18n( "Add subproject" ) );
    QWhatsThis::add( addSubdirButton, i18n( "<b>Add subproject</b><p>Creates a <i>new</i> or adds an <i>existing</i> subproject to a currently selected subproject. "
                                                "This action is allowed only if a type of the subproject is 'subdirectories'. The type of the subproject can be "
                                                "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)." ) );
    // Create scope
    createScopeButton = new QToolButton ( projectTools, "Create scope button" );
    createScopeButton->setPixmap ( SmallIcon ( "qmake_scopenew" ) );
    createScopeButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, createScopeButton->sizePolicy().hasHeightForWidth() ) );
    createScopeButton->setEnabled ( true );
    QToolTip::add( createScopeButton, i18n( "Create scope" ) );
    QWhatsThis::add( createScopeButton, i18n( "<b>Create scope</b><p>Creates QMake scope in the project file in case the subproject is selected or creates nested scope in case the scope is selected." ) );

    // build
    buildProjectButton = new QToolButton ( projectTools, "Make button" );
    buildProjectButton->setPixmap ( SmallIcon ( "make_kdevelop" ) );
    buildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, buildProjectButton->sizePolicy().hasHeightForWidth() ) );
    buildProjectButton->setEnabled ( true );
    QToolTip::add( buildProjectButton, i18n( "Build project" ) );
    QWhatsThis::add( buildProjectButton, i18n( "<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                         "Environment variables and make arguments can be specified "
                         "in the project settings dialog, <b>Make Options</b> tab." ) );
    // rebuild
    rebuildProjectButton = new QToolButton ( projectTools, "Rebuild button" );
    rebuildProjectButton->setPixmap ( SmallIcon ( "rebuild" ) );
    rebuildProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, rebuildProjectButton->sizePolicy().hasHeightForWidth() ) );
    rebuildProjectButton->setEnabled ( true );
    QToolTip::add( rebuildProjectButton, i18n( "Rebuild project" ) );
    QWhatsThis::add( rebuildProjectButton, i18n( "<b>Rebuild project</b><p>Runs <b>make clean</b> and then <b>make</b> from the project directory.<br>"
                         "Environment variables and make arguments can be specified "
                         "in the project settings dialog, <b>Make Options</b> tab." ) );
    // run
    executeProjectButton = new QToolButton ( projectTools, "Run button" );
    executeProjectButton->setPixmap ( SmallIcon ( "exec" ) );
    executeProjectButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, executeProjectButton->sizePolicy().hasHeightForWidth() ) );
    executeProjectButton->setEnabled ( true );
    QToolTip::add( executeProjectButton, i18n( "Execute main program" ) );
    QWhatsThis::add( executeProjectButton, i18n( "<b>Execute main program</b><p>Executes the main program specified in project settings, <b>Run Options</b> tab." ) );
    // spacer
    QWidget *spacer = new QWidget( projectTools );
    projectTools->setStretchFactor( spacer, 1 );
    // Project configuration
    projectconfButton = new QToolButton ( projectTools, "Project configuration button" );
    projectconfButton->setPixmap ( SmallIcon ( "configure" ) );
    projectconfButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, projectconfButton->sizePolicy().hasHeightForWidth() ) );
    projectconfButton->setEnabled ( true );
    QToolTip::add( projectconfButton, i18n( "Subproject settings" ) );
    QWhatsThis::add( projectconfButton, i18n( "<b>Subproject settings</b><p>Opens <b>QMake Subproject Configuration</b> dialog for the currently selected subproject. "
                         "It provides settings for:<br>subproject type and configuration,<br>include and library paths,<br>lists of dependencies and "
                         "external libraries,<br>build order,<br>intermediate files locations,<br>compiler options." ) );

    // Project button connections
    connect ( addSubdirButton, SIGNAL ( clicked () ), this, SLOT ( slotAddSubproject () ) );
    connect ( createScopeButton, SIGNAL ( clicked () ), this, SLOT ( slotCreateScope () ) );


    connect ( buildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildProject () ) );
    connect ( rebuildProjectButton, SIGNAL ( clicked () ), this, SLOT ( slotRebuildProject () ) );
    connect ( executeProjectButton, SIGNAL ( clicked () ), m_part, SLOT ( slotBuildAndExecuteProject () ) );



    connect ( projectconfButton, SIGNAL ( clicked () ), this, SLOT ( slotConfigureProject () ) );

    // Project tree
    overview = new TrollListView( this, overviewContainer, SubprojectView, "project overview widget" );
//     overview->setResizeMode( QListView::LastColumn );
    overview->setSorting( -1 );
    overview->header() ->hide();
    overview->addColumn( QString::null );

    // Project tree connections
    connect( overview, SIGNAL( selectionChanged( QListViewItem* ) ),
             this, SLOT( slotOverviewSelectionChanged( QListViewItem* ) ) );
    connect( overview, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
             this, SLOT( slotOverviewContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );


    /////////////////
    // DETAIL VIEW //
    /////////////////

    // Details tree
    detailContainer = new QVBox( splitter, "Details" );
    detailContainer->setMargin ( 2 );
    detailContainer->setSpacing ( 2 );
    //    detailContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // Details Toolbar
    fileTools = new QHBox( detailContainer, "Detail buttons" );
    fileTools->setMargin ( 2 );
    fileTools->setSpacing ( 2 );

    // Add new file button
    newfileButton = new QToolButton ( fileTools, "Create new file" );
    newfileButton->setPixmap ( SmallIcon ( "filenew" ) );
    newfileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, newfileButton->sizePolicy().hasHeightForWidth() ) );
    newfileButton->setEnabled ( true );
    QToolTip::add( newfileButton, i18n( "Create new file" ) );
    QWhatsThis::add( newfileButton, i18n( "<b>Create new file</b><p>Creates a new file and adds it to a currently selected group." ) );

    // Add existing files button
    addfilesButton = new QToolButton ( fileTools, "Add existing files" );
    addfilesButton->setPixmap ( SmallIcon ( "fileimport" ) );
    addfilesButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, addfilesButton->sizePolicy().hasHeightForWidth() ) );
    addfilesButton->setEnabled ( true );
    QToolTip::add( addfilesButton, i18n( "Add existing files" ) );
    QWhatsThis::add( addfilesButton, i18n( "<b>Add existing files</b><p>Adds existing files to a currently selected group. It is "
                                               "possible to copy files to a current subproject directory, create symbolic links or "
                                               "add them with the relative path." ) );

    // remove file button
    removefileButton = new QToolButton ( fileTools, "Remove file" );
    removefileButton->setPixmap ( SmallIcon ( "button_cancel" ) );
    removefileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, removefileButton->sizePolicy().hasHeightForWidth() ) );
    removefileButton->setEnabled ( true );
    QToolTip::add( removefileButton, i18n( "Remove file" ) );
    QWhatsThis::add( removefileButton, i18n( "<b>Remove file</b><p>Removes file from a current group. Does not remove file from disk." ) );

    // build selected file
    buildFileButton = new QToolButton ( fileTools, "Make file button" );
    buildFileButton->setPixmap ( SmallIcon ( "compfile" ) );
    buildFileButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, buildFileButton->sizePolicy().hasHeightForWidth() ) );
    buildFileButton->setEnabled ( true );
    QToolTip::add( buildFileButton, i18n( "Compile file" ) );
    QWhatsThis::add( buildFileButton, i18n( "<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                                                "Environment variables and make arguments can be specified "
                                                "in the project settings dialog, <b>Make Options</b> tab." ) );

    // build
    buildTargetButton = new QToolButton ( fileTools, "Make sp button" );
    buildTargetButton->setPixmap ( SmallIcon ( "make_kdevelop" ) );
    buildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, buildTargetButton->sizePolicy().hasHeightForWidth() ) );
    buildTargetButton->setEnabled ( true );
    QToolTip::add( buildTargetButton, i18n( "Build subproject" ) );
    QWhatsThis::add( buildTargetButton, i18n( "<b>Build subproject</b><p>Runs <b>make</b> from the current subproject directory. "
                         "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                         "Environment variables and make arguments can be specified "
                         "in the project settings dialog, <b>Make Options</b> tab." ) );
    // rebuild
    rebuildTargetButton = new QToolButton ( fileTools, "Rebuild sp button" );
    rebuildTargetButton->setPixmap ( SmallIcon ( "rebuild" ) );
    rebuildTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, rebuildTargetButton->sizePolicy().hasHeightForWidth() ) );
    rebuildTargetButton->setEnabled ( true );
    QToolTip::add( rebuildTargetButton, i18n( "Rebuild subproject" ) );
    QWhatsThis::add( rebuildTargetButton, i18n( "<b>Rebuild subproject</b><p>Runs <b>make clean</b> and then <b>make</b> from the current subproject directory. "
                         "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                         "Environment variables and make arguments can be specified "
                         "in the project settings dialog, <b>Make Options</b> tab." ) );
    // run
    executeTargetButton = new QToolButton ( fileTools, "Run sp button" );
    executeTargetButton->setPixmap ( SmallIcon ( "exec" ) );
    executeTargetButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, executeTargetButton->sizePolicy().hasHeightForWidth() ) );
    executeTargetButton->setEnabled ( true );
    QToolTip::add( executeTargetButton, i18n( "Execute subproject" ) );
    QWhatsThis::add( executeTargetButton, i18n( "<b>Execute subproject</b><p>Executes the target program for the currently selected subproject. "
                         "This action is allowed only if a type of the subproject is 'application'. The type of the subproject can be "
                         "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)." ) );


    // spacer
    spacer = new QWidget( fileTools );
    projectTools->setStretchFactor( spacer, 1 );

    // Configure file button
    excludeFileFromScopeButton = new QToolButton ( fileTools, "Exclude file" );
    excludeFileFromScopeButton->setPixmap ( SmallIcon ( "configure_file" ) );
    excludeFileFromScopeButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, excludeFileFromScopeButton->sizePolicy().hasHeightForWidth() ) );
    excludeFileFromScopeButton->setEnabled ( true );
    QToolTip::add( excludeFileFromScopeButton , i18n( "Exclude file" ) );
    QWhatsThis::add( excludeFileFromScopeButton , i18n( "<b>Exclude file</b><p>Exclude the selected file from this scope." ) );

    // detail tree
    details = new TrollListView( this, detailContainer, DetailsView, "details widget" );
    details->setRootIsDecorated( true );
    details->setResizeMode( QListView::LastColumn );
    details->setSorting( -1 );
    details->header() ->hide();
    details->addColumn( QString::null );
    // Detail button connections
    connect ( addfilesButton, SIGNAL ( clicked () ), this, SLOT ( slotAddFiles () ) );
    connect ( newfileButton, SIGNAL ( clicked () ), this, SLOT ( slotNewFile () ) );
    connect ( removefileButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveFile () ) );
    connect ( buildFileButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildSelectedFile () ) );
    connect ( excludeFileFromScopeButton, SIGNAL ( clicked () ), this, SLOT ( slotExcludeFileFromScopeButton() ) );

    // Detail tree connections
    connect( details, SIGNAL( selectionChanged( QListViewItem* ) ),
             this, SLOT( slotDetailsSelectionChanged( QListViewItem* ) ) );
    connect( details, SIGNAL( executed( QListViewItem* ) ),
             this, SLOT( slotDetailsExecuted( QListViewItem* ) ) );
    connect( details, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
             this, SLOT( slotDetailsContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );

    connect ( buildTargetButton, SIGNAL ( clicked () ), this, SLOT ( slotBuildTarget () ) );
    connect ( rebuildTargetButton, SIGNAL ( clicked () ), this, SLOT ( slotRebuildTarget () ) );
    connect ( executeTargetButton, SIGNAL ( clicked () ), m_part, SLOT ( slotBuildAndExecuteTarget () ) );
    buildTargetButton->setEnabled( false );
    rebuildTargetButton->setEnabled( false );
    executeTargetButton->setEnabled( false );

    m_configDlg = new ProjectConfigurationDlg( overview, this, this );

    connect( m_part->dirWatch(), SIGNAL( dirty(const QString&) ), this, SLOT( slotProjectDirty(const QString&) ) );
}


TrollProjectWidget::~TrollProjectWidget()
{
    delete m_configDlg;
}


void TrollProjectWidget::openProject( const QString &dirName )
{
    QDomDocument & dom = *( m_part->projectDom() );
    m_subclasslist = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                     "subclass", "sourcefile", "uifile" );
    QFileInfo fi( dirName );
    QDir dir( dirName );
    //    QString proname = item->path + "/" + fi.baseName() + ".pro";

    QStringList l = dir.entryList( "*.pro" );

    QString profile;
    if( !l.count() || (l.count() && l.findIndex( fi.baseName() + ".pro") != -1  ) )
        profile = fi.baseName()+".pro";
    else
      profile = l[0];

    QString proname = dirName + QString( QChar( QDir::separator() ) ) + profile;

    kdDebug( 9024 ) << "Parsing " << proname << endl;

    m_rootScope = new Scope( proname, m_part );

    m_rootSubproject = new QMakeScopeItem( overview, m_rootScope->scopeName(), m_rootScope, this );

    m_rootSubproject->setOpen( true );
    if ( m_rootSubproject->firstChild() && m_rootSubproject->scope->variableValues( "TEMPLATE" ).findIndex("subdirs") != -1 )
    {
        overview->setSelected( m_rootSubproject->firstChild(), true );
    }
    else
    {
        overview->setSelected( m_rootSubproject, true );
    }
//     kdDebug(9024) << "Adding " << allFiles().count() << " Files" << endl;
//     kdDebug(9024) << allFiles() << endl;

}

void TrollProjectWidget::createQMakeScopeItems()
{

}

void TrollProjectWidget::closeProject()
{
    m_rootSubproject = 0;
    overview->clear();
    details->clear();
    delete m_rootScope;
}

QStringList TrollProjectWidget::allFiles()
{
    if( m_filesCached )
        return m_allFilesCache;
    m_allFilesCache = m_rootScope->allFiles( m_rootScope->projectDir() );
    m_filesCached = true;
    return m_allFilesCache;
}

QString TrollProjectWidget::projectDirectory()
{
    if ( !m_rootScope )
        return QString::null; //confused

    return m_rootScope->projectDir();
}


QString TrollProjectWidget::subprojectDirectory()
{
    if ( !m_shownSubproject )
        return QString::null;

    return m_shownSubproject->scope->projectDir();
}

void TrollProjectWidget::setupContext()
{
    if ( !m_shownSubproject )
        return ;
    bool buildable = true;
    bool runable = true;
    bool fileconfigurable = true;
    bool hasSourceFiles = true;
    bool hasSubdirs = false;

    QStringList tmpl = m_shownSubproject->scope->variableValues( "TEMPLATE" );

    if ( tmpl.findIndex( "lib" ) != -1 )
    {
        runable = false;
    }
    else if ( tmpl.findIndex( "subdirs" ) != -1 )
    {
        hasSubdirs = true;
        runable = false;
        hasSourceFiles = false;
        fileconfigurable = false;
    }
    if ( m_shownSubproject->scope->scopeType() != Scope::ProjectScope )
    {
        runable = false;
        buildable = false;
    }


    // Setup toolbars according to context
    addSubdirButton->setEnabled( hasSubdirs );
    buildTargetButton->setEnabled( buildable );
    m_part->actionCollection() ->action( "build_build_target" ) ->setEnabled( buildable );

    rebuildTargetButton->setEnabled( buildable );
    m_part->actionCollection() ->action( "build_rebuild_target" ) ->setEnabled( buildable );

    executeTargetButton->setEnabled( runable );
    m_part->actionCollection() ->action( "build_execute_target" ) ->setEnabled( runable );

    excludeFileFromScopeButton->setEnabled( !hasSubdirs );
    newfileButton->setEnabled( !hasSubdirs );
    removefileButton->setEnabled( !hasSubdirs );
    addfilesButton->setEnabled( !hasSubdirs );
    buildFileButton->setEnabled( !hasSubdirs );

    details->setEnabled( hasSourceFiles );
}

void TrollProjectWidget::slotOverviewSelectionChanged( QListViewItem *item )
{
    if ( !item )
    {
        kdDebug(9024) << "Trying to select a non-existing item" << endl;
        return ;
    }
    cleanDetailView( m_shownSubproject );
    m_shownSubproject = static_cast<QMakeScopeItem*>( item );
    setupContext();
    buildProjectDetailTree( m_shownSubproject, details );

    QDomDocument &dom = *( m_part->projectDom() );
    DomUtil::writeEntry( dom, "/kdevtrollproject/general/activedir", m_shownSubproject->relativePath() );
    if ( m_configDlg && m_configDlg->isShown() )
    {
        m_configDlg->updateSubproject( m_shownSubproject );
    }
}

QString TrollProjectWidget::getCurrentTarget()
{
    if ( !m_shownSubproject )
        return "";
    QString destdir = m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->variableValues( "DESTDIR" ).front());
    if ( destdir.isEmpty() )
        return destdir + m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->variableValues( "TARGET" ).front());
    else
        return destdir + QString( QChar( QDir::separator() ) ) + m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->variableValues( "TARGET" ).front());
}

QString TrollProjectWidget::getCurrentDestDir()
{
    if ( !m_shownSubproject )
        return "";
    return m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->variableValues( "DESTDIR" ).front());
}

QString TrollProjectWidget::getCurrentOutputFilename()
{
    if ( !m_shownSubproject )
        return "";
    if ( m_shownSubproject->scope->variableValues( "TARGET" ).isEmpty() )
    {
        QString exe = m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->fileName());
        return exe.replace( QRegExp( "\\.pro$" ), "" );
    }
    else
        return m_shownSubproject->scope->resolveVariables(m_shownSubproject->scope->variableValues( "TARGET" ).front());
}

void TrollProjectWidget::cleanDetailView( QMakeScopeItem *item )
{
    // If no children in detailview
    // it is a subdir template
    if ( item && details->childCount() )
    {
        QListViewItem* i = details->firstChild();
        while( i )
        {
            QListViewItem* old = i;
            i = i->nextSibling();
            details->takeItem(old);
        }
//         QMapIterator<GroupItem::GroupType, GroupItem*> it1 = item->groups.begin() ;
//         for ( ; it1 != item->groups.end(); ++it1 )
//         {
//             // After AddTargetDialog, it can happen that an
//             // item is not yet in the list view, so better check...
//             if ( it1.data() ->parent() )
//                 while ( it1.data() ->firstChild() )
//                     it1.data() ->takeItem( it1.data() ->firstChild() );
//             details->takeItem( it1.data() );
//         }
    }
}

void TrollProjectWidget::buildProjectDetailTree( QMakeScopeItem *item, KListView *listviewControl )
{

    // Insert all GroupItems and all of their children into the view
    if ( !listviewControl || item->scope->variableValues( "TEMPLATE" ).findIndex("subdirs") != -1 )
        return ;

    QMapIterator<GroupItem::GroupType, GroupItem*> it2 = item->groups.begin();
    QListViewItem* lastItem = 0;
    for ( ; it2 != item->groups.end(); ++it2 )
    {
        listviewControl->insertItem( it2.data() );
        if(lastItem)
            it2.data()->moveItem(lastItem);
        lastItem = it2.data();
        if ( it2.key() == GroupItem::InstallRoot )
        {
            QListViewItem* lastinstallitem = 0;
            QPtrListIterator<GroupItem> it3( it2.data() ->installs );
            for ( ; it3.current(); ++it3 )
            {
                it2.data() ->insertItem( *it3 );
                if ( lastinstallitem )
                    it3.current()->moveItem(lastinstallitem);
                lastinstallitem = it3.current();
                QPtrListIterator<FileItem> it4( ( *it3 ) ->files );
                QListViewItem* lastfileitem = 0;
                for ( ; it4.current(); ++it4 )
                {
                    ( *it3 ) ->insertItem( *it4 );
                    if ( lastfileitem )
                        it4.current()->moveItem(lastfileitem);
                    lastfileitem = it4.current();
                }
                ( *it3 ) ->setOpen( true );
                ( *it3 ) ->sortChildItems( 0, true );
            }
            it2.data() ->setOpen( true );
            it2.data() ->sortChildItems( 0, true );
        }
        else
        {
            QPtrListIterator<FileItem> it3( it2.data() ->files );
            QListViewItem* lastfileitem = 0;
            for ( ; it3.current(); ++it3 )
            {
                it2.data() ->insertItem( *it3 );
                if ( lastfileitem )
                    it3.current()->moveItem(lastfileitem);
                lastfileitem = it3.current();
            }
            it2.data() ->setOpen( true );
            it2.data() ->sortChildItems( 0, true );
        }
    }
    listviewControl->setSelected( listviewControl->selectedItem(), false );
    listviewControl->setCurrentItem( 0 );
}

void TrollProjectWidget::slotDetailsExecuted( QListViewItem *item )
{
    if ( !item )
        return ;

    // We assume here that ALL items in both list views
    // are qProjectItem's
    qProjectItem *pvitem = static_cast<qProjectItem*>( item );
    if ( pvitem->type() != qProjectItem::File )
        return ;

    QString dirName = m_shownSubproject->scope->projectDir();
    FileItem *fitem = static_cast<FileItem*>( pvitem );

    bool isUiFile = QFileInfo( fitem->text( 0 ) ).extension() == "ui";
    kdDebug(9024) << "Opening file: " << dirName + "/" + m_shownSubproject->scope->resolveVariables( fitem->text( 0 ) ) << endl;
    if ( isTMakeProject() && isUiFile )
    {
        // start designer in your PATH
        KShellProcess proc;
        proc << "designer" << ( dirName + QString( QChar( QDir::separator() ) ) + QString( fitem->text( 0 ) ) );
        proc.start( KProcess::DontCare, KProcess::NoCommunication );

    }
    else
        m_part->partController() ->editDocument( KURL( dirName + QString( QChar( QDir::separator() ) ) + QString( m_shownSubproject->scope->resolveVariables( fitem->text( 0 ) ) ) ) );
}


void TrollProjectWidget::slotConfigureProject()
{
    m_configDlg->updateSubproject( m_shownSubproject );
    m_configDlg->show();
}

void TrollProjectWidget::slotExecuteTarget()
{
    //m_part->slotExecute();

    // no subproject selected
    if ( !m_shownSubproject )
        return ;

    // can't build from scope
    if ( m_shownSubproject->scope->scopeType() != Scope::ProjectScope )
        return ;


    // Only run application projects
    if ( m_shownSubproject->scope->variableValues( "TEMPLATE" ).findIndex( "app" ) == -1 )
        return ;

    QString dircmd = "cd " + KProcess::quote( subprojectDirectory() + QString( QChar( QDir::separator() ) ) + getCurrentDestDir() ) + " && ";
    QString program = KProcess::quote( "." + QString( QChar( QDir::separator() ) ) + getCurrentOutputFilename() );

    // Build environment variables to prepend to the executable path
    QString runEnvVars = QString::null;
    DomUtil::PairList list =
        DomUtil::readPairListEntry( *( m_part->projectDom() ), "/kdevtrollproject/run/envvars", "envvar", "name", "value" );

    DomUtil::PairList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
    {
        const DomUtil::Pair &pair = ( *it );
        if ( ( !pair.first.isEmpty() ) && ( !pair.second.isEmpty() ) )
            runEnvVars += pair.first + "=" + pair.second + " ";
    }
    program.prepend( runEnvVars );

    program.append( " " + DomUtil::readEntry( *( m_part->projectDom() ), "/kdevtrollproject/run/programargs" ) + " " );
    //  std::cerr<<dircmd + "./"+program<<std::endl;
    //  m_part->execute(dircmd + "./"+program);
    //  m_part->appFrontend()->startAppCommand(dircmd +"./"+program,true);

    bool inTerminal = DomUtil::readBoolEntry( *m_part->projectDom(), "/kdevtrollproject/run/terminal" );
    m_part->appFrontend() ->startAppCommand( subprojectDirectory() + QString( QChar( QDir::separator() ) ) + getCurrentDestDir(), program, inTerminal );

}

void TrollProjectWidget::slotBuildProject()
{
    if ( m_part->partController() ->saveAllFiles() == false )
        return ; //user cancelled

    QString dir = projectDirectory();

    if ( !m_rootSubproject )
        return ;

    createMakefileIfMissing( dir, m_rootSubproject );

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    QString dircmd = "cd " + KProcess::quote( dir ) + " && " ;
    QString buildcmd = constructMakeCommandLine( m_rootSubproject->scope );
    m_part->queueCmd( dir, dircmd + buildcmd );
}
void TrollProjectWidget::slotBuildTarget()
{
    // no subproject selected
    m_part->partController() ->saveAllFiles();
    if ( !m_shownSubproject )
        return ;
    // can't build from scope
    if ( m_shownSubproject->scope->scopeType() != Scope::ProjectScope )
        return ;
    QString dir = subprojectDirectory();
    createMakefileIfMissing( dir, m_shownSubproject );

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    QString dircmd = "cd " + KProcess::quote( dir ) + " && " ;
    QString buildcmd = constructMakeCommandLine( m_shownSubproject->scope );
    m_part->queueCmd( dir, dircmd + buildcmd );
}

void TrollProjectWidget::slotRebuildProject()
{
    m_part->partController() ->saveAllFiles();
    QString dir = this-> projectDirectory();

    if ( !m_rootSubproject )
        return ;

    createMakefileIfMissing( dir, m_rootSubproject );

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    QString dircmd = "cd " + KProcess::quote( dir ) + " && " ;
    QString rebuildcmd = constructMakeCommandLine( m_rootSubproject->scope ) + " clean && " + constructMakeCommandLine( m_rootSubproject->scope );
    m_part->queueCmd( dir, dircmd + rebuildcmd );
}

void TrollProjectWidget::slotRebuildTarget()
{
    // no subproject selected
    m_part->partController() ->saveAllFiles();
    if ( !m_shownSubproject )
        return ;
    // can't build from scope
    if ( m_shownSubproject->scope->scopeType() != Scope::ProjectScope )
        return ;

    QString dir = subprojectDirectory();
    createMakefileIfMissing( dir, m_shownSubproject );

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    QString dircmd = "cd " + KProcess::quote( dir ) + " && " ;
    QString rebuildcmd = constructMakeCommandLine( m_shownSubproject->scope ) + " clean && " + constructMakeCommandLine( m_shownSubproject->scope );
    m_part->queueCmd( dir, dircmd + rebuildcmd );
}

void TrollProjectWidget::slotCreateScope( QMakeScopeItem *spitem )
{
    if ( spitem == 0 && m_shownSubproject == 0 )
        return ;
    else
        spitem = m_shownSubproject;
    CreateScopeDlg dlg( spitem, this );
    if ( dlg.exec() == QDialog::Accepted )
    {
        spitem->scope->saveToFile( );
        spitem->sortChildItems( 0, true );
    }
    return ;
}

void TrollProjectWidget::slotAddSubproject( QMakeScopeItem *spitem )
{
    if ( spitem == 0 && m_shownSubproject == 0 )
        return ;
    else
        spitem = m_shownSubproject;

    m_filesCached = false;
    m_allFilesCache.clear();

    QString projectdir = spitem->scope->projectDir();

    KURLRequesterDlg dialog( i18n( "Add Subproject" ), i18n( "Please enter a name for the subproject: " ), this, 0 );
    dialog.urlRequester() ->setMode( KFile::Directory | KFile::Files | KFile::LocalOnly );
    dialog.urlRequester() ->setURL( QString::null );
    dialog.urlRequester() ->completionObject() ->setDir( projectdir );

    if ( dialog.exec() == QDialog::Accepted && !dialog.urlRequester() ->url().isEmpty() )
    {
        QString subdirname;
        if ( !QDir::isRelativePath( dialog.urlRequester() ->url() ) )
            subdirname = getRelativePath( projectdir, dialog.urlRequester()->url() );
        else
            subdirname = dialog.urlRequester()->url();

        while( subdirname.endsWith( QString(QChar(QDir::separator())) ) )
            subdirname = subdirname.left(subdirname.length()-1);
        if( !subdirname.endsWith(".pro") )
        {
            kdDebug(9024) << "Cleaned subdirname: " << subdirname << endl;
            QDir dir( projectdir );
            QString realdir = spitem->scope->resolveVariables( subdirname );
            if ( !dir.exists( realdir ) )
            {
                if ( !dir.mkdir( realdir ) )
                {
                    KMessageBox::error( this, i18n( "Failed to create subdirectory. "
                                                "Do you have write permission "
                                                "in the project folder?" ) );
                    return ;
                }
            }
        }

        addSubprojectToItem( spitem, subdirname );

    }
}

void TrollProjectWidget::addSubprojectToItem( QMakeScopeItem* spitem, const QString& subdirname )
{
    QListViewItem* item = spitem->firstChild();
    while( item )
    {
        QMakeScopeItem* sitem = static_cast<QMakeScopeItem*>(item);
        if( sitem->scope->scopeName() == subdirname )
        {
            if( sitem->scope->isEnabled() )
            {
                return;
            }else
            {
                spitem->scope->removeFromMinusOp( "SUBDIRS", subdirname );
                delete item;
                if( spitem->scope->variableValues( "SUBDIRS" ).findIndex( subdirname ) != -1 )
                    return;
            }
        }
        item = item->nextSibling();
    }

    Scope* subproject = spitem->scope->createSubProject( subdirname );
    if( subproject )
    {
        new QMakeScopeItem( spitem, subproject->scopeName(), subproject );
//             QListViewItem* lastitem = spitem->firstChild();
//             while( lastitem->nextSibling() != 0 )
//                 lastitem = lastitem->nextSibling();
//             newitem->moveItem( lastitem );
    }else
    {
        KMessageBox::error(this, i18n("Couldn't create subproject. This means that either the project you wanted"
                                      " to add a subproject isn't parsed correctly or it's not a"
                                      " subdirs-project."), i18n("Subproject creation failed") );
    }
    spitem->scope->saveToFile();
    spitem->sortChildItems( 0, true );
}

void TrollProjectWidget::slotRemoveSubproject( QMakeScopeItem *spitem )
{
    if ( spitem == 0 && m_shownSubproject == 0 )
        return ;
    else if ( ( spitem = dynamic_cast<QMakeScopeItem *>( m_shownSubproject->parent() ) ) != NULL )
    {

        m_filesCached = false;
        m_allFilesCache.clear();

        bool delsubdir = false;
        if ( KMessageBox::warningYesNo( this, i18n( "Delete the directory of the subproject from disk?" ), i18n( "Delete subdir?" ) ) == KMessageBox::Yes )
            delsubdir = true;
        if( !spitem->scope->deleteSubProject( m_shownSubproject->scope->getNum(), delsubdir ) )
        {
            KMessageBox::error(this, i18n("Couldn't delete subproject.\nThis is an internal error, please write a"
                                          " bugreport to bugs.kde.org and include the output of kdevelop when run"
                                          "from a shell."),i18n("Subproject Deletion failed"));
            return;
        }
        delete m_shownSubproject;
        m_shownSubproject = spitem;
        spitem->scope->saveToFile( );
        overview->setCurrentItem( m_shownSubproject );
        overview->setSelected( m_shownSubproject, true );
    }
}

void TrollProjectWidget::slotOverviewContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
    if ( !item )
        return ;

    QMakeScopeItem *spitem = static_cast<QMakeScopeItem*>( item );

    KPopupMenu popup( this );
    popup.insertTitle( i18n( "Subproject %1" ).arg( item->text( 0 ) ) );

    int idBuild = -2;
    int idRebuild = -2;
    int idClean = -2;
    int idDistClean = -2;
    int idQmake = -2;
    int idQmakeRecursive = -2;
    int idProjectConfiguration = -2;
    int idAddSubproject = -2;
    int idRemoveSubproject = -2;
    int idDisableSubproject = -2;
    int idRemoveScope = -2;
    int idAddScope = -2;


    if ( spitem->scope->scopeType() == Scope::ProjectScope && ( !spitem->scope->parent() || spitem->scope->parent()->scopeType() == Scope::ProjectScope ) )
    {
        idBuild = popup.insertItem( SmallIcon( "make_kdevelop" ), i18n( "Build" ) );
        popup.setWhatsThis( idBuild, i18n( "<b>Build</b><p>Runs <b>make</b> from the selected subproject directory.<br>"
                                           "Environment variables and make arguments can be specified "
                                           "in the project settings dialog, <b>Make Options</b> tab." ) );
        idClean = popup.insertItem( i18n( "Clean" ) );
        popup.setWhatsThis( idBuild, i18n( "<b>Clean project</b><p>Runs <b>make clean</b> command from the project "
                                           "directory.<br> Environment variables and make arguments can be specified "
                                           "in the project settings dialog, <b>Make Options</b> tab." ) );
        idDistClean = popup.insertItem( i18n( "Dist-Clean" ) );
        popup.setWhatsThis( idBuild, i18n( "<b>Dist-Clean project</b><p>Runs <b>make distclean</b> command from the project "
                                           "directory.<br> Environment variables and make arguments can be specified "
                                           "in the project settings dialog, <b>Make Options</b> tab." ) );

        idRebuild = popup.insertItem( SmallIcon( "rebuild" ), i18n( "Rebuild" ) );
        popup.setWhatsThis( idRebuild, i18n( "<b>Rebuild project</b><p>Runs <b>make clean</b> and then <b>make</b> from "
                                             "the project directory.<br>Environment variables and make arguments can be "
                                             "specified in the project settings dialog, <b>Make Options</b> tab." ) );
        idQmake = popup.insertItem( SmallIcon( "qmakerun" ), i18n( "Run qmake" ) );
        popup.setWhatsThis( idQmake, i18n( "<b>Run qmake</b><p>Runs <b>qmake</b> from the selected subproject directory. This creates or regenerates Makefile." ) );
        idQmakeRecursive = popup.insertItem( SmallIcon( "qmakerun" ), i18n( "Run qmake recursively" ) );
        popup.setWhatsThis( idQmakeRecursive, i18n( "<b>Run qmake recursively</b><p>Runs <b>qmake</b> from the selected"
                            "subproject directory and recurses into all subproject directories. "
                            "This creates or regenerates Makefile." ) );

        popup.insertSeparator();
        idAddSubproject = popup.insertItem( SmallIcon( "folder_new" ), i18n( "Add Subproject..." ) );
        popup.setWhatsThis( idAddSubproject, i18n( "<b>Add subproject</b><p>Creates a <i>new</i> or adds an <i>existing</i> subproject to a currently selected subproject. "
                            "This action is allowed only if a type of the subproject is 'subdirectories'. The type of the subproject can be "
                            "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)." ) );
        if ( spitem->scope->variableValues( "TEMPLATE" ).findIndex( "subdirs" ) == -1 )
            popup.setItemEnabled( idAddSubproject, false );
        idRemoveSubproject = popup.insertItem( SmallIcon( "remove_subdir" ), i18n( "Remove Subproject..." ) );
        popup.setWhatsThis( idRemoveSubproject, i18n( "<b>Remove subproject</b><p>Removes currently selected subproject. Does not delete any file from disk. Deleted subproject can be later added by calling 'Add Subproject' action." ) );
        if ( spitem->parent() == NULL )
            popup.setItemEnabled( idRemoveSubproject, false );
        idAddScope = popup.insertItem( SmallIcon( "qmake_scopenew" ), i18n( "Create Scope..." ) );
        popup.setWhatsThis( idAddScope, i18n( "<b>Create scope</b><p>Creates QMake scope in the project file of the currently selected subproject." ) );
        popup.insertSeparator();
        idProjectConfiguration = popup.insertItem( SmallIcon( "configure" ), i18n( "Subproject Settings" ) );
        popup.setWhatsThis( idProjectConfiguration, i18n( "<b>Subproject settings</b><p>Opens <b>QMake Subproject Configuration</b> dialog. "
                            "It provides settings for:<br>subproject type and configuration,<br>include and library paths,<br>lists of dependencies and "
                            "external libraries,<br>build order,<br>intermediate files locations,<br>compiler options." ) );
    }
    else
    {
        idAddScope = popup.insertItem( SmallIcon( "qmake_scopenew" ), i18n( "Create Scope..." ) );
        popup.setWhatsThis( idAddScope, i18n( "<b>Create Scope</b><p>Creates QMake scope in the currently selected scope." ) );
        idRemoveScope = popup.insertItem( SmallIcon( "editdelete" ), i18n( "Remove Scope" ) );
        popup.setWhatsThis( idRemoveScope, i18n( "<b>Remove Scope</b><p>Removes currently selected scope." ) );
        popup.insertSeparator();
        idAddSubproject = popup.insertItem( SmallIcon( "folder_new" ), i18n( "Add Subproject..." ) );
        popup.setWhatsThis( idAddSubproject, i18n( "<b>Add subproject</b><p>Creates a <i>new</i> or adds an <i>existing</i> subproject to the currently selected scope. "
                            "This action is allowed only if a type of the subproject is 'subdirectories'. The type of the subproject can be "
                            "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)." ) );
        if ( spitem->scope->variableValues( "TEMPLATE" ).findIndex( "subdirs" ) == -1 )
            popup.setItemEnabled( idAddSubproject, false );
        idDisableSubproject = popup.insertItem( SmallIcon( "remove_subdir" ), i18n( "Disable Subproject..." ) );
        popup.setWhatsThis( idRemoveSubproject, i18n( "<b>Disable subproject</b><p>Disables the currently selected subproject when this scope is active. Does not delete the directory from disk. Deleted subproject can be later added by calling 'Add Subproject' action." ) );
        if( spitem->scope->variableValues( "TEMPLATE" ).findIndex( "subdirs" ) == -1 && spitem->scope->parent()->variableValues( "TEMPLATE" ).findIndex( "subdirs" ) == -1 )
            popup.setItemEnabled( idDisableSubproject, false );
        popup.insertSeparator();
        idProjectConfiguration = popup.insertItem( SmallIcon( "configure" ), i18n( "Scope Settings" ) );
        popup.setWhatsThis( idProjectConfiguration, i18n( "<b>Scope settings</b><p>Opens <b>QMake Subproject Configuration</b> dialog. "
                            "It provides settings for:<br>subproject type and configuration,<br>include and library paths,<br>lists of dependencies and "
                            "external libraries,<br>build order,<br>intermediate files locations,<br>compiler options." ) );
    }

    int r = popup.exec( p );

    QString relpath = spitem->relativePath();
    if ( r == idAddSubproject )
    {
        slotAddSubproject( spitem );
    }
    if ( r == idRemoveSubproject )
    {
        slotRemoveSubproject( spitem );
    }
    if ( r == idDisableSubproject )
    {
        slotDisableSubproject( spitem );
    }
    if ( r == idAddScope )
    {
        slotCreateScope( spitem );
    }
    else if ( r == idRemoveScope )
    {
        slotRemoveScope( spitem );
    }
    else if ( r == idBuild )
    {
        slotBuildTarget();
        //        m_part->startMakeCommand(projectDirectory() + relpath, QString::fromLatin1(""));
    }
    else if ( r == idRebuild )
    {
        slotRebuildTarget();
    }
    else if ( r == idClean )
    {
        slotCleanTarget();
    }
    else if ( r == idDistClean )
    {
        slotDistCleanTarget();
    }

    else if ( r == idQmake )
    {
        m_part->startQMakeCommand( projectDirectory() + QString(QChar(QDir::separator())) + relpath );
    }
    else if ( r == idQmakeRecursive )
    {
        runQMakeRecursive( spitem );
    }

    else if ( r == idProjectConfiguration )
    {
        m_configDlg->updateSubproject( spitem );
        m_configDlg->show();
    }
}

void TrollProjectWidget::addFileToCurrentSubProject( GroupItem *titem, const QString &filename )
{

    m_filesCached = false;
    m_allFilesCache.clear();
    titem->addFileToScope( filename );
}

void TrollProjectWidget::addFileToCurrentSubProject( GroupItem::GroupType gtype, const QString &filename )
{
    if ( !m_shownSubproject )
        return ;

    m_filesCached = false;
    m_allFilesCache.clear();

    GroupItem *gitem = 0;

    if ( m_shownSubproject->groups.contains( gtype ) )
        gitem = m_shownSubproject->groups[ gtype ];

    if ( !gitem )
        return ;

    gitem->addFileToScope( filename );
}

/**
* Method adds a file to the current project by grouped
* by file extension
*/
void TrollProjectWidget::addFiles( QStringList &files, bool relativeToProjectRoot )
{
    if ( !m_shownSubproject )
        return ;
    kdDebug(9024) << "Files to add:"<<files << " " << relativeToProjectRoot <<endl;
    m_filesCached = false;
    m_allFilesCache.clear();

    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
        QString fileName = *it;
        kdDebug(9024) << "Adding file:" << fileName << " " << relativeToProjectRoot << endl;

        if ( m_shownSubproject->scope->variableValues( "TEMPLATE" ).findIndex( "subdirs" ) != -1 && !fileName.endsWith(".pro") )
        {
            ChooseSubprojectDlg dlg( this, false );
            if ( dlg.exec() == QDialog::Accepted )
            {
                if ( dlg.selectedSubproject() && dlg.selectedSubproject()->scope->variableValues("TEMPLATE").findIndex( "subdirs" ) != -1 )
                {
                    fileName = getRelativePath( dlg.selectedSubproject()->scope->projectDir() ,
                                                QDir::cleanDirPath(
                                                    m_shownSubproject->scope->projectDir()+
                                                    QString(QChar(QDir::separator()))+
                                                    fileName ) );
                    overview->setCurrentItem( dlg.selectedSubproject() );

                }
            }
            else
            {
                KMessageBox::error( this, i18n("You didn't select a subproject to add the file to or selected a subproject that has subdirs."), i18n( "File adding aborted" ) );
            }
        }

        QFileInfo info( fileName );
        QString ext = info.extension( false ).simplifyWhiteSpace();

        QString noPathFileName;
        if( relativeToProjectRoot )
            noPathFileName = getRelativePath( m_shownSubproject->scope->projectDir(), QDir::cleanDirPath(projectDirectory()+QString(QChar(QDir::separator()))+fileName ) );
        else
            noPathFileName = getRelativePath( m_shownSubproject->scope->projectDir(), QDir::cleanDirPath(m_shownSubproject->scope->projectDir()+QString(QChar(QDir::separator()))+fileName ) );

        if( DomUtil::readBoolEntry( *m_part->projectDom(), "/kdevtrollproject/qmake/replacePaths", false ) )
        {
            QString var = m_shownSubproject->scope->findCustomVarForPath( QFileInfo( noPathFileName ).dirPath() );
            if( !var.isEmpty() )
            {
                noPathFileName = "$${"+var+"}"+QString( QChar( QDir::separator() ) )+QFileInfo( noPathFileName ).fileName();
            }
        }

        kdDebug(9024) << "calc filename:" << noPathFileName << endl;
//         GroupItem *gitem = 0;
//         GroupItem::GroupType gtype = GroupItem::groupTypeForExtension( ext );
//         if ( m_shownSubproject->groups.contains( gtype ) )
//             gitem = m_shownSubproject->groups[ gtype ];


        if( ext == "pro" )
        {
            addSubprojectToItem( findSubprojectForPath( QFileInfo( fileName ).dirPath() ), QFileInfo( fileName ).fileName() );
        }else
        {
            addFileToCurrentSubProject( GroupItem::groupTypeForExtension( ext, m_part->isQt4Project() ), noPathFileName );
            slotOverviewSelectionChanged( m_shownSubproject );
            kdDebug(9024) << "emitting" << relativeToProjectRoot << " " << fileName << endl;
            if( relativeToProjectRoot )
                emitAddedFile ( projectDirectory()+QString( QChar( QDir::separator() ) ) + fileName );
            else
                emitAddedFile ( m_shownSubproject->scope->projectDir()+QString( QChar( QDir::separator() ) ) + fileName );
        }
    }

}


void TrollProjectWidget::slotAddFiles()
{
    static KURL lastVisited;
    QString cleanSubprojectDir = QDir::cleanDirPath( m_shownSubproject->scope->projectDir() );
    QString title, filter;
    QString otherTitle, otherFilter;

    GroupItem* item = dynamic_cast<GroupItem*>( details->selectedItem() );
    GroupItem::GroupType type = item ? item->groupType : GroupItem::NoType;
    GroupItem::groupTypeMeanings( type, title, filter );
    filter += "|" + title;

    m_filesCached = false;
    m_allFilesCache.clear();

    for ( int i = GroupItem::NoType + 1; i < GroupItem::MaxTypeEnum; ++i )
    {
        if ( type != i )
        {
            GroupItem::groupTypeMeanings( static_cast<GroupItem::GroupType>( i ), otherTitle, otherFilter );
            filter += "\n" + otherFilter + "|" + otherTitle;
        }
    }

    filter += "\n*|" + i18n( "All Files" );

    AddFilesDialog *dialog = new AddFilesDialog( cleanSubprojectDir,
                             filter,
                             this,
                             "Insert existing files",
                             true, new QComboBox( false ) );

    dialog->setMode( KFile::Files | KFile::ExistingOnly | KFile::LocalOnly );

    if ( !lastVisited.isEmpty() )
    {
        dialog->setURL( lastVisited );
    }

    dialog->exec();
    QStringList files = dialog->selectedFiles();
    lastVisited = dialog->baseURL();

    for ( unsigned int i = 0; i < files.count(); i++ )
    {
        switch ( dialog->mode() )
        {
            case AddFilesDialog::Copy:
                {
                    // Copy selected files to current subproject folder
                    // and add them to the filelist
                    QString filename = KURL( files[ i ] ).fileName();
                    KIO::NetAccess::file_copy( files[ i ], cleanSubprojectDir + QString( QChar( QDir::separator() ) ) + filename, -1, false, false, this );
                    QFile testExist( cleanSubprojectDir + QString( QChar( QDir::separator() ) ) + filename );

                    if ( testExist.exists() )
                    {
                        QStringList files( filename );
                        addFiles( files, false );
                    }
                }
                break;

            case AddFilesDialog::Link:
                {
                    // Link selected files to current subproject folder
                    KProcess *proc = new KProcess( this );
                    *proc << "ln";
                    *proc << "-s";
                    *proc << files[ i ];
                    *proc << cleanSubprojectDir;
                    proc->start(KProcess::Block);
                    QString filename = files[ i ].right( files[ i ].length() - files[ i ].findRev( '/' ) - 1 );
                    // and add them to the filelist
                    QFile testExist( cleanSubprojectDir + QString( QChar( QDir::separator() ) ) + filename );
                    if ( testExist.exists() )
                    {
                        QStringList files( filename );
                        addFiles( files, false );
                    }
                }
                break;

            case AddFilesDialog::Relative:
                {
                    // Form relative path to current subproject folder
                    QString theFile = files[ i ];
                    QStringList files( URLUtil::relativePathToFile( cleanSubprojectDir , theFile )
                                     );
                    addFiles( files, false );
                }
                break;
        }
    }
}

GroupItem* TrollProjectWidget::getInstallRoot( QMakeScopeItem* item )
{
    if ( item->groups.contains( GroupItem::InstallRoot ) )
        return item->groups[ GroupItem::InstallRoot ];
    return 0;
}

GroupItem* TrollProjectWidget::getInstallObject( QMakeScopeItem* item, const QString& objectname )
{
    GroupItem * instroot = getInstallRoot( item );
    if ( !instroot )
        return 0;
    QPtrListIterator<GroupItem> it( instroot->installs );
    for ( ;it.current();++it )
    {
        if ( ( *it ) ->groupType == GroupItem::InstallObject &&
                ( *it ) ->text( 0 ) == objectname )
            return * it;
    }
    return 0;

}

void TrollProjectWidget::slotNewFile()
{
    GroupItem * gitem = dynamic_cast<GroupItem*>( details->currentItem() );

    m_filesCached = false;
    m_allFilesCache.clear();

    if( !gitem )
    {
        gitem = dynamic_cast<GroupItem*>( details->currentItem()->parent() );
    }

    if ( gitem )
    {
        if ( gitem->groupType == GroupItem::InstallObject )
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
                addFileToCurrentSubProject( gitem, filepattern );
                slotOverviewSelectionChanged( m_shownSubproject );
            }
            return ;
        }
        if ( gitem->groupType == GroupItem::InstallRoot )
        {
            //          QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
            bool ok = FALSE;
            QString install_obj = KInputDialog::getText(
                                      i18n( "Insert New Install Object" ),
                                      i18n( "Please enter a name for the new object:" ),
                                      QString::null, &ok, this );
            if ( ok && !install_obj.isEmpty() )
            {
                gitem->addInstallObject( install_obj );
                //GroupItem * institem = createGroupItem( GroupItem::InstallObject, install_obj , m_shownSubproject );
                //gitem->owner->scope->addToPlusOp("INSTALLS", install_obj);
                gitem->owner->scope->saveToFile();
                slotOverviewSelectionChanged( m_shownSubproject );
            }
            return ;
        }
    }
    KDevCreateFile * createFileSupport = m_part->extension<KDevCreateFile>( "KDevelop/CreateFile" );
    QString fcext;
    if( gitem )
    {
        switch ( gitem->groupType )
        {
            case GroupItem::Sources:
                fcext = "cpp";
                break;
            case GroupItem::Headers:
                fcext = "h";
                break;
            case GroupItem::Forms:
                if ( !m_part->isQt4Project() )
                    fcext = "ui-widget";
                else
                    fcext = "ui-widget-qt4";
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
            case GroupItem::Resources:
                fcext = "qrc";
                break;
            default:
                fcext = QString::null;
        }
    }
    KDevCreateFile::CreatedFile crFile =
    createFileSupport->createNewFile( fcext, projectDirectory() + QString(QChar(QDir::separator()))+ m_shownSubproject->relativePath() );
}

void TrollProjectWidget::slotRemoveFile()
{
    QListViewItem * selectedItem = details->currentItem();
    if ( !selectedItem )
        return ;

    m_filesCached = false;
    m_allFilesCache.clear();

    qProjectItem *pvitem = static_cast<qProjectItem*>( selectedItem );
    // Check that it is a file (just in case)
    if ( pvitem->type() != qProjectItem::File )
        return ;
    FileItem *fitem = static_cast<FileItem*>( pvitem );
    removeFile( m_shownSubproject, fitem );
}

void TrollProjectWidget::slotExcludeFileFromScopeButton()
{
    QListViewItem * selectedItem = details->currentItem();
    if ( !selectedItem )
        return ;
    qProjectItem *pvitem = static_cast<qProjectItem*>( selectedItem );
    // Check that it is a file (just in case)
    if ( pvitem->type() != qProjectItem::File )
        return ;
    FileItem *fitem = static_cast<FileItem*>( pvitem );

    GroupItem *gitem = static_cast<GroupItem*>( fitem->parent() );

    gitem->removeFileFromScope( fitem->text( 0 ) );
}

void TrollProjectWidget::slotDetailsSelectionChanged( QListViewItem *item )
{
    if ( !item )
    {
        removefileButton->setEnabled( false );
        excludeFileFromScopeButton->setEnabled( false );
        return ;
    }
    removefileButton->setEnabled( false );
    excludeFileFromScopeButton->setEnabled( false );

    qProjectItem *pvitem = static_cast<qProjectItem*>( item );
    if ( pvitem->type() == qProjectItem::Group )
    {
        GroupItem * gitem = static_cast<GroupItem*>( item );
        if ( gitem->groupType == GroupItem::InstallObject )
        {
            excludeFileFromScopeButton->setEnabled( true );
            newfileButton->setEnabled( true );
        }
        else if ( gitem->groupType == GroupItem::InstallRoot )
        {
            newfileButton->setEnabled( true );
        }
        else
        {
            addfilesButton->setEnabled( true );
            newfileButton->setEnabled( true );
        }


    }
    else if ( pvitem->type() == qProjectItem::File )
    {
        removefileButton->setEnabled( true );
        excludeFileFromScopeButton->setEnabled( true );
        /*        buildTargetButton->setEnabled(true);
                rebuildTargetButton->setEnabled(true);
                executeTargetButton->setEnabled(true);*/
    }
}

void TrollProjectWidget::slotDetailsContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
    if ( !item )
        return ;

    qProjectItem *pvitem = static_cast<qProjectItem*>( item );
    if ( pvitem->type() == qProjectItem::Group )
    {
        GroupItem * titem = static_cast<GroupItem*>( pvitem );
        QString title, ext;
        GroupItem::groupTypeMeanings( titem->groupType, title, ext );

        KPopupMenu popup( this );
        popup.insertTitle( title );

        int idInsExistingFile = -2;
        int idInsNewFile = -2;
        int idInsInstallObject = -2;
        int idInsNewFilepatternItem = -2;
        int idSetInstObjPath = -2;
        int idLUpdate = -2;
        int idLRelease = -2;

        //       int idFileProperties = popup.insertItem(SmallIconSet("filenew"),i18n("Properties..."));
        if ( titem->groupType == GroupItem::InstallRoot )
        {
            idInsInstallObject = popup.insertItem( SmallIconSet( "fileopen" ), i18n( "Add Install Object..." ) );
            popup.setWhatsThis( idInsInstallObject, i18n( "<b>Add install object</b><p>Creates QMake install object. "
                                "It is possible to define a list of files to install and installation locations for each object. Warning! "
                                "Install objects without path specified will not be saved to a project file." ) );
        }
        else if ( titem->groupType == GroupItem::InstallObject )
        {
            idSetInstObjPath = popup.insertItem( SmallIconSet( "fileopen" ), i18n( "Install Path..." ) );
            popup.setWhatsThis( idSetInstObjPath, i18n( "<b>Install path</b><p>Allows to choose the installation path for the current install object." ) );
            idInsNewFilepatternItem = popup.insertItem( SmallIconSet( "fileopen" ), i18n( "Add Pattern of Files to Install..." ) );
            popup.setWhatsThis( idInsNewFilepatternItem, i18n( "<b>Add pattern of files to install</b><p>Defines the pattern to match files which will be installed. "
                                "It is possible to use wildcards and relative paths like <i>docs/*</i>." ) );
        }
        else if ( titem->groupType == GroupItem::Translations )
        {
            idInsNewFile = popup.insertItem( SmallIconSet( "filenew" ), i18n( "Create New File..." ) );
            popup.setWhatsThis( idInsNewFile, i18n( "<b>Create new file</b><p>Creates a new translation file and adds it to a currently selected TRANSLATIONS group." ) );
            idInsExistingFile = popup.insertItem( SmallIconSet( "fileopen" ), i18n( "Add Existing Files..." ) );
            popup.setWhatsThis( idInsExistingFile, i18n( "<b>Add existing files</b><p>Adds existing translation (*.ts) files to a currently selected TRANSLATIONS group. It is "
                                "possible to copy files to a current subproject directory, create symbolic links or "
                                "add them with the relative path." ) );
            idLUpdate = popup.insertItem( SmallIconSet( "konsole" ), i18n( "Update Translation Files" ) );
            popup.setWhatsThis( idLUpdate, i18n( "<b>Update Translation Files</b><p>Runs <b>lupdate</b> command from the current subproject directory. It collects translatable "
                                                 "messages and saves them into translation files." ) );
            idLRelease = popup.insertItem( SmallIconSet( "konsole" ), i18n( "Release Binary Translations" ) );
            popup.setWhatsThis( idLRelease, i18n( "<b>Release Binary Translations</b><p>Runs <b>lrelease</b> command from the current subproject directory. It creates binary "
                                                  "translation files that are ready to be loaded at program execution." ) );
        }
        else // File group containing files
        {
            idInsNewFile = popup.insertItem( SmallIconSet( "filenew" ), i18n( "Create New File..." ) );
            popup.setWhatsThis( idInsNewFile, i18n( "<b>Create new file</b><p>Creates a new file and adds it to a currently selected group." ) );
            idInsExistingFile = popup.insertItem( SmallIconSet( "fileopen" ), i18n( "Add Existing Files..." ) );
            popup.setWhatsThis( idInsExistingFile, i18n( "<b>Add existing files</b><p>Adds existing files to a currently selected group. It is "
                                "possible to copy files to a current subproject directory, create symbolic links or "
                                "add them with the relative path." ) );
        }
        int r = popup.exec( p );
        QString cleanSubprojectPath = QDir::cleanDirPath( m_shownSubproject->scope->projectDir() );

        if ( r == idSetInstObjPath )
        {
            KURLRequesterDlg dialog( i18n( "Choose Install Path" ), i18n( "Enter a path "
                                     "(example /usr/local/share/... ):" ), this, 0 );
            dialog.urlRequester() ->setMode( KFile::Directory );
            dialog.urlRequester() ->setURL( titem->owner->scope->variableValues( titem->text( 0 ) + ".path" ).front() );
            if ( dialog.exec() == QDialog::Accepted )
            {
                titem->owner->scope->setEqualOp( titem->text( 0 ) + ".path", dialog.urlRequester() ->url() );
                titem->owner->scope->saveToFile( );
            }
        }
        else if ( r == idInsNewFilepatternItem )
        {
            bool ok = FALSE;
            QString filepattern = KInputDialog::getText(
                                      i18n( "Add Pattern of Files to Install" ),
                                      i18n( "Enter a pattern relative to the current "
                                            "subproject (example docs/*.html):" ),
                                      QString::null, &ok, this );
            if ( ok && !filepattern.isEmpty() )
            {
                addFileToCurrentSubProject( titem, filepattern );
                slotOverviewSelectionChanged( m_shownSubproject );
            }
        }
        else if ( r == idInsExistingFile )
        {
            AddFilesDialog * dialog = new AddFilesDialog( cleanSubprojectPath,
                                      ext + "|" + title + " (" + ext + ")",
                                      this,
                                      "Add existing files",
                                      true, new QComboBox( false ) );
            dialog->setMode( KFile::Files | KFile::ExistingOnly | KFile::LocalOnly );
            if ( dialog->exec() == QDialog::Rejected )
                return ;
            QStringList files = dialog->selectedFiles();
            for ( unsigned int i = 0;i < files.count();++i )
            {
                switch ( dialog->mode() )
                {
                    case AddFilesDialog::Copy:
                        {
                            // Copy selected files to current subproject folder
                            // and add them to the filelist
                            QString filename = KURL( files[ i ] ).fileName();
                            KIO::NetAccess::file_copy( files[ i ], cleanSubprojectPath + QString( QChar( QDir::separator() ) ) + filename, -1, false, false, this );
                            QFile testExist( cleanSubprojectPath + QString( QChar( QDir::separator() ) ) + filename );

                            if ( testExist.exists() )
                            {
                                QStringList files( filename );
                                addFiles( files, false );
                            }
                        }
                        break;

                    case AddFilesDialog::Link:
                        {
                            // Link selected files to current subproject folder
                            KProcess *proc = new KProcess( this );
                            *proc << "ln";
                            *proc << "-s";
                            *proc << files[ i ];
                            *proc << cleanSubprojectPath;
                            proc->start(KProcess::Block);
                            QString filename = files[ i ].right( files[ i ].length() - files[ i ].findRev( '/' ) - 1 );
                            // and add them to the filelist
                            QFile testExist( cleanSubprojectPath + QString( QChar( QDir::separator() ) ) + filename );
                            if ( testExist.exists() )
                            {
                                QStringList files( filename );
                                addFiles( files, false );
                            }
                        }
                        break;

                    case AddFilesDialog::Relative:
                        {
                            // Form relative path to current subproject folder
                            QString theFile = files[ i ];
                            QStringList files( URLUtil::relativePathToFile( cleanSubprojectPath, theFile )
                                             );
                            addFiles( files, false );
                        }
                        break;
                }
            }
            // Update project file
            if ( titem && titem->owner )
            {
                titem->owner->scope->saveToFile( );
            }
            // Update subprojectview
            slotOverviewSelectionChanged( m_shownSubproject );
        }
        else if ( r == idInsNewFile )
        {
            KDevCreateFile * createFileSupport = m_part->extension<KDevCreateFile>( "KDevelop/CreateFile" );
            QString fcext;
            switch ( titem->groupType )
            {
                case GroupItem::Sources:
                    fcext = "cpp";
                    break;
                case GroupItem::Headers:
                    fcext = "h";
                    break;
                case GroupItem::Forms:
                    if ( !m_part->isQt4Project() )
                        fcext = "ui-widget";
                    else
                        fcext = "ui-widget-qt4";
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
                case GroupItem::Resources:
                    fcext = "qrc";
                    break;
                default:
                    fcext = QString::null;
            }
            KDevCreateFile::CreatedFile crFile =
                createFileSupport->createNewFile( fcext, cleanSubprojectPath );

        }
        else if ( r == idInsInstallObject )
        {
            bool ok = FALSE;
            QString install_obj = KInputDialog::getText(
                                      i18n( "Add Install Object" ),
                                      i18n( "Enter a name for the new object:" ),
                                      QString::null, &ok, this );
            if ( ok && !install_obj.isEmpty() )
            {
                titem->addInstallObject( install_obj );
                slotOverviewSelectionChanged( m_shownSubproject );
            }
        }
        else if ( r == idLUpdate )
        {
            QString cmd = "lupdate ";
            cmd += m_shownSubproject->scope->fileName();
            m_part->appFrontend() ->startAppCommand( m_shownSubproject->scope->projectDir(), cmd, false );
        }
        else if ( r == idLRelease )
        {
            QString cmd = "lrelease ";
            cmd += m_shownSubproject->scope->fileName();
            m_part->appFrontend() ->startAppCommand( m_shownSubproject->scope->projectDir(), cmd, false );
        }
    }
    else if ( pvitem->type() == qProjectItem::File )
    {

        removefileButton->setEnabled( true );
        FileItem *fitem = static_cast<FileItem*>( pvitem );
        GroupItem::GroupType gtype = static_cast<GroupItem*>( item->parent() ) ->groupType;

        KPopupMenu popup( this );
        if ( !( gtype == GroupItem::InstallObject ) )
            popup.insertTitle( i18n( "File: %1" ).arg( fitem->text( 0 ) ) );
        else
            popup.insertTitle( i18n( "Pattern: %1" ).arg( fitem->text( 0 ) ) );

        int idRemoveFile = -2;
        int idSubclassWidget = -2;
        int idUpdateWidgetclass = -2;
        int idBuildFile = -2;
        int idUISubclasses = -2;
        int idViewUIH = -2;
        int idFileProperties = -2;
        int idEditInstallPattern = -2;

        if ( !fitem->uiFileLink.isEmpty() )
        {
            idUpdateWidgetclass = popup.insertItem( SmallIconSet( "qmake_subclass" ), i18n( "Edit ui-Subclass..." ) );
            popup.setWhatsThis( idUpdateWidgetclass, i18n( "<b>Edit ui-subclass</b><p>Launches <b>Subclassing</b> wizard "
                                "and prompts to implement missing in childclass slots and functions." ) );
        }
        if ( fitem->text( 0 ).contains( ".ui" ) )
        {
            idSubclassWidget = popup.insertItem( SmallIconSet( "qmake_subclass" ), i18n( "Subclassing Wizard..." ) );
            popup.setWhatsThis( idSubclassWidget, i18n( "<b>Subclass widget</b><p>Launches <b>Subclassing</b> wizard. "
                                "It allows to create a subclass from the class defined in .ui file. "
                                "There is also possibility to implement slots and functions defined in the base class." ) );
            if ( !m_part->isQt4Project() )
            {
                idViewUIH = popup.insertItem( SmallIconSet( "qmake_ui_h" ), i18n( "Open ui.h File" ) );
                popup.setWhatsThis( idViewUIH, i18n( "<b>Open ui.h file</b><p>Opens .ui.h file associated with the selected .ui." ) );
            }
            idUISubclasses = popup.insertItem( SmallIconSet( "qmake_subclass" ), i18n( "List of Subclasses..." ) );
            popup.setWhatsThis( idUISubclasses, i18n( "<b>List of subclasses</b><p>Shows subclasses list editor. "
                                "There is possibility to add or remove subclasses from the list." ) );
        }
        if ( !( gtype == GroupItem::InstallObject ) )
        {
            idRemoveFile = popup.insertItem( SmallIconSet( "editdelete" ), i18n( "Remove File" ) );
            popup.setWhatsThis( idRemoveFile, i18n( "<b>Remove file</b><p>Removes file from a current group. Does not remove file from disk." ) );
            idFileProperties = popup.insertItem( SmallIconSet( "configure_file" ), i18n( "Exclude File" ) );
            popup.setWhatsThis( idFileProperties, i18n( "<b>Exclude File</b><p>Excludes the file from this Scope." ) );
        }
        else
        {
            idEditInstallPattern = popup.insertItem( SmallIconSet( "configure_file" ), i18n( "Edit Pattern" ) );
            popup.setWhatsThis( idEditInstallPattern, i18n( "<b>Edit pattern</b><p>Allows to edit install files pattern." ) );
            idRemoveFile = popup.insertItem( SmallIconSet( "editdelete" ), i18n( "Remove Pattern" ) );
            popup.setWhatsThis( idRemoveFile, i18n( "<b>Remove pattern</b><p>Removes install files pattern from the current install object." ) );
        }
        if ( !( gtype == GroupItem::InstallObject ) )
        {
            KURL::List urls;
            urls.append( m_shownSubproject->scope->projectDir() + QString( QChar( QDir::separator() ) ) + fitem->text( 0 ) );
            FileContext context( urls );
            m_part->core() ->fillContextMenu( &popup, &context );
        }
        if ( gtype == GroupItem::Sources )
        {
            idBuildFile = popup.insertItem( SmallIconSet( "make_kdevelop" ), i18n( "Build File" ) );
            popup.setWhatsThis( idBuildFile, i18n( "<b>Build File</b><p>Builds the object file for this source file." ) );
        }

        int r = popup.exec( p );
        if ( r == idRemoveFile )
            removeFile( m_shownSubproject, fitem );
        // Fileproperties
        else if ( r == idFileProperties )
        {
            /*
              GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());
              if (!gitem)
                return;
              QStringList dirtyScopes;
              FilePropertyDlg *propdlg = new FilePropertyDlg(m_shownSubproject,gitem->groupType,fitem,dirtyScopes);
              QMakeScopeItem *scope;
              propdlg->exec();
              for (uint i=0; i<dirtyScopes.count();i++)
              {
                scope = getScope(m_shownSubproject,dirtyScopes[i]);
                if (scope)
                   updateProjectFile(scope);
              }
            */
            slotExcludeFileFromScopeButton();
        }
        else if ( r == idViewUIH )
        {
            m_part->partController() ->editDocument( KURL( m_shownSubproject->scope->projectDir() + QString( QChar( QDir::separator() ) ) +
                    QString( fitem->text( 0 ) + ".h" ) ) );

        }
        else if ( r == idSubclassWidget )
        {
            QStringList newFileNames;
            newFileNames = m_part->languageSupport() ->subclassWidget( m_shownSubproject->scope->projectDir() + QString( QChar( QDir::separator() ) ) + fitem->text( 0 ) );
            kdDebug(9024) << "got new filenames: " << newFileNames << endl;
            if ( !newFileNames.empty() )
            {
                QDomDocument & dom = *( m_part->projectDom() );
                for ( uint i = 0; i < newFileNames.count(); ++i )
                {
                    QString srcfile_relpath = getRelativePath( m_shownSubproject->scope->projectDir(), newFileNames[ i ] ) ;
                    newFileNames[i] = getRelativePath( projectDirectory(), newFileNames[ i ] ) ;
                    QString uifile_relpath = m_shownSubproject->relativePath() + QString( QChar( QDir::separator() ) ) + fitem->text( 0 );
                    DomUtil::PairList list = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                                             "subclass", "sourcefile", "uifile" );

                    list << DomUtil::Pair( srcfile_relpath, uifile_relpath );
                    DomUtil::writePairListEntry( dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list );
                    //                    newFileNames[i] = newFileNames[i].replace(QRegExp(projectDirectory()+"/"),"");
                    qWarning( "new file: %s", newFileNames[ i ].latin1() );
                }
                m_subclasslist = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                                 "subclass", "sourcefile", "uifile" );

                m_part->addFiles( newFileNames );
            }
        }
        else if ( r == idUpdateWidgetclass )
        {
            QString noext = m_shownSubproject->scope->projectDir() + QString( QChar( QDir::separator() ) ) + fitem->text( 0 );
            if ( noext.findRev( '.' ) > -1 )
                noext = noext.left( noext.findRev( '.' ) );
            QStringList dummy;
            QString uifile = fitem->uiFileLink;
            if ( uifile.findRev( QString( QChar( QDir::separator() ) ) ) > -1 )
            {
                QStringList uisplit = QStringList::split( QString( QChar( QDir::separator() ) ), uifile );
                uifile = uisplit[ uisplit.count() - 1 ];
            }
            m_part->languageSupport() ->updateWidget( m_shownSubproject->scope->projectDir() + QString( QChar( QDir::separator() ) ) + uifile, noext );
        }
        else if ( r == idUISubclasses )
        {
            QDomDocument & dom = *( m_part->projectDom() );
            DomUtil::PairList list = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                                     "subclass", "sourcefile", "uifile" );
            SubclassesDlg *sbdlg = new SubclassesDlg( m_shownSubproject->relativePath() + QString( QChar( QDir::separator() ) ) + fitem->text( 0 ),
                                   list, projectDirectory() );

            if ( sbdlg->exec() )
            {
                QDomElement el = DomUtil::elementByPath( dom, "/kdevtrollproject" );
                QDomElement el2 = DomUtil::elementByPath( dom, "/kdevtrollproject/subclassing" );
                if ( ( !el.isNull() ) && ( !el2.isNull() ) )
                {
                    el.removeChild( el2 );
                }

                DomUtil::writePairListEntry( dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list );

                m_subclasslist = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                                 "subclass", "sourcefile", "uifile" );
            }
        }
        else if ( r == idEditInstallPattern )
        {
            GroupItem * titem = static_cast<GroupItem*>( item->parent() );

            bool ok = FALSE;
            QString filepattern = KInputDialog::getText(
                                      i18n( "Edit Pattern" ),
                                      i18n( "Enter a pattern relative to the current "
                                            "subproject (example docs/*.html):" ),
                                      fitem->text( 0 ) , &ok, this );
            if ( ok && !filepattern.isEmpty() )
            {
                removeFile( m_shownSubproject, fitem );
                addFileToCurrentSubProject( titem, filepattern );
                slotOverviewSelectionChanged( m_shownSubproject );
            }
        }
        else if ( r == idBuildFile )
        {
            buildFile( m_shownSubproject, fitem );
        }
    }
}


void TrollProjectWidget::removeFile( QMakeScopeItem *spitem, FileItem *fitem )
{
    GroupItem * gitem = static_cast<GroupItem*>( fitem->parent() );

    m_filesCached = false;
    m_allFilesCache.clear();
    QString realfilename = spitem->scope->resolveVariables( fitem->text( 0 ) );
    if ( KMessageBox::warningYesNo( this,
                                    "<qt>" +
                                    i18n( "Are you sure you wish to remove <strong>%1</strong> from this subproject/scope?" )
                                    .arg( fitem->text( 0 ) ) +
                                    "</qt>",
                                    i18n( "Remove File" ),
#if KDE_IS_VERSION(3,3,90)
                                    KStdGuiItem::remove(),
#else
                                    i18n( "&Remove" ),
#endif
                                        KStdGuiItem::no(),
                                        "removeFileFromQMakeProject" ) == KMessageBox::No )
    {
        return ;
    }

    if ( gitem->groupType != GroupItem::InstallObject )
    {
        QString removedFileName = spitem->relativePath() + QString( QChar( QDir::separator() ) ) + realfilename;
        if ( removedFileName.startsWith( QDir::rootDirPath() ) )
            removedFileName = removedFileName.mid( 1 );
        emitRemovedFile( removedFileName );
    }


    //remove subclassing info
    QDomDocument &dom = *( m_part->projectDom() );
    DomUtil::PairList list = DomUtil::readPairListEntry( dom, "/kdevtrollproject/subclassing" ,
                             "subclass", "sourcefile", "uifile" );
    QPtrList<DomUtil::Pair> pairsToRemove;
    DomUtil::PairList::iterator it;
  QString file = QString( spitem->scope->projectDir() + QString( QChar( QDir::separator() ) ) + realfilename.mid(1, realfilename.length()-2)  ).remove( 0, projectDirectory().length() );
    for ( it = list.begin(); it != list.end(); ++it )
    {
        if ( ( ( *it ).first == file ) || ( ( *it ).second == file ) )
        {
            pairsToRemove.append( &( *it ) );
        }
    }
    DomUtil::Pair *pair;
    for ( pair = pairsToRemove.first(); pair; pair = pairsToRemove.next() )
    {
        list.remove( *pair );
    }
    QDomElement el = DomUtil::elementByPath( dom, "/kdevtrollproject" );
    QDomElement el2 = DomUtil::elementByPath( dom, "/kdevtrollproject/subclassing" );
    if ( ( !el.isNull() ) && ( !el2.isNull() ) )
    {
        el.removeChild( el2 );
    }
    DomUtil::writePairListEntry( dom, "/kdevtrollproject/subclassing", "subclass", "sourcefile", "uifile", list );

    gitem->removeFileFromScope( fitem->text( 0 ) );
}

void TrollProjectWidget::emitAddedFile( const QString &fileName )
{
    emit m_part->addedFilesToProject( QStringList( fileName ) );
}


void TrollProjectWidget::emitRemovedFile( const QString &fileName )
{
    emit m_part->removedFilesFromProject( QStringList( fileName ) );
}


QString TrollProjectWidget::getUiFileLink( const QString &relpath, const QString& filename )
{
    DomUtil::PairList::iterator it;
    for ( it = m_subclasslist.begin();it != m_subclasslist.end(); ++it )
    {
        if ( ( *it ).first == relpath + filename )
            return ( *it ).second;
    }
    return "";
}

void TrollProjectWidget::slotBuildOpenFile()
{
    KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( m_part->partController() ->activePart() );
    if ( !part || !part->url().isLocalFile() )
        return ;

    QString fileName = part->url().path();
    QFileInfo fi( fileName );
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName( true );
    kdDebug( 9024 ) << "Compiling " << fileName
    << "in dir " << sourceDir
    << " with baseName " << baseName << endl;


    QString buildDir = sourceDir;
    QString target = baseName + ".o";

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    //    m_part->startMakeCommand(buildDir, target);

    QPtrList<QMakeScopeItem> list = findSubprojectForFile( fi );

    QMakeScopeItem *spitem;
    for ( spitem = list.first(); spitem; spitem = list.next() )
    {
        QString buildcmd = constructMakeCommandLine( spitem->scope );
        QString dircmd = "cd " + KProcess::quote( spitem->scope->projectDir() ) + " && " ;
        kdDebug( 9024 ) << "builddir " << spitem->scope->projectDir() << ", cmd " << dircmd + buildcmd + " " + target << endl;
        m_part->queueCmd( spitem->scope->projectDir(), dircmd + buildcmd + " " + target );
    }

    //    startMakeCommand(buildDir, target);

}


void TrollProjectWidget::slotExecuteProject()
{
    QString program = m_part->mainProgram();
    if ( !program.startsWith( QDir::rootDirPath() ) )
        program.prepend( "." + QString( QChar( QDir::separator() ) ) );

    if ( program.isEmpty() )
    {
        KMessageBox::sorry( this, i18n( "Please specify the executable name in the "
                                        "project options dialog first." ), i18n( "No Executable Name Given" ) );
        return ;
    }

    // Build environment variables to prepend to the executable path
    QString runEnvVars = QString::null;
    DomUtil::PairList list =
        DomUtil::readPairListEntry( *( m_part->projectDom() ), "/kdevtrollproject/run/envvars", "envvar", "name", "value" );

    DomUtil::PairList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
    {
        const DomUtil::Pair &pair = ( *it );
        if ( ( !pair.first.isEmpty() ) && ( !pair.second.isEmpty() ) )
            runEnvVars += pair.first + "=" + pair.second + " ";
    }
    program.prepend( runEnvVars );
    program.append( " " + DomUtil::readEntry( *( m_part->projectDom() ), "/kdevtrollproject/run/programargs" ) + " " );

    QString dircmd = "cd " + KProcess::quote( this->projectDirectory() ) + " && " ;

    bool inTerminal = DomUtil::readBoolEntry( *( m_part->projectDom() ), "/kdevtrollproject/run/terminal" );
    //    m_part->appFrontend()->startAppCommand(dircmd + program, inTerminal);
    //    m_part->execute(this->projectDirectory(), "./"+program );
    m_part->appFrontend() ->startAppCommand( this->projectDirectory(), program, inTerminal );
}


void TrollProjectWidget::slotCleanProject()
{
    runClean(m_rootSubproject, "clean");
}

void TrollProjectWidget::slotCleanTarget()
{
    runClean(m_shownSubproject, "clean");
}

void TrollProjectWidget::slotDistCleanProject()
{
    runClean(m_rootSubproject, "distclean");

}

void TrollProjectWidget::slotDistCleanTarget()
{
    runClean(m_shownSubproject, "distclean");
}

void TrollProjectWidget::runClean( QMakeScopeItem* item, const QString& cleantargetname )
{
    // no subproject selected
    m_part->partController() ->saveAllFiles();
    if ( !item )
        return ;
    // can't build from scope
    if ( item->scope->scopeType() != Scope::ProjectScope )
        return ;

    QString dir = item->scope->projectDir();
    createMakefileIfMissing( dir, item );

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    QString dircmd = "cd " + KProcess::quote( dir ) + " && " ;
    QString rebuildcmd = constructMakeCommandLine( item->scope ) + " "+cleantargetname;
    m_part->queueCmd( dir, dircmd + rebuildcmd );
}

QString TrollProjectWidget::constructMakeCommandLine( Scope* s )
{
    QString makeFileName;
    if ( s )
        makeFileName = s->resolveVariables( s->variableValues( "MAKEFILE" ).front() );

    QDomDocument & dom = *( m_part->projectDom() );

    QString cmdline = DomUtil::readEntry( dom, "/kdevtrollproject/make/makebin" );
    if ( cmdline.isEmpty() )
        cmdline = MAKE_COMMAND;
    if ( !makeFileName.isEmpty() )
    {
        cmdline += " -f " + makeFileName;
    }
    if ( !DomUtil::readBoolEntry( dom, "/kdevtrollproject/make/abortonerror" ) )
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry( dom, "/kdevtrollproject/make/numberofjobs" );
    if ( jobs != 0 )
    {
        cmdline += " -j";
        cmdline += QString::number( jobs );
    }
    if ( DomUtil::readBoolEntry( dom, "/kdevtrollproject/make/dontact" ) )
        cmdline += " -n";

    cmdline += " ";
    cmdline.prepend( m_part->makeEnvironment() );

    return cmdline;
}

void TrollProjectWidget::startMakeCommand( const QString & dir, const QString & target )
{
    m_part->partController() ->saveAllFiles();

    QDomDocument &dom = *( m_part->projectDom() );

    if ( target == "clean" )
    {
        QString cmdline = DomUtil::readEntry( dom, "/kdevtrollproject/make/makebin" );
        if ( cmdline.isEmpty() )
            cmdline = MAKE_COMMAND;
        cmdline += " clean";
        QString dircmd = "cd " + KProcess::quote( dir ) + " && ";
        cmdline.prepend( m_part->makeEnvironment() );
        m_part->makeFrontend() ->queueCommand( dir, dircmd + cmdline );
    }

    QString cmdline = constructMakeCommandLine() + " " + target;

    QString dircmd = "cd " + KProcess::quote( dir ) + " && ";

    cmdline.prepend( m_part->makeEnvironment() );
    m_part->makeFrontend() ->queueCommand( dir, dircmd + cmdline );
}

void TrollProjectWidget::createMakefileIfMissing( const QString &dir, QMakeScopeItem *item )
{
    QFileInfo fi;
    QFileInfo fi2;
    if ( item->scope->variableValues( "MAKEFILE" ).isEmpty() )
    {
        fi.setFile( dir + QString( QChar( QDir::separator() ) ) + "Makefile" );
        fi2.setFile( dir + QString( QChar( QDir::separator() ) ) + "makefile" );
    }
    else
    {
        QString realmf = item->scope->resolveVariables( item->scope->variableValues( "MAKEFILE" ).front() );
        fi.setFile( realmf );
        fi2.setFile( dir + QString( QChar( QDir::separator() ) ) + realmf );
    }
    if ( !fi.exists() && !fi2.exists() )
    {
        int r = KMessageBox::questionYesNo( this, i18n( "There is no Makefile in this directory. Run qmake first?" ), QString::null, i18n( "Run qmake" ), i18n( "Do Not Run" ) );
        if ( r == KMessageBox::No )
            return ;
        m_part->startQMakeCommand( dir );
    }
}

QMakeScopeItem* TrollProjectWidget::findSubprojectForPath( const QString& relPath )
{
    QStringList dirs = QStringList::split("/", relPath);
    QMakeScopeItem* pitem = static_cast<QMakeScopeItem*>(m_rootSubproject);
    for( QStringList::iterator it = dirs.begin(); it != dirs.end(); ++it)
    {
        QListViewItem* item = pitem->firstChild();
        while( item )
        {
            QMakeScopeItem* sitem = static_cast<QMakeScopeItem*>(item);
            if( QFileInfo( sitem->scope->projectDir() ).fileName() == *it )
            {
                pitem = sitem;
                break;
            }
        }
    }
    return pitem;
}

QPtrList<QMakeScopeItem> TrollProjectWidget::findSubprojectForFile( QFileInfo fi )
{
    QPtrList<QMakeScopeItem> list;
    findSubprojectForFile( list, m_rootSubproject, fi.absFilePath() );
    return list;
}

void TrollProjectWidget::findSubprojectForFile( QPtrList<QMakeScopeItem> &list, QMakeScopeItem * item, QString absFilePath )
{
    QDir d( item->scope->projectDir() );

    QStringList vars = item->scope->variableValues( "SOURCES" );
    for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it )
    {
        QFileInfo fi2( d, item->scope->resolveVariables( *it ) );
        if ( absFilePath == fi2.absFilePath() )
            list.append( item );
    }

    vars = item->scope->variableValues( "HEADERS" );
    for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it )
    {
        QFileInfo fi2( d, item->scope->resolveVariables( *it ) );
        if ( absFilePath == fi2.absFilePath() )
            list.append( item );
    }

    QListViewItem * child = item->firstChild();
    while ( child )
    {
        QMakeScopeItem * spitem = dynamic_cast<QMakeScopeItem*>( child );

        if ( spitem )
        {
            findSubprojectForFile( list, spitem, absFilePath );
        }

        child = child->nextSibling();
    }
}

void TrollProjectWidget::slotRemoveScope( QMakeScopeItem * spitem )
{
    if ( spitem == 0 && m_shownSubproject == 0 )
        return ;
    else
    {
        m_filesCached = false;
        m_allFilesCache.clear();

        QMakeScopeItem* pitem = dynamic_cast<QMakeScopeItem *>( spitem->parent() );
        if ( pitem != 0 )
        {
            switch ( spitem->scope->scopeType() )
            {
                case Scope::FunctionScope:
                    if( !pitem->scope->deleteFunctionScope( spitem->scope->getNum() ) )
                    {
                        KMessageBox::error(this, i18n("Couldn't delete Function Scope.\nThis is an internal error, please write a bugreport to bugs.kde.org and include the output of kdevelop when run from a shell."),i18n("Function Scope Deletion failed"));
                        return;
                    }
                    //                     pitem->scopes.remove( spitem );
                    break;
                case Scope::IncludeScope:
                    if( !pitem->scope->deleteIncludeScope( spitem->scope->getNum() ) )
                    {
                        KMessageBox::error(this, i18n("Couldn't delete Include Scope.\nThis is an internal error, please write a bugreport to bugs.kde.org and include the output of kdevelop when run from a shell."),i18n("Include Scope Deletion failed"));
                        return;
                    }
                    //                     pitem->scopes.remove( spitem );
                    delete spitem;
                    spitem = pitem;
                    pitem = dynamic_cast<QMakeScopeItem *>( pitem->parent() );
                    //                     pitem->scopes.remove(spitem);
                    break;
                case Scope::SimpleScope:
                    if( !pitem->scope->deleteSimpleScope( spitem->scope->getNum() ) )
                    {
                        KMessageBox::error(this, i18n("Couldn't delete Scope.\nThis is an internal error, please write a bugreport to bugs.kde.org and include the output of kdevelop when run from a shell."),i18n("Scope Deletion failed"));
                        return;
                    }
                    //                     pitem->scopes.remove( spitem );
                    break;
                default:
                    break;
            }
            pitem->scope->saveToFile();
            delete spitem;
            m_shownSubproject = pitem;
            overview->setCurrentItem ( m_shownSubproject );
            overview->setSelected( m_shownSubproject, true );
            slotOverviewSelectionChanged( m_shownSubproject );
        }
    }
}

QMakeScopeItem * TrollProjectWidget::findSubprojectForScope( QMakeScopeItem * scope )
{
    if ( ( scope == 0 ) || ( scope->parent() == 0 ) )
        return 0;
    if ( scope->scope->scopeType() == Scope::ProjectScope )
        return scope;
    return findSubprojectForScope( dynamic_cast<QMakeScopeItem *>( scope->parent() ) );
}

void TrollProjectWidget::focusInEvent( QFocusEvent * /*e*/ )
{
    switch ( m_lastFocusedView )
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

void TrollProjectWidget::runQMakeRecursive( QMakeScopeItem* proj )
{
    if ( proj->scope->scopeType() == Scope::ProjectScope )
    {
        m_part->startQMakeCommand( proj->scope->projectDir() );
    }
    QMakeScopeItem* item = static_cast<QMakeScopeItem*>( proj->firstChild() );
    while ( item )
    {
        runQMakeRecursive( item );
        item = static_cast<QMakeScopeItem*>( item->nextSibling() );
    }
}

void TrollProjectWidget::slotBuildSelectedFile()
{
    QListViewItem * selectedItem = details->currentItem();
    if ( !selectedItem )
        return ;
    qProjectItem *pvitem = static_cast<qProjectItem*>( selectedItem );
    // Check that it is a file (just in case)
    if ( pvitem->type() != qProjectItem::File )
        return ;
    FileItem *fitem = static_cast<FileItem*>( pvitem );
    buildFile( m_shownSubproject, fitem );
}

void TrollProjectWidget::buildFile( QMakeScopeItem* spitem, FileItem* fitem )
{
    QFileInfo fi( spitem->scope->projectDir() + QString( QChar( QDir::separator() ) ) + spitem->scope->resolveVariables( fitem->text( 0 ) ) );
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName( true );
    kdDebug( 9024 ) << "Compiling " << spitem->scope->resolveVariables( fitem->text( 0 ) )
    << "in dir " << sourceDir
    << " with baseName " << baseName << endl;

    QString buildDir = sourceDir;
    QString target = baseName + ".o";
    if( !spitem->scope->variableValues("OBJECTS_DIR").isEmpty() )
        target = spitem->scope->resolveVariables( spitem->scope->variableValues("OBJECTS_DIR").first() )+ QString( QChar( QDir::separator() ) )+target;
    kdDebug( 9024 ) << "builddir " << buildDir << ", target " << target << endl;

    m_part->mainWindow() ->raiseView( m_part->makeFrontend() ->widget() );
    //    m_part->startMakeCommand(buildDir, target);

    QString buildcmd = constructMakeCommandLine( spitem->scope );
    QString dircmd = "cd " + KProcess::quote( spitem->scope->projectDir() ) + " && " ;
    kdDebug( 9024 ) << "builddir " << spitem->scope->projectDir() << ", cmd " << dircmd + buildcmd + " " + target << endl;
    m_part->queueCmd( spitem->scope->projectDir(), dircmd + buildcmd + " " + target );


    //    startMakeCommand(buildDir, target);

}

TrollProjectWidget::SaveType TrollProjectWidget::dialogSaveBehaviour() const
{
    switch ( DomUtil::readIntEntry( *m_part->projectDom(), "/kdevtrollproject/qmake/savebehaviour", 2 ) )
    {
        case 0:
            return AlwaysSave;
            break;
        case 1:
            return NeverSave;
            break;
        case 2:
        default:
            return Ask;
            break;
    }
}

bool TrollProjectWidget::isTMakeProject()
{
    return m_part->isTMakeProject();
}

void TrollProjectWidget::slotDisableSubproject( QMakeScopeItem* spitem )
{
    m_filesCached = false;
    m_allFilesCache.clear();

    if( spitem->scope->variableValues("TEMPLATE").findIndex("subdirs") != -1 )
    {
        QStringList subdirs = spitem->scope->variableValues( "SUBDIRS" );
        DisableSubprojectDlg dlg( subdirs );
        if( dlg.exec() )
        {
            QStringList values = dlg.selectedProjects();
            QListViewItem* item = spitem->firstChild();
            while( item )
            {
                if( values.findIndex( item->text(0) ) != -1  )
                    delete item;
                item = item->nextSibling();
            }
            spitem->disableSubprojects( values );
            spitem->scope->saveToFile();
            m_shownSubproject = spitem;
            slotOverviewSelectionChanged( m_shownSubproject );
        }
    }else
    {
        QMakeScopeItem* parent = static_cast<QMakeScopeItem*>(spitem->parent());
        parent->disableSubprojects( QStringList( spitem->scope->scopeName() ) );
        delete spitem;
        parent->scope->saveToFile();
        m_shownSubproject = parent;
        slotOverviewSelectionChanged( m_shownSubproject );
    }
}

void TrollProjectWidget::slotProjectDirty(const QString& path)
{
    kdDebug(9024) << "File is dirty:" << path << " using method " << endl;
    if( KMessageBox::warningYesNo(this, i18n("The project file \"%1\" has changed on disk\n(Or you have \"%2\" opened in the editor, which also triggers a reload when you change something in the QMake Manager).\n\nDo you want to reload the it?").arg(path).arg(path), i18n("Project File Changed"), i18n("Reload"), i18n("Do Not Reload"), "trollproject_reload_project_file" ) != KMessageBox::No )
    {
        m_part->dirWatch()->stopScan();
        QListViewItemIterator it(m_rootSubproject);
        QValueList<QMakeScopeItem*> itemstoreload;
        while( it.current() )
        {
            QMakeScopeItem* projectitem = static_cast<QMakeScopeItem*>( it.current() );
            if( projectitem->scope->scopeType() == Scope::ProjectScope
                || projectitem->scope->scopeType() == Scope::IncludeScope )
            {
                QString projectfile = projectitem->scope->projectDir() + QString(QChar(QDir::separator())) + projectitem->scope->fileName();
                if( projectfile == path )
                {
                    itemstoreload.append(projectitem);
                }
            }
            it++;
        }

        QValueList<QMakeScopeItem*>::const_iterator reloadit = itemstoreload.begin();
        for( ; reloadit != itemstoreload.end() ; ++reloadit )
        {
            (*reloadit)->reloadProject();
            if( m_shownSubproject == (*reloadit)  )
            {
                cleanDetailView(*reloadit);
                setupContext();
                buildProjectDetailTree( *reloadit, details );
            }
            if( m_configDlg->isShown() && m_configDlg->currentProjectItem() == (*reloadit) )
            {
                m_configDlg->reject();
                m_configDlg->updateSubproject(m_shownSubproject);
                m_configDlg->show();
            }
        }
        m_part->dirWatch()->startScan();
    }
}


QMakeScopeItem* TrollProjectWidget::currentSubproject()
{
    return m_shownSubproject;
}


#include "trollprojectwidget.moc"

//kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

