/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
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

#include "quickopendialog.h"
#include "quickopen_part.h"

#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include <klistbox.h>
#include <klineedit.h>
#include <qregexp.h>

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    m_fileList = m_part->project()->allFiles();

    m_completion = new KCompletion();
    m_completion->insertItems( m_fileList );
    m_completion->setIgnoreCase( true );

    fileNameEdit->setFocus();

    projectFileList->insertStringList( m_fileList );
}

QuickOpenDialog::~QuickOpenDialog()
{
    delete( m_completion );
    m_completion = 0;
}

/*$SPECIALIZATION$*/
void QuickOpenDialog::slotExecuted( QListBoxItem* item )
{
    m_part->partController()->editDocument( item->text() );
    accept();
}

void QuickOpenDialog::reject()
{
    QDialog::reject();
}

void QuickOpenDialog::accept()
{
    QDialog::accept();
}

void QuickOpenDialog::slotReturnPressed( )
{
    if( m_fileList.contains(fileNameEdit->text()) ) {
        m_part->partController()->editDocument( fileNameEdit->text() );
        accept();
    }
}

void QuickOpenDialog::slotTextChanged( const QString & text )
{
    projectFileList->clear();
    projectFileList->insertStringList( m_completion->substringCompletion(text) );
}


#include "quickopendialog.moc"

