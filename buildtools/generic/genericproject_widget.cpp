/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "genericproject_part.h"
#include "genericproject_widget.h"
#include "overviewlistview.h"
#include "detailslistview.h"
#include "genericlistviewitem.h"

#include "kdevcore.h"
#include "kdevpartcontroller.h"

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <klistview.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>

#include <qsplitter.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>
#include <qheader.h>
#include <qdir.h>
#include <qfile.h>

#include "kdevbuildsystem.h"
#include "removesubprojectdialog.h"
#include "kdevcreatefile.h"

GenericProjectWidget::GenericProjectWidget(GenericProjectPart *part)
    : QVBox( 0, "GenericProjectWidget" ), m_part( part ), m_activeGroup( 0 ), m_activeTarget( 0 )
{
    QSplitter* splitter = new QSplitter( Vertical, this );
    initOverviewListView( splitter );
    initDetailsListView( splitter );

    initActions();

    connect( m_part, SIGNAL(mainGroupChanged(BuildGroupItem*)),
        this, SLOT(slotMainGroupChanged(BuildGroupItem*)) );
}

GenericProjectWidget::~GenericProjectWidget()
{
}

void GenericProjectWidget::initOverviewListView( QSplitter * splitter )
{
    QVBox* vbox = new QVBox( splitter );

    QHBox* buttonBox = new QHBox( vbox );
    buttonBox->setMargin( 2 );
    buttonBox->setSpacing( 2 );

    QToolButton* btn = 0;

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("group_new") );
    QToolTip::add( btn, i18n("Add new group") );
    QWhatsThis::add( btn, i18n("<b>Add new group</b><p>Adds a new group of targets and files. "
        "Group is usually a subdirectory in the project directory but that depends on an underlying build system.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotNewGroup()));

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("targetnew_kdevelop") );
    QToolTip::add( btn, i18n("Add new target") );
    QWhatsThis::add( btn, i18n("<b>Add new target</b><p>Adds a new target. "
        "Target is usually an application, static or shared library but that depends on an underlying build system.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotNewTarget()));

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("launch") );
    QToolTip::add( btn, i18n("Build") );
    QWhatsThis::add( btn, i18n("<b>Build</b><p>Executes all commands necessary to build the current group.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotBuildGroup()));

    QWidget *spacer1 = new QWidget( buttonBox );
    buttonBox->setStretchFactor( spacer1, 1 );

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("configure") );
    QToolTip::add( btn, i18n("Configure group") );
    QWhatsThis::add( btn, i18n("<b>Configure group</b><p>Opens a group configuration dialog supplied by a build system plugin.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotConfigureGroup()));

    buttonBox->setMaximumHeight( btn->height() );

    m_overviewListView = new OverviewListView( m_part, vbox, "GroupListView" );

    m_overviewListView->setResizeMode( QListView::LastColumn );
    m_overviewListView->setSorting( -1 );
    m_overviewListView->header()->hide();
    m_overviewListView->addColumn( QString::null );

    connect( m_overviewListView, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotItemSelected(QListViewItem*)) );

    connect( this, SIGNAL(groupSelected(BuildGroupItem*)),
            this, SLOT(showDetails(BuildGroupItem*)) );

    connect(m_overviewListView, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
            this, SLOT(showGroupContextMenu(KListView *, QListViewItem *, const QPoint &)));
}

void GenericProjectWidget::initDetailsListView( QSplitter * splitter )
{
    QVBox* vbox = new QVBox( splitter );

    QHBox* buttonBox = new QHBox( vbox );
    buttonBox->setMargin( 2 );
    buttonBox->setSpacing( 2 );

    QToolButton* btn = 0;

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("filenew") );
    QToolTip::add( btn, i18n("New file") );
    QWhatsThis::add( btn, i18n("<b>New file</b><p>Creates a new file and adds it to a current target.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotNewFile()));

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("fileimport") );
    QToolTip::add( btn, i18n("Add files") );
    QWhatsThis::add( btn, i18n("<b>Add files</b><p>Adds existing files to a current target.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotAddFiles()));

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("editdelete") );
    QToolTip::add( btn, i18n("Remove target or file") );
    QWhatsThis::add( btn, i18n("<b>Remove target or file</b><p>Removes current target or file.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotDeleteTargetOrFile()));

/*    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("editdelete") );
    QToolTip::add( btn, i18n("Remove file") );
    QWhatsThis::add( btn, i18n("<b></b><p>") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotDeleteFile()));
*/
    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("launch") );
    QToolTip::add( btn, i18n("Build target") );
    QWhatsThis::add( btn, i18n("<b>Build target</b><p>Executes all commands necessary to build the current target.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotBuildTarget()));


    QWidget *spacer1 = new QWidget( buttonBox );
    buttonBox->setStretchFactor( spacer1, 1 );

    btn = new QToolButton( buttonBox );
    btn->setPixmap( SmallIcon("configure") );
    QToolTip::add( btn, i18n("Configure target or file") );
    QWhatsThis::add( btn, i18n("<b>Configure target or file</b><p>Opens a target or file configuration dialog supplied by a build system plugin.") );
    connect(btn, SIGNAL(clicked()), this, SLOT(slotConfigureTargetOrFile()));

    buttonBox->setMaximumHeight( btn->height() );

    m_detailsListView = new DetailsListView( m_part, vbox, "DetailsListView" );

    m_detailsListView->setResizeMode( QListView::LastColumn );
    m_detailsListView->setSorting( -1 );
    m_detailsListView->header()->hide();
    m_detailsListView->addColumn( QString::null );

    connect( m_detailsListView, SIGNAL(clicked(QListViewItem*)),
        this, SLOT(slotItemSelected(QListViewItem*)) );
    connect( m_detailsListView, SIGNAL(executed(QListViewItem*)),
        this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( m_detailsListView, SIGNAL(returnPressed(QListViewItem*)),
        this, SLOT(slotItemExecuted(QListViewItem*)) );

    connect( this, SIGNAL(targetSelected(BuildTargetItem*)),
        this, SLOT(showTargetDetails(BuildTargetItem*)) );
    connect( this, SIGNAL(fileExecuted(BuildFileItem*)),
        this, SLOT(showFileDetails(BuildFileItem*)) );
    connect(m_detailsListView, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
        this, SLOT(showDetailContextMenu(KListView *, QListViewItem *, const QPoint &)));

}

void GenericProjectWidget::slotMainGroupChanged( BuildGroupItem * mainGroup )
{
    m_overviewListView->clear();

    m_groupToItem.clear();
    m_targetToItem.clear();
    m_fileToItem.clear();

    if( !mainGroup )
        return;

    GenericGroupListViewItem* mainItem = new GenericGroupListViewItem( m_overviewListView, mainGroup );
    mainItem->setOpen( true );
    fillGroupItem( mainGroup, mainItem );
}

void GenericProjectWidget::fillGroupItem( BuildGroupItem * group, GenericGroupListViewItem * item )
{
    m_groupToItem.insert( group, item );

    QValueList<BuildGroupItem*> groups = group->groups();
    QValueListIterator<BuildGroupItem*> it = groups.begin();
    while( it != groups.end() ){
        GenericGroupListViewItem* createdItem = new GenericGroupListViewItem( item, *it );
        createdItem->setOpen( (*it)->groups().size() > 0 );
        fillGroupItem( *it, createdItem );

        ++it;
    }
}

void GenericProjectWidget::slotItemSelected( QListViewItem * item )
{
    GenericGroupListViewItem* groupItem = dynamic_cast<GenericGroupListViewItem*>( item );
    GenericTargetListViewItem* targetItem = dynamic_cast<GenericTargetListViewItem*>( item );
//  GenericFileListViewItem* fileItem = dynamic_cast<GenericFileListViewItem*>( item );

    if( groupItem && groupItem->groupItem() )
        emit groupSelected( groupItem->groupItem() );
    else if( targetItem && targetItem->targetItem() ){
        kdDebug() << "set active target" << endl;
        m_activeTarget = targetItem->targetItem();
        emit targetSelected( m_activeTarget );
    }
//    else if( fileItem && m_itemToFile.contains(fileItem) )
//	emit fileSelected( m_itemToFile[fileItem] );
}

void GenericProjectWidget::showDetails( BuildGroupItem * groupItem )
{
    m_activeGroup = groupItem;
    kdDebug() << "unset active target" << endl;
    m_activeTarget = 0;

    m_detailsListView->clear();

    m_targetToItem.clear();
    m_fileToItem.clear();

    if( !groupItem )
	return;

    QValueList<BuildTargetItem*> targets = groupItem->targets();
    QValueListIterator<BuildTargetItem*> it = targets.begin();
    while( it != targets.end() ){
        GenericTargetListViewItem* createdItem = new GenericTargetListViewItem( m_detailsListView, *it );
        m_targetToItem.insert( *it, createdItem );
        fillTarget( *it, createdItem );
        createdItem->setOpen( true );
        ++it;
    }
}

void GenericProjectWidget::fillTarget( BuildTargetItem * target, GenericTargetListViewItem * targetItem )
{
    QValueList<BuildFileItem*> files = target->files();
    QValueListIterator<BuildFileItem*> it = files.begin();
    while( it != files.end() ){
	GenericFileListViewItem* createdItem = new GenericFileListViewItem( targetItem, *it );
	m_fileToItem.insert( *it, createdItem );
	++it;
    }
}

BuildGroupItem * GenericProjectWidget::activeGroup( )
{
    return m_activeGroup;
}

GenericGroupListViewItem * GenericProjectWidget::addGroup( BuildGroupItem * group )
{
    if( !group )
        return 0;

    GenericGroupListViewItem* createdItem = 0;
    if( group->parentGroup() && m_groupToItem.contains(group->parentGroup()) ){
        kdDebug() << "creating GenericGroupListViewItem from parent group" << endl;
        createdItem = new GenericGroupListViewItem( m_groupToItem[group->parentGroup()], group );
        m_groupToItem.insert( group, createdItem );
    } else if( group->parentGroup() ){
        kdDebug() << "creating GenericGroupListViewItem from parent group (wo map)" << endl;
        addGroup( group->parentGroup() );
        createdItem = new GenericGroupListViewItem( m_groupToItem[group->parentGroup()], group );
        m_groupToItem.insert( group, createdItem );
        m_groupToItem[group->parentGroup()]->setExpandable( true );
    } else {
        kdDebug() << "creating GenericGroupListViewItem standalone" << endl;
        createdItem = new GenericGroupListViewItem( m_overviewListView, group );
        m_groupToItem.insert( group, createdItem );
    }
    return createdItem;
}

void GenericProjectWidget::addTarget( BuildTargetItem * target )
{
    if( !target || !target->parentGroup() || activeGroup() != target->parentGroup() )
        return;

    if(  m_groupToItem.contains(target->parentGroup()) ){
//        GenericTargetListViewItem* createdItem = new GenericTargetListViewItem( m_groupToItem[target->parentGroup()], target );
        GenericTargetListViewItem* createdItem = new GenericTargetListViewItem( m_detailsListView, target );
        m_detailsListView->takeItem(createdItem);
        m_targetToItem.insert( target, createdItem );

        showDetails(target->parentGroup());
    }
}

void GenericProjectWidget::addFile( BuildFileItem * file )
{
    if( !file || !file->parentTarget() || file->parentTarget()->parentGroup() != activeGroup() )
	return;

    if( m_targetToItem.contains(file->parentTarget()) ){
	GenericFileListViewItem* createdItem = new GenericFileListViewItem( m_targetToItem[file->parentTarget()], file );
	m_fileToItem.insert( file, createdItem );
    }
}

BuildTargetItem * GenericProjectWidget::activeTarget( )
{
    return m_activeTarget;
}

void GenericProjectWidget::showTargetDetails( BuildTargetItem * /*targetItem*/ )
{
}

void GenericProjectWidget::showFileDetails( BuildFileItem * fileItem )
{
    kdDebug() << "GenericProjectWidget::showFileDetails" << endl;
    m_part->partController()->editDocument(fileItem->url());
}

void GenericProjectWidget::initActions( )
{
    newGroupAction = new KAction (i18n("New Group"), "group_new", 0,
        this, SLOT( slotNewGroup() ), m_part->actionCollection(), "new_group" );
    newGroupAction->setToolTip(i18n("New group"));
    newGroupAction->setWhatsThis(i18n("<b>New group</b><p>Adds a new group of targets and files. "
        "Group is usually a subdirectory in the project directory but that depends on an underlying build system."));
    newTargetAction = new KAction (i18n("New Target"), "targetnew_kdevelop", 0,
        this, SLOT( slotNewTarget() ), m_part->actionCollection(), "new_target" );
    newTargetAction->setToolTip(i18n("New target"));
    newTargetAction->setWhatsThis(i18n("<b>New target</b><p>Adds a new target. "
        "Target is usually an application, static or shared library but that depends on an underlying build system."));
    buildGroupAction = new KAction (i18n("Build Group"), "launch", 0,
        this, SLOT( slotBuildGroup() ), m_part->actionCollection(), "build_group" );
    buildGroupAction->setToolTip(i18n("Build group"));
    buildGroupAction->setWhatsThis(i18n("<b>Build group</b><p>Executes all commands necessary to build the current group."));
    buildAction = new KAction (i18n("Build"), "launch", 0,
        this, SLOT( slotBuild() ), m_part->actionCollection(), "build" );
    buildAction->setToolTip(i18n("Build project"));
    buildAction->setWhatsThis(i18n("<b>Build project</b><p>Executes all commands necessary to build the project."));
    buildTargetAction = new KAction (i18n("Build Target"), "launch", 0,
        this, SLOT( slotBuildTarget() ), m_part->actionCollection(), "build_target" );
    buildTargetAction->setToolTip(i18n("Build target"));
    buildTargetAction->setWhatsThis(i18n("<b>Build target</b><p>Executes all commands necessary to build the current target."));
    buildFileAction = new KAction (i18n("Build File"), "launch", 0,
        this, SLOT( slotBuildFile() ), m_part->actionCollection(), "build_file" );
    buildFileAction->setToolTip(i18n("Build file"));
    buildFileAction->setWhatsThis(i18n("<b>Build file</b><p>Executes all commands necessary to build the current file."));
    installGroupAction = new KAction (i18n("Install Group"), 0, 0,
        this, SLOT( slotInstallGroup() ), m_part->actionCollection(), "install_group" );
    installGroupAction->setToolTip(i18n("Install group"));
    installGroupAction->setWhatsThis(i18n("<b>Install group</b><p>Executes all commands necessary to install the current group."));
    installAction = new KAction (i18n("Install"), 0, 0,
        this, SLOT( slotInstall() ), m_part->actionCollection(), "install" );
    installAction->setToolTip(i18n("Install"));
    installAction->setWhatsThis(i18n("<b>Install</b><p>Executes all commands necessary to install the project."));
    newFileAction = new KAction (i18n("New File"), "filenew", 0,
        this, SLOT( slotNewFile() ), m_part->actionCollection(), "new_file" );
    newFileAction->setToolTip(i18n("New file"));
    newFileAction->setWhatsThis(i18n("<b>New file</b><p>Creates a new file and adds it to a current target."));
    addFilesAction = new KAction (i18n("Add Files"), "fileimport", 0,
        this, SLOT( slotAddFiles() ), m_part->actionCollection(), "add_files" );
    addFilesAction->setToolTip(i18n("Add files"));
    addFilesAction->setWhatsThis(i18n("<b>Add files</b><p>Adds existing files to a current target."));
    deleteGroupAction = new KAction (i18n("Remove Group"), 0, 0,
        this, SLOT( slotDeleteGroup() ), m_part->actionCollection(), "remove_group" );
    deleteGroupAction->setToolTip(i18n("Remove group"));
    deleteGroupAction->setWhatsThis(i18n("<b>Remove group</b><p>Removes current group."));
    deleteTargetAction = new KAction (i18n("Remove Target"), 0, 0,
        this, SLOT( slotDeleteTarget() ), m_part->actionCollection(), "remove_target" );
    deleteTargetAction->setToolTip(i18n("Remove target"));
    deleteTargetAction->setWhatsThis(i18n("<b>Remove target</b><p>Removes current target."));
    deleteFileAction = new KAction (i18n("Remove File"), 0, 0,
        this, SLOT( slotDeleteFile() ), m_part->actionCollection(), "remove_file" );
    deleteFileAction->setToolTip(i18n("Remove file"));
    deleteFileAction->setWhatsThis(i18n("<b>Remove file</b><p>Removes current file."));
    configureGroupAction = new KAction (i18n("Options..."), "configure", 0,
        this, SLOT( slotConfigureGroup() ), m_part->actionCollection(), "configure_group" );
    configureGroupAction->setToolTip(i18n("Options"));
    configureGroupAction->setWhatsThis(i18n("<b>Options</b><p>Opens a group configuration dialog supplied by a build system plugin."));
    configureTargetAction = new KAction (i18n("Options..."), "configure", 0,
        this, SLOT( slotConfigureTarget() ), m_part->actionCollection(), "configure_target" );
    configureTargetAction->setToolTip(i18n("Options"));
    configureTargetAction->setWhatsThis(i18n("<b>Options</b><p>Opens a target configuration dialog supplied by a build system plugin."));
    configureFileAction = new KAction (i18n("Options..."), "configure", 0,
        this, SLOT( slotConfigureFile() ), m_part->actionCollection(), "configure_item" );
    configureFileAction->setToolTip(i18n("Options"));
    configureFileAction->setWhatsThis(i18n("<b>Options</b><p>Opens a file configuration dialog supplied by a build system plugin."));
    executeAction = new KAction (i18n("Execute"), "exec", 0,
        this, SLOT( slotExecute() ), m_part->actionCollection(), "execute" );
    executeAction->setToolTip(i18n("Execute project"));
    executeAction->setWhatsThis(i18n("<b>Execute project</b><p>Executes the main project executable or the current application target."));
    executeGroupAction = new KAction (i18n("Execute"), "exec", 0,
        this, SLOT( slotExecuteGroup() ), m_part->actionCollection(), "execute_group" );
    executeGroupAction->setToolTip(i18n("Execute group"));
    executeGroupAction->setWhatsThis(i18n("<b>Execute group</b><p>Executes the current application target in a group."));
    executeTargetAction = new KAction (i18n("Execute"), "exec", 0,
        this, SLOT( slotExecuteTarget() ), m_part->actionCollection(), "execute_target" );
    executeTargetAction->setToolTip(i18n("Execute target"));
    executeTargetAction->setWhatsThis(i18n("<b>Execute target</b><p>Executes application target."));
    cleanAction = new KAction (i18n("Clean"), 0, 0,
        this, SLOT( slotClean() ), m_part->actionCollection(), "clean" );
    cleanAction->setToolTip(i18n("Clean project"));
    cleanAction->setWhatsThis(i18n("<b>Clean project</b><p>Executes all commands necessary to clean the project."));
    cleanGroupAction = new KAction (i18n("Clean"), 0, 0,
        this, SLOT( slotCleanGroup() ), m_part->actionCollection(), "clean_group" );
    cleanGroupAction->setToolTip(i18n("Clean group"));
    cleanGroupAction->setWhatsThis(i18n("<b>Clean group</b><p>Executes all commands necessary to clean the current group."));
    cleanTargetAction = new KAction (i18n("Clean"), 0, 0,
        this, SLOT( slotCleanTarget() ), m_part->actionCollection(), "clean_target" );
    cleanTargetAction->setToolTip(i18n("Clean target"));
    cleanTargetAction->setWhatsThis(i18n("<b>Clean target</b><p>Executes all commands necessary to clean the current target."));

}

void GenericProjectWidget::showGroupContextMenu( KListView * l, QListViewItem * i, const QPoint & p )
{
    if (( !l ) || (!i) )
        return ;

    KPopupMenu popup( this );
    popup.insertTitle(i18n( "Group: %1" ).arg( i->text( 0 ) ));
    configureGroupAction->plug(&popup);
    popup.insertSeparator();
    newGroupAction->plug(&popup);
    newTargetAction->plug(&popup);
    popup.insertSeparator();
    deleteGroupAction->plug(&popup);
    popup.insertSeparator();
    buildGroupAction->plug(&popup);
    cleanGroupAction->plug(&popup);
    executeGroupAction->plug(&popup);

    popup.exec(p);
}

void GenericProjectWidget::showDetailContextMenu( KListView * l, QListViewItem * i, const QPoint & p )
{
    if (( !l ) || (!i) )
        return ;

    GenericTargetListViewItem *t = dynamic_cast<GenericTargetListViewItem*>(i);
    GenericFileListViewItem *f = dynamic_cast<GenericFileListViewItem*>(i);

    if (t)
    {
        KPopupMenu popup( this );
	popup.insertTitle(i18n( "Target: %1" ).arg( t->text( 0 ) ));

        configureTargetAction->plug(&popup);
        popup.insertSeparator();
        newFileAction->plug(&popup);
        addFilesAction->plug(&popup);
        popup.insertSeparator();
        deleteTargetAction->plug(&popup);
        popup.insertSeparator();
        buildTargetAction->plug(&popup);
        cleanTargetAction->plug(&popup);
        executeTargetAction->plug(&popup);

        popup.exec(p);
    }
    if (f)
    {
        KPopupMenu popup( this );
	popup.insertTitle(i18n( "File: %1" ).arg( f->text( 0 ) ));

        configureFileAction->plug(&popup);
        popup.insertSeparator();
        deleteFileAction->plug(&popup);

        popup.exec(p);
    }
}



//Important slot definitions:

void GenericProjectWidget::slotNewGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    bool ok;
    QString groupName = KInputDialog::getText(i18n("Group Name"), i18n("Enter the group name:"), "", &ok, this );
    if (!ok)
        return;
    QDir dir;
    if (!dir.mkdir( QDir::cleanDirPath(m_part->projectDirectory() + "/" + git->groupItem()->path() + "/" + groupName)) )
        return;
    BuildGroupItem *bit = new BuildGroupItem(groupName, git->groupItem());
    addGroup(bit);
}

void GenericProjectWidget::slotNewTarget( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    bool ok;
    QString targetName = KInputDialog::getText(i18n("Target Name"), i18n("Enter the target name:"), "", &ok, this );
    if (!ok)
        return;
    BuildTargetItem *tit = new BuildTargetItem(targetName, git->groupItem());
    addTarget(tit);
}

void GenericProjectWidget::slotBuild( )
{
    m_part->buildSystem()->build();
}

void GenericProjectWidget::slotBuildGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    m_part->buildSystem()->build(git->groupItem());
}

void GenericProjectWidget::slotBuildTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;
    m_part->buildSystem()->build(tit->targetItem());
}

void GenericProjectWidget::slotBuildFile( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericFileListViewItem *fit = dynamic_cast<GenericFileListViewItem *>(m_detailsListView->currentItem());
    if (!fit)
        return;
    m_part->buildSystem()->build(fit->fileItem());
}

void GenericProjectWidget::slotInstall( )
{
    m_part->buildSystem()->install();
}

void GenericProjectWidget::slotInstallGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    m_part->buildSystem()->install(git->groupItem());
}

void GenericProjectWidget::slotInstallTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;
    m_part->buildSystem()->install(tit->targetItem());
}

void GenericProjectWidget::slotExecute( )
{
    m_part->buildSystem()->execute();
}

void GenericProjectWidget::slotExecuteGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    m_part->buildSystem()->execute(git->groupItem());
}

void GenericProjectWidget::slotExecuteTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;
    m_part->buildSystem()->execute(tit->targetItem());
}

void GenericProjectWidget::slotNewFile( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;

    KDevCreateFile * createFileSupport = m_part->createFileSupport();
    if (createFileSupport)
    {
        kdDebug() << "GenericProjectWidget::slotNewFile " << tit->targetItem()->parentGroup()->name() << endl;
        KDevCreateFile::CreatedFile crFile =
            createFileSupport->createNewFile(QString::null, QDir::cleanDirPath(m_part->projectDirectory() + "/" + tit->targetItem()->parentGroup()->path()));
        kdDebug() << "status for " << QDir::cleanDirPath(m_part->projectDirectory() + "/" + tit->targetItem()->parentGroup()->path()) << " is " << crFile.status << endl;
//        KURL url;
//        url.setPath(QDir::cleanDirPath(crFile.dir + "/" + crFile.filename));
//        BuildFileItem *fit = new BuildFileItem(url, tit->targetItem());
//        addFile(fit);
    }
}

void GenericProjectWidget::slotAddFiles( )
{
    QString startDir = m_part->projectDirectory();
    if (activeTarget())
        startDir += "/" + activeTarget()->path();
    else if (activeGroup())
        startDir += "/" + activeGroup()->path();

    QStringList fileList = KFileDialog::getOpenFileNames( startDir );
    if( fileList.isEmpty() )
	return;

    QStringList lst;
    for( QStringList::Iterator it=fileList.begin(); it!=fileList.end(); ++it )
    {
	QString absFileName = *it;
	if( !absFileName.startsWith(m_part->projectDirectory()) )
	    continue;

	lst << absFileName.mid( m_part->projectDirectory().length() + 1 );
    }

    m_part->addFiles( lst );
}

void GenericProjectWidget::slotDeleteGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;

    RemoveSubprojectDialog dia(i18n("Remove Group"), i18n("Remove group?"), this);
    if (dia.exec() == QDialog::Accepted)
    {
        if (dia.removeFromDisk())
        {
            QDir d;
            d.rmdir( QDir::cleanDirPath(m_part->projectDirectory() + "/" + git->groupItem()->path()));
        }
        takeGroup(git);
    }
}

