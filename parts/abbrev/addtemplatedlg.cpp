/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include "addtemplatedlg.h"

#include <qcombobox.h>
#include <qlineedit.h>


AddTemplateDialog::AddTemplateDialog( QStringList suffixesList, QWidget *parent, const char *name )
    : AddTemplateDialogBase( parent, name )
{
    setFocusProxy( editTemplate );
    comboSuffixes->insertStringList(suffixesList);
}


AddTemplateDialog::~AddTemplateDialog()
{
}

QString AddTemplateDialog::templ() const
{
    return editTemplate->text();
}

QString AddTemplateDialog::description() const
{
    return editDescription->text();
}

QString AddTemplateDialog::suffixes() const
{
    return comboSuffixes->currentText();
}

#include "addtemplatedlg.moc"
