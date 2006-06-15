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

#include <qlabel.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include "releaseinputdialog.h"

///////////////////////////////////////////////////////////////////////////////
// class ReleaseInputDialog
///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::ReleaseInputDialog( QWidget* parent)
    : ReleaseInputDialogBase( parent, "releaseinputdialog", true, 0 )
{
}

///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::~ReleaseInputDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

bool ReleaseInputDialog::isRevert() const
{
    return revertCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

QString ReleaseInputDialog::release() const
{
    if (type() == byRevision)
        return " -r " + revisionEdit->text();
    else if (type() == byDate)
        return " -D " + dateEdit->text();
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::ReleaseType ReleaseInputDialog::type() const
{
    if (revisionRadio->isChecked())
        return byRevision;
    else if (dateRadio->isChecked())
        return byDate;
    else
        return byHead;
}

#include "releaseinputdialog.moc"

