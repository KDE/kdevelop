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
#ifndef __GENERICPROJECT_WIDGET_H__
#define __GENERICPROJECT_WIDGET_H__

#include <qvbox.h>
#include <qmap.h>

class QSplitter;
class QListViewItem;
class KDevProject;
class GenericProjectPart;
class OverviewListView;
class DetailsListView;
class BuildGroupItem;
class BuildTargetItem;
class BuildFileItem;
class GenericGroupListViewItem;
class GenericTargetListViewItem;
class GenericFileListViewItem;
class KAction;
class KListView;

class GenericProjectWidget : public QVBox
{
    Q_OBJECT
public:
    GenericProjectWidget(GenericProjectPart *part);
    virtual ~GenericProjectWidget();

    GenericGroupListViewItem *addGroup( BuildGroupItem* group );
    void addTarget( BuildTargetItem* target );
    void addFile( BuildFileItem* file );

    void takeGroup(GenericGroupListViewItem *it);
    void takeTarget(GenericTargetListViewItem *it);
    void takeFile(GenericFileListViewItem *it);

    BuildGroupItem* activeGroup();
    BuildTargetItem* activeTarget();

signals:
    void groupSelected( BuildGroupItem* groupItem );
    void targetSelected( BuildTargetItem* targetItem );
    void fileSelected( BuildFileItem* fileItem );
    void groupExecuted( BuildGroupItem* groupItem );
    void targetExecuted( BuildTargetItem* targetItem );
    void fileExecuted( BuildFileItem* fileItem );

public slots:
    void showDetails( BuildGroupItem* groupItem );
    void showTargetDetails( BuildTargetItem* targetItem );
    void showFileDetails( BuildFileItem* fileItem );

private slots:
    void slotMainGroupChanged( BuildGroupItem* mainGroup );
    void slotItemSelected( QListViewItem* item );
    void slotItemExecuted( QListViewItem* item );
    void showGroupContextMenu(KListView *l, QListViewItem *i, const QPoint &p);
    void showDetailContextMenu(KListView *l, QListViewItem *i, const QPoint &p);

    void slotNewGroup();
    void slotNewTarget();
    void slotBuild();
    void slotBuildGroup();
    void slotBuildTarget();
    void slotBuildFile();
    void slotInstall();
    void slotInstallGroup();
    void slotInstallTarget();
    void slotExecute();
    void slotExecuteGroup();
    void slotExecuteTarget();
    void slotNewFile();
    void slotAddFiles();
    void slotDeleteGroup();
    void slotDeleteTarget();
    void slotDeleteFile();
    void slotDeleteTargetOrFile();
    void slotConfigureGroup();
    void slotConfigureTarget();
    void slotConfigureFile();
    void slotConfigureTargetOrFile();
    void slotClean();
    void slotCleanGroup();
    void slotCleanTarget();

private:
    void initOverviewListView( QSplitter* splitter );
    void initDetailsListView( QSplitter* splitter );
    void fillGroupItem( BuildGroupItem* group, GenericGroupListViewItem* item );
    void fillTarget( BuildTargetItem* target, GenericTargetListViewItem* item );
    void initActions();

private:
    GenericProjectPart* m_part;
    OverviewListView* m_overviewListView;
    DetailsListView* m_detailsListView;
    BuildGroupItem* m_activeGroup;
    BuildTargetItem* m_activeTarget;

    QMap<BuildGroupItem*, GenericGroupListViewItem*> m_groupToItem;
    QMap<BuildTargetItem*, GenericTargetListViewItem*> m_targetToItem;
    QMap<BuildFileItem*, GenericFileListViewItem*> m_fileToItem;

    KAction *newGroupAction;
    KAction *newTargetAction;
    KAction *buildGroupAction;
    KAction *buildAction;
    KAction *buildTargetAction;
    KAction *buildFileAction;
    KAction *installGroupAction;
    KAction *installAction;
    KAction *newFileAction;
    KAction *addFilesAction;
    KAction *deleteGroupAction;
    KAction *deleteTargetAction;
    KAction *deleteFileAction;
    KAction *configureGroupAction;
    KAction *configureTargetAction;
    KAction *configureFileAction;
    KAction *executeAction;
    KAction *executeGroupAction;
    KAction *executeTargetAction;
    KAction *cleanAction;
    KAction *cleanGroupAction;
    KAction *cleanTargetAction;
};


#endif