void GenericProjectWidget::slotDeleteTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;

    if (KMessageBox::questionYesNo(this, i18n("Remove target?")) == KMessageBox::Yes)
    {
        takeTarget(tit);
    }
}

void GenericProjectWidget::slotDeleteFile( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericFileListViewItem *fit = dynamic_cast<GenericFileListViewItem *>(m_detailsListView->currentItem());
    if (!fit)
        return;

    RemoveSubprojectDialog dia(i18n("Remove File"), i18n("Remove file?"), this);
    if (dia.exec() == QDialog::Accepted)
    {
        if (dia.removeFromDisk())
        {
            kdDebug() << "GenericProjectWidget::slotDeleteFile " << fit->fileItem()->url().url() << endl;
            KIO::NetAccess::del(fit->fileItem()->url(), (QWidget*)0);
        }
        takeFile(fit);
    }
}

void GenericProjectWidget::slotConfigureGroup( )
{
    kdDebug() << "GenericProjectWidget::slotConfigureGroup 1" << endl;
    if (!m_overviewListView->currentItem())
        return;
    kdDebug() << "GenericProjectWidget::slotConfigureGroup 2" << endl;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    kdDebug() << "GenericProjectWidget::slotConfigureGroup 3" << endl;

    KDialogBase *dia = new KDialogBase(KDialogBase::Tabbed, i18n("Group Options"),
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this);
    kdDebug() << "GenericProjectWidget::slotConfigureGroup 4" << endl;

    m_part->buildSystem()->configureBuildItem(dia, git->buildItem());
    kdDebug() << "GenericProjectWidget::slotConfigureGroup 5" << endl;
}

