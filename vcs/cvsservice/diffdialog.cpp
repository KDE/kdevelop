/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlineedit.h>
#include <qradiobutton.h>

#include "diffdialog.h"
#include <klocale.h>
#include <q3buttongroup.h>

///////////////////////////////////////////////////////////////////////////////
// class DiffDialog
///////////////////////////////////////////////////////////////////////////////

DiffDialog::DiffDialog( const CVSEntry &entry, QWidget *parent, const char *name, Qt::WFlags f )
    : DiffDialogBase( parent, name, true, f)
{
    m_entry = entry;
    QString currentRevision = entry.revision();
    revaEdit->setText(currentRevision);
    revbEdit->setText(currentRevision);
    revOtherEdit->setText(currentRevision);
    languageChange();
}

///////////////////////////////////////////////////////////////////////////////

DiffDialog::~DiffDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

DiffDialog::DiffType DiffDialog::requestedDiff() const
{
    if (diffArbitraryRevRadio->isChecked())
        return diffArbitrary;
    else if (diffLocalOtherRadio->isChecked())
        return diffLocalOther;
    else if (diffLocalBaseRadio->isChecked())
        return diffLocalBASE;
    else
        return diffLocalHEAD;
}

///////////////////////////////////////////////////////////////////////////////

QString DiffDialog::revA() const
{
    if (requestedDiff() == diffArbitrary)
        return revaEdit->text();
    else  if (requestedDiff() == diffLocalOther)
        return revOtherEdit->text();
    else  if (requestedDiff() == diffLocalHEAD)
        return "HEAD";
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString DiffDialog::revB() const
{
    if (requestedDiff())
        return this->revbEdit->text();
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

void DiffDialog::languageChange() {
    DiffDialogBase::languageChange();
    //buttonGroup1->setTitle( tr2i18n( "Build Difference Between" ) );
    
    //FIXME:  We need a function in CVSEntry to return the latest revision there is in cvs
//    if(!m_entry.revision().isNull())
//      diffLocalHeadRadio->setText( tr2i18n( "Local copy and &HEAD (%1)" ).arg( m_entry.revision()) );
}

#include "diffdialog.moc"
