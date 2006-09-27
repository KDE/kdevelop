/***************************************************************************
*   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
*   jsgaarde@tdcspace.dk                                                  *
*   Copyright (C) 2002-2003 by Alexander Dymo                             *
*   cloudtemple@mksat.net                                                 *
*   Copyright (C) 2003 by Thomas Hasart                                   *
*   thasart@gmx.de                                                        *
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef PROJECTCONFIGURATIONDLG_H
#define PROJECTCONFIGURATIONDLG_H

#include "projectconfigurationdlgbase.h"
//#include "trollprojectwidget.h"
#include <kdeversion.h>
#include <qlistview.h>

class QMakeScopeItem;
class qProjectItem;
class ProjectConfigurationDlg;
class TrollProjectWidget;

class InsideCheckListItem : public QCheckListItem
{
public:
    InsideCheckListItem( QListView *parent, QMakeScopeItem *item, ProjectConfigurationDlg *config );

    InsideCheckListItem( QListView *parent, QListViewItem *after, QMakeScopeItem *item, ProjectConfigurationDlg *config );
    QMakeScopeItem *prjItem;
    ProjectConfigurationDlg *m_config;

protected:
    virtual void stateChange ( bool state );
};


class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
public:
    ProjectConfigurationDlg( QListView *_prjList, TrollProjectWidget* _prjWidget, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ProjectConfigurationDlg();
    void updateControls();
    void updateSubproject( QMakeScopeItem* _item );

public slots:
    //  virtual void           radioLibrarytoggled(bool);
    virtual void updateProjectConfiguration();
    virtual void browseTargetPath();


    virtual void buildorderMoveUpClicked();
    virtual void buildorderMoveDownClicked();

    virtual void insideIncMoveUpClicked();
    virtual void insideIncMoveDownClicked();
    virtual void outsideIncMoveUpClicked();
    virtual void outsideIncMoveDownClicked();
    virtual void outsideIncAddClicked();
    virtual void outsideIncRemoveClicked();
    virtual void outsideIncEditClicked();

    virtual void insideLibMoveUpClicked();
    virtual void insideLibMoveDownClicked();
    virtual void outsideLibMoveUpClicked();
    virtual void outsideLibMoveDownClicked();
    virtual void outsideLibAddClicked();
    virtual void outsideLibRemoveClicked();
    virtual void outsideLibEditClicked();

    virtual void outsideLibDirMoveUpClicked();
    virtual void outsideLibDirMoveDownClicked();
    virtual void outsideLibDirAddClicked();
    virtual void outsideLibDirRemoveClicked();
    virtual void outsideLibDirEditClicked();

    virtual void extAdd_button_clicked();
    virtual void extEdit_button_clicked();
    virtual void extMoveDown_button_clicked();
    virtual void extMoveUp_button_clicked();
    virtual void extRemove_button_clicked();
    virtual void intMoveDown_button_clicked();
    virtual void intMoveUp_button_clicked();

    virtual void addCustomValueClicked();
    virtual void removeCustomValueClicked();
    virtual void editCustomValueClicked();
    virtual void upCustomValueClicked();
    virtual void downCustomValueClicked();

    virtual void newCustomVariableActive();

    virtual void groupLibrariesChanged( int );
    virtual void groupRequirementsChanged( int );
    virtual void groupTemplateChanged( int );
    virtual void targetInstallChanged( bool );
    virtual void accept();
    virtual void reject();
    virtual void apply();
    virtual void activateApply( int );
    virtual void activateApply( const QString& );


    void updateIncludeControl();
    void updateLibControls();
    void updateBuildOrderControl();
    void updateDependenciesControl();
    virtual void activateApply(QListViewItem*);

protected:
    QListView *prjList;
    QMakeScopeItem *myProjectItem;
    QPtrList <QMakeScopeItem> getAllProjects();
    TrollProjectWidget* prjWidget;
    void getAllSubProjects( QMakeScopeItem *item, QPtrList<QMakeScopeItem> *itemList );


};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
