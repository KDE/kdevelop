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
enum QMakeDepends      {QD_QT=		1<<0,
			QD_OPENGL=	1<<1,
			QD_THREAD=	1<<2,
			QD_X11=		1<<3,
			QD_STATIC=	1<<4,
                        QD_SHARED=	1<<5,
			QD_PLUGIN=	1<<6,
			QD_EXCEPTIONS=	1<<8,
			QD_STL=		1<<9,
			QD_RTTI=	1<<10,
			QD_ORDERED=	1<<11,
			QD_LIBTOOL=	1<<12,
			QD_PKGCONF=	1<<13,
			QD_DLL=		1<<14,
			QD_CONSOLE=	1<<15,
			QD_PCH=		1<<16
			};
enum QMakeTemplate     {QTMP_APPLICATION,QTMP_LIBRARY,QTMP_SUBDIRS};
#include "projectconfigurationdlgbase.h"
#include "trollprojectwidget.h"

struct ProjectConfiguration;
class SubqmakeprojectItem;
class qProjectItem;


class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
public:
  ProjectConfigurationDlg(SubqmakeprojectItem * _item,QListView *_prjList,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
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


void updateIncludeControl();
void updateLibaddControl();
void updateLibDirAddControl();
void updateBuildOrderControl();
void updateDependenciesControl();



protected:
  QListView *prjList;
  SubqmakeprojectItem *myProjectItem;
  QPtrList <qProjectItem> getAllProjects();
  void getAllSubProjects(qProjectItem *item,QPtrList <qProjectItem> *itemList);


};

#endif