void GenericProjectWidget::slotConfigureTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;

    KDialogBase *dia = new KDialogBase(KDialogBase::Tabbed, i18n("Target Options"),
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this);

    m_part->buildSystem()->configureBuildItem(dia, tit->buildItem());
}

void GenericProjectWidget::slotConfigureFile( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericFileListViewItem *fit = dynamic_cast<GenericFileListViewItem *>(m_detailsListView->currentItem());
    if (!fit)
        return;

    KDialogBase *dia = new KDialogBase(KDialogBase::Tabbed, i18n("File Options"),
        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this);

    m_part->buildSystem()->configureBuildItem(dia, fit->buildItem());
}

void GenericProjectWidget::slotClean( )
{
    m_part->buildSystem()->clean();
}

void GenericProjectWidget::slotCleanGroup( )
{
    if (!m_overviewListView->currentItem())
        return;
    GenericGroupListViewItem *git = dynamic_cast<GenericGroupListViewItem *>(m_overviewListView->currentItem());
    if (!git)
        return;
    m_part->buildSystem()->clean(git->groupItem());
}

void GenericProjectWidget::slotCleanTarget( )
{
    if (!m_detailsListView->currentItem())
        return;
    GenericTargetListViewItem *tit = dynamic_cast<GenericTargetListViewItem *>(m_detailsListView->currentItem());
    if (!tit)
        return;
    m_part->buildSystem()->clean(tit->targetItem());
}


