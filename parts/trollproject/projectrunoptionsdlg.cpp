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
 
#include "projectrunoptionsdlg.h"
#include "trollprojectpart.h"
#include <qfiledialog.h>
#include <klineedit.h>


ProjectRunOptionsDlg::ProjectRunOptionsDlg( TrollProjectPart *part, QWidget* parent, const char* name, WFlags fl )
: ProjectRunOptionsDlgBase (parent,name,fl)
//==================================================================================================
{
  m_part = part;
}

ProjectRunOptionsDlg::~ProjectRunOptionsDlg()
//=====================================
{
}

void ProjectRunOptionsDlg::browseMainProgram()
//=========================================
{
  m_mainProgramEdit->setText(QFileDialog::getOpenFileName("(*)",m_part->projectDirectory()));
}

