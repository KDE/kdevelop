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

QString DiffDialog::revA() const
{
    return this->revaEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString DiffDialog::revB() const
{
    return this->revbEdit->text();
}