void GenericProjectWidget::takeGroup( GenericGroupListViewItem * it )
{
    //@todo adymo: allow to remove nested subdirs
    BuildGroupItem *group = it->groupItem();
    if (group->groups().count() == 0)
    {
        m_groupToItem.remove( group );
        delete group;
        delete it;
    }
}

void GenericProjectWidget::takeTarget( GenericTargetListViewItem * item )
{
    kdDebug() << "============> takeTarget" << endl;

    BuildTargetItem *target = item->targetItem();
    if( !target ){
	kdDebug() << "============> no target!!!!!" << endl;
	return;
    }

    QStringList fileList;
    QValueList<BuildFileItem*> files = target->files();
    for( QValueList<BuildFileItem*>::Iterator it=files.begin(); it!=files.end(); ++ it )
    {
	QString path = (*it)->url().path();
	kdDebug() << "============> remove: " << path << endl;
	if( !path.startsWith(m_part->projectDirectory()) )
	    continue;

	fileList << path.mid( m_part->projectDirectory().length() + 1 );
    }

    delete item;
    m_targetToItem.remove( target );
    delete target;

    kdDebug() << "===========> remove files: " << fileList.join( ", " );
}

void GenericProjectWidget::takeFile( GenericFileListViewItem * it )
{
    BuildFileItem *file = it->fileItem();
    m_fileToItem.remove( file );
    delete file;
    delete it;
}

