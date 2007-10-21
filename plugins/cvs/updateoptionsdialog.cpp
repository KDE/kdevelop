/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                              *
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

KDevelop::VcsRevision UpdateOptionsDialog::revision()
{
    KDevelop::VcsRevision rev;

    if (revisionRadio->isChecked()) {
        rev.setRevisionValue( revisionEdit->text(), KDevelop::VcsRevision::FileNumber );
    } else if (dateRadio->isChecked()) {
        rev.setRevisionValue( dateEdit->text(), KDevelop::VcsRevision::Date );
    }

    return rev;
}

#include "updateoptionsdialog.moc"
