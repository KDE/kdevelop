/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *   Copyright (C) 2002-2003 by Alexander Dymo                             *
 *   cloudtemple@mksat.net                                                 *
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTCONFIGURATIONDLG_H
#define PROJECTCONFIGURATIONDLG_H

enum QMakeBuildMode    {QBM_DEBUG,QBM_RELEASE};
enum QMakeWarnings     {QWARN_ON,QWARN_OFF};
enum QMakeDepends      {QD_QT=1,QD_OPENGL=2,QD_THREAD=4,QD_X11=8,QD_STATIC=16,
                        QD_SHARED=32,QD_PLUGIN=64,QD_EXCEPTIONS=128,QD_STL=256,
                        QD_RTTI=512};
enum QMakeTemplate     {QTMP_APPLICATION,QTMP_LIBRARY,QTMP_SUBDIRS};

#include "projectconfigurationdlgbase.h"
#include "trollprojectwidget.h"

struct ProjectConfiguration;
class SubprojectItem;
class ProjectItem;


class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
public:
  ProjectConfigurationDlg(SubprojectItem * _item,QListView *_prjList,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~ProjectConfigurationDlg();
  void UpdateControls();

public slots:
//  virtual void           radioLibrarytoggled(bool);
  virtual void           updateProjectConfiguration();
  virtual void           browseTargetPath();
  virtual void           clickSubdirsTemplate();
  virtual void           templateLibraryClicked(int);

  
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

virtual void slotStaticLibClicked(int);
virtual void slotInstallTargetClicked();

void updateIncludeControl();
void updateLibaddControl();
void updateLibDirAddControl();
void updateBuildOrderControl();


  
protected:
  QListView *prjList;
  SubprojectItem *myProjectItem;
  QPtrList <ProjectItem> getAllProjects();
  void getAllSubProjects(ProjectItem *item,QPtrList <ProjectItem> *itemList);
  

};

#endif

