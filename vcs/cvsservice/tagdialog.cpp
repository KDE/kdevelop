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
#include <qcheckbox.h>

#include "tagdialog.h"

///////////////////////////////////////////////////////////////////////////////
// class TagDialog
///////////////////////////////////////////////////////////////////////////////

TagDialog::TagDialog( const QString &caption, QWidget *parent, const char *name )
    : TagDialogBase( parent, name ? name : "tagdialog", true )
{
    if (!caption.isEmpty())
    {
        setCaption( caption );
    }
}

///////////////////////////////////////////////////////////////////////////////

TagDialog::~TagDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

void TagDialog::accept()
{
    if (tagBranchEdit->text().isEmpty())
        return;

    TagDialogBase::accept();
}

///////////////////////////////////////////////////////////////////////////////

QString TagDialog::tagName() const
{
    return tagBranchEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString TagDialog::branchName() const
{
    return tagName();
}

///////////////////////////////////////////////////////////////////////////////

bool TagDialog::isBranch() const
{
    return tagAsBranchCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool TagDialog::force() const
{
    return forceCheck->isChecked();
}

#include "tagdialog.moc"
