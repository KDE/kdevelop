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

#include "projectconfigurationdlg.h"

ProjectConfigurationDlg::ProjectConfigurationDlg(TrollProjectWidget *caller,QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name,modal,fl)
//=================================================
{
  m_trollProjectWidget = caller;
}


QMakeBuildMode ProjectConfigurationDlg::getBuildMode()
//====================================================
{

}