/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
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
enum QMakeDepends      {QD_QT=1,QD_OPENGL=2,QD_THREAD=4,QD_X11=8,QD_STATIC=16};
enum QMakeTemplate     {QTMP_APPLICATION,QTMP_LIBRARY,QTMP_SUBDIRS};

#include "projectconfigurationdlgbase.h"
#include "trollprojectwidget.h"

struct ProjectConfiguration;

class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
public:
                         ProjectConfigurationDlg(ProjectConfiguration *conf,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
                         ~ProjectConfigurationDlg();
  void                   UpdateControls();

public slots:
  virtual void           radioLibrarytoggled(bool);
  virtual void           updateProjectConfiguration();
  virtual void           browseTargetPath();
  virtual void           clickSubdirsTemplate();

protected:
  ProjectConfiguration   *m_projectConfiguration;

};

#endif

