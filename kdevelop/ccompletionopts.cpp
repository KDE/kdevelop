/***************************************************************************
                          ccompletionopts.cpp  -  description
                             -------------------
    begin                : Mon Feb 27 2002
    copyright            : (C) 2002 by Roberto Raggi
    email                : raggi@cli.di.unipi.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ccompletionopts.h"
#include "ccompletionopts.moc"
#include "ckdevelop.h"
#include <qcheckbox.h>

CCompletionOpts::CCompletionOpts(CKDevelop* pDev,
                                 QWidget *parent, const char *name )
    : CCompletionOptsDlg(parent,name), m_pDevelop( pDev )
{
    checkBoxCompleteText->setChecked( m_pDevelop->getAutomaticCompletion() );
    checkBoxArgHint->setChecked( m_pDevelop->getAutomaticArgsHint() );
}

CCompletionOpts::~CCompletionOpts()
{
}

void CCompletionOpts::slotSettingsChanged()
{
    m_pDevelop->setAutomaticCompletion( checkBoxCompleteText->isChecked() );
    m_pDevelop->setAutomaticArgsHint( checkBoxArgHint->isChecked() );
}
