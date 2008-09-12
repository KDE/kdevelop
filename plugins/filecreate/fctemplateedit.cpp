/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <klineedit.h>
#include "fctemplateedit.h"
#include <qpushbutton.h>

FCTemplateEdit::FCTemplateEdit(QWidget *parent, const char *name):
    FCTemplateEditBase(parent, name)
{
    connect( templatename_edit, SIGNAL(textChanged ( const QString & )),
             this, SLOT( slotTemplateNameChanged( )));
    slotTemplateNameChanged();
}

FCTemplateEdit::~FCTemplateEdit()
{
}

void FCTemplateEdit::slotTemplateNameChanged( )
{
    ok_button->setEnabled( !templatename_edit->text().isEmpty() );
}

void FCTemplateEdit::accept()
{
    if ( !templatename_edit->text().isEmpty() )
        FCTemplateEditBase::accept();
}
#include "fctemplateedit.moc"
