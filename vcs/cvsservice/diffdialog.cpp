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

///////////////////////////////////////////////////////////////////////////////
// class DiffDialog
///////////////////////////////////////////////////////////////////////////////

DiffDialog::DiffDialog( QWidget *parent, const char *name, WFlags f )
    : DiffDialogBase( parent, name, true, f)
{
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

#include "diffdialog.moc"
