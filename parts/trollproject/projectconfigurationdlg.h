/***************************************************************************
 *   Copyright (C) 2001 by Jakob Simon-Gaarde                              *
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

#include "projectconfigurationdlgbase.h"
#include "trollprojectwidget.h"

enum QMakeBuildMode    {QBM_DEBUG,QBM_RELEASE};
enum QMakeWarnings     {QWARN_ON,QWARN_OFF};
enum QMakeDepends      {QD_QT,QD_OPENGL,QD_THREAD,QD_X11};

class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
public:
                         ProjectConfigurationDlg(TrollProjectWidget *caller,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );

public slots:
  virtual QMakeBuildMode getBuildMode();

protected:
  TrollProjectWidget *m_trollProjectWidget;

};

#endif
