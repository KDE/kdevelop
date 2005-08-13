/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qlineedit.h>
#include <qpushbutton.h>
#include "fctypeedit.h"


FCTypeEdit::FCTypeEdit(QWidget *parent, char *name)
 : FCTypeEditBase(parent, name)
{
    connect( typeext_edit, SIGNAL( textChanged ( const QString & )),this, SLOT( slotTypeEditTextChanged()));
    connect( typename_edit, SIGNAL( textChanged ( const QString & )),this, SLOT( slotTypeEditTextChanged( )));
    slotTypeEditTextChanged( );
}

FCTypeEdit::~FCTypeEdit()
{
}

void FCTypeEdit::slotTypeEditTextChanged( )
{
    ok_button->setEnabled( !typeext_edit->text().isEmpty() &&
                           !typename_edit->text().isEmpty());
}

void FCTypeEdit::accept()
{
    if ( !typeext_edit->text().isEmpty() &&
        !typename_edit->text().isEmpty() )
        FCTypeEditBase::accept();
}
#include "fctypeedit.moc"
