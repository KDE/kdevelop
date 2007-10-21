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

QString DiffOptionsDialog::revA() const
{
    if (requestedDiff() == diffArbitrary)
        return revaEdit->text();
    else  if (requestedDiff() == diffLocalOther)
        return revOtherEdit->text();
    else  if (requestedDiff() == diffLocalHEAD)
        return "HEAD";
    else
        return QString();
}

QString DiffOptionsDialog::revB() const
{
    if (requestedDiff())
        return this->revbEdit->text();
    else
        return QString();
}

#include "diffoptionsdialog.moc"
