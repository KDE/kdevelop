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
#include <qcombobox.h>
#include <qcheckbox.h>

#include "cvsform.h"

///////////////////////////////////////////////////////////////////////////////
// class CvsForm
///////////////////////////////////////////////////////////////////////////////

CvsForm::CvsForm( QWidget *parent, const char *name, Qt::WFlags f )
    : CvsFormBase( parent, name, f )
{
    setWFlags( getWFlags() | Qt::WDestructiveClose );
}

///////////////////////////////////////////////////////////////////////////////

CvsForm::~CvsForm()
{
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::module() const
{
    return module_edit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::vendor() const
{
    return vendor_edit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::message() const
{
    return message_edit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::release() const
{
    return release_edit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::location() const
{
    return serverPathEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsForm::cvsRsh() const
{
    return cvsRshComboBox->currentText();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsForm::mustInitRoot() const
{
    return init_check->isChecked();
}

#include "cvsform.moc"
