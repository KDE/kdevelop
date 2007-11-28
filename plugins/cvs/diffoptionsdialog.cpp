/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diffoptionsdialog.h"


DiffOptionsDialog::DiffOptionsDialog(QWidget *parent, const KUrl& url)
    : QDialog(parent), Ui::DiffOptionsDialogBase(), m_url(url)
{
    Ui::DiffOptionsDialogBase::setupUi(this);

    labelUrl->setText( url.toLocalFile() );
}

DiffOptionsDialog::~DiffOptionsDialog()
{
}

DiffOptionsDialog::DiffType DiffOptionsDialog::requestedDiff() const
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

KDevelop::VcsRevision DiffOptionsDialog::revA() const
{
    KDevelop::VcsRevision rev;

    if (requestedDiff() == diffArbitrary)
        rev.setRevisionValue( revaEdit->text(), KDevelop::VcsRevision::FileNumber );
    else  if (requestedDiff() == diffLocalOther)
        rev.setRevisionValue( revOtherEdit->text(), KDevelop::VcsRevision::FileNumber );
    else  if (requestedDiff() == diffLocalHEAD)
        rev.setRevisionValue( "HEAD", KDevelop::VcsRevision::FileNumber );

    return rev;
}

KDevelop::VcsRevision DiffOptionsDialog::revB() const
{
    KDevelop::VcsRevision rev;

    if (requestedDiff() == diffArbitrary)
        rev.setRevisionValue( this->revbEdit->text(), KDevelop::VcsRevision::FileNumber );

    return rev;
}

#include "diffoptionsdialog.moc"