void GenericProjectWidget::slotItemExecuted( QListViewItem * item )
{
    GenericGroupListViewItem* groupItem = dynamic_cast<GenericGroupListViewItem*>( item );
    GenericTargetListViewItem* targetItem = dynamic_cast<GenericTargetListViewItem*>( item );
    GenericFileListViewItem* fileItem = dynamic_cast<GenericFileListViewItem*>( item );

    if( groupItem && groupItem->groupItem() )
        emit groupExecuted( groupItem->groupItem() );
    else if( targetItem && targetItem->targetItem() ){
        kdDebug() << "set active target while execute" << endl;
        m_activeTarget = targetItem->targetItem();
        emit targetExecuted( m_activeTarget );
    }
    else if( fileItem )
    {
        kdDebug() << "emit fileExecuted()" << endl;
        emit fileExecuted( fileItem->fileItem() );
    }
}

void GenericProjectWidget::slotDeleteTargetOrFile( )
{
    QListViewItem * item = m_detailsListView->currentItem();
    if (!item)
        return;
    GenericTargetListViewItem* targetItem = dynamic_cast<GenericTargetListViewItem*>( item );
    GenericFileListViewItem* fileItem = dynamic_cast<GenericFileListViewItem*>( item );
    if (targetItem)
        slotDeleteTarget();
    else if (fileItem)
        slotDeleteFile();
}

void GenericProjectWidget::slotConfigureTargetOrFile( )
{
    QListViewItem * item = m_detailsListView->currentItem();
    if (!item)
        return;
    GenericTargetListViewItem* targetItem = dynamic_cast<GenericTargetListViewItem*>( item );
    GenericFileListViewItem* fileItem = dynamic_cast<GenericFileListViewItem*>( item );
    if (targetItem)
        slotConfigureTarget();
    else if (fileItem)
        slotConfigureFile();
}

#include "genericproject_widget.moc"
