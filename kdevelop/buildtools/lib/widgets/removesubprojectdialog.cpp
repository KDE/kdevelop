/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <qcheckbox.h>
#include <qlabel.h>
#include "removesubprojectdialog.h"

RemoveSubprojectDialog::RemoveSubprojectDialog(QString caption, QString question, QWidget* parent, const char* name, bool modal, WFlags fl)
    : RemoveSubprojectDlgBase(parent,name, modal,fl)
{
    setCaption(caption);
    removeLabel->setText(question);
}

RemoveSubprojectDialog::~RemoveSubprojectDialog()
{
}

/*$SPECIALIZATION$*/
void RemoveSubprojectDialog::reject()
{
    QDialog::reject();
}

void RemoveSubprojectDialog::accept()
{
    QDialog::accept();
}

bool RemoveSubprojectDialog::removeFromDisk( )
{
    return removeCheckBox->isChecked();
}


#include "removesubprojectdialog.moc"

