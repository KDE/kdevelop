/***************************************************************************
                          cprjcompletionopts.cpp  -  description
                             -------------------
    begin                : Mon Feb 25 2002
    copyright            : (C) 2002 by kdevelop-team
    email                : kdevelop-team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cprjcompletionopts.h"
#include "cproject.h"
#include <qcheckbox.h>

CPrjCompletionOpts::CPrjCompletionOpts(CProject* prj, QWidget *parent, const char *name )
: CPrjCompletionOptsDlg(parent,name), m_pProject( prj )
{
  checkBoxCompleteText->setChecked( m_pProject->getAutomaticCompletion() );
  checkBoxArgHint->setChecked( m_pProject->getAutomaticArgsHint() );
}

CPrjCompletionOpts::~CPrjCompletionOpts()
{
}

void CPrjCompletionOpts::slotSettingsChanged()
{
  m_pProject->setAutomaticCompletion( checkBoxCompleteText->isChecked() );
  m_pProject->setAutomaticArgsHint( checkBoxArgHint->isChecked() );
}
