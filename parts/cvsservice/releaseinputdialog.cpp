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
#include <qlineedit.h>
#include <qradiobutton.h>

#include "releaseinputdialog.h"

///////////////////////////////////////////////////////////////////////////////
// class ReleaseInputDialog
///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::ReleaseInputDialog( const QString &releaseMsg, QWidget* parent,
    bool enforceNotNullInput )
    : ReleaseInputDialogBase( parent, "releaseinputdialog", true, 0 ),
    m_enforceNullInput( enforceNotNullInput )
{
    this->setCaption( releaseMsg );
}

///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::~ReleaseInputDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

QString ReleaseInputDialog::release() const
{
    // Can be a date or a tag/branch name
    QString releaseInfo = this->releaseLineEdit->text();
    // Avoid adding "-opt" to a null input
    if (releaseInfo.isEmpty())
        return QString::null;

    if (isDate())
        return  " -D " + releaseInfo; // Look at the leading white space!!
    else
        return  " -r " + releaseInfo;

}

///////////////////////////////////////////////////////////////////////////////

bool ReleaseInputDialog::isTag() const
{
    return this->tagRadio->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool ReleaseInputDialog::isDate() const
{
    return this->dateRadio->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

void ReleaseInputDialog::accept()
{
    if (m_enforceNullInput && this->releaseLineEdit->text().isEmpty())
        return;

    QDialog::accept();
}



#include "releaseinputdialog.moc"

