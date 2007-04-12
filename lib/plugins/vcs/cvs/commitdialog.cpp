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

#include "commitdialog.h"

CommitDialog::CommitDialog(QDialog *parent)
    : QDialog(parent), Ui::CommitDialogBase()
{
    Ui::CommitDialogBase::setupUi(this);
}

CommitDialog::~CommitDialog()
{
}

#include "commitdialog.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
