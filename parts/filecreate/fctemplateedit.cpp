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
#include "fctemplateedit.h"


FCTemplateEdit::FCTemplateEdit(QWidget *parent, const char *name):
    FCTemplateEditBase(parent, name)
{
}

FCTemplateEdit::~FCTemplateEdit()
{
}

void FCTemplateEdit::accept()
{
    if ( !templatename_edit->text().isEmpty() )
        FCTemplateEditBase::accept();
}
#include "fctemplateedit.moc"
