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

#include "updateoptionsdialog.h"

#include <KLineEdit>
#include <QCheckBox>
#include <QRadioButton>

UpdateOptionsDialog::UpdateOptionsDialog( QWidget* parent)
    : QDialog(parent), Ui::UpdateOptionsDialogBase()
{
    Ui::UpdateOptionsDialogBase::setupUi(this);
}

UpdateOptionsDialog::~UpdateOptionsDialog()
{
}

QString UpdateOptionsDialog::options() const
{
    QString options;

    if (revertCheck->isChecked()) {
        options += " -C ";
    }

    if (revisionRadio->isChecked()) {
        options += " -r"+revisionEdit->text();
    } else if (dateRadio->isChecked()) {
        options += " -D"+dateEdit->text();
    }

    return options;
}

#include "updateoptionsdialog.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

