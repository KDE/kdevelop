/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diffoptionsdlg.h"


DiffOptionsDlg::DiffOptionsDlg(QWidget *parent, const KUrl& url)
    : QDialog(parent), Ui::DiffOptionsDlgBase(), m_url(url)
{
    Ui::DiffOptionsDlgBase::setupUi(this);

    labelUrl->setText( url.path() );
}

DiffOptionsDlg::~DiffOptionsDlg()
{
}

DiffOptionsDlg::DiffType DiffOptionsDlg::requestedDiff() const
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

QString DiffOptionsDlg::revA() const
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

QString DiffOptionsDlg::revB() const
{
    if (requestedDiff())
        return this->revbEdit->text();
    else
        return QString::null;
}

#include "diffoptionsdlg.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
