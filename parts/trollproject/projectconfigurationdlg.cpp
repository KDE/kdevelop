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
#include <qradiobutton.h>

ProjectConfigurationDlg::ProjectConfigurationDlg(ProjectConfiguration *conf,QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name,modal,fl)
//=================================================
{
  m_projectConfiguration = conf;
  UpdateControls();
}


ProjectConfigurationDlg::~ProjectConfigurationDlg()
//==============================================
{
}


void ProjectConfigurationDlg::updateProjectConfiguration()
//=======================================================
{
  if (radioApplication->isChecked())
    m_projectConfiguration->m_template = QTMP_APPLICATION;
  else if (radioLibrary->isChecked())
    m_projectConfiguration->m_template = QTMP_LIBRARY;
  else if (radioSubdirs->isChecked())
    m_projectConfiguration->m_template = QTMP_SUBDIRS;
  QDialog::accept();
}


void ProjectConfigurationDlg::UpdateControls()
//============================================
{
  QRadioButton *activeTemplate=NULL;
  // Project template
  switch (m_projectConfiguration->m_template)
  {
    case QTMP_APPLICATION:
      activeTemplate = radioApplication;
      break;
    case QTMP_LIBRARY:
      activeTemplate = radioLibrary;
      break;
    case QTMP_SUBDIRS:
      activeTemplate = radioSubdirs;
      break;
  }
  if (activeTemplate)
    activeTemplate->setChecked(true);
}
