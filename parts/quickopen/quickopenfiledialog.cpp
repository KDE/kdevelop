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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include <kdevproject.h>
#include <kdevpartcontroller.h>


#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcompletion.h>
#include <klineedit.h>


#include <qregexp.h>
#include <qlabel.h>

#include "quickopenfiledialog.h"
#include "quickopen_part.h"

QuickOpenFileDialog::QuickOpenFileDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QuickOpenDialog( part, parent, name, modal, fl )
{
    nameLabel->setText( i18n("File &name:") );
    itemListLabel->setText( i18n("File &list:") );

    m_fileList = m_part->project()->allFiles();

    m_completion = new KCompletion();
    m_completion->insertItems( m_fileList );
    m_completion->setIgnoreCase( true );

    nameEdit->setFocus();

    itemList->insertStringList( m_fileList );
    itemList->setCurrentItem(0);
}

QuickOpenFileDialog::~QuickOpenFileDialog()
{
    delete( m_completion );
    m_completion = 0;
}

void QuickOpenFileDialog::slotExecuted( Q3ListBoxItem* item )
{
    m_part->partController()->editDocument( KURL::fromPathOrURL( m_part->project()->projectDirectory() + "/" + item->text() ) );
    accept();
}

void QuickOpenFileDialog::slotReturnPressed( )
{
/*    if( m_fileList.contains(nameEdit->text()) ) {
        m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + nameEdit->text() );
        accept();
    }*/
    if( itemList->currentItem() != -1 ) {
        m_part->partController()->editDocument( KURL::fromPathOrURL( m_part->project()->projectDirectory() + "/" + itemList->currentText() ) );
        accept();
    }
}

#include "quickopenfiledialog.moc"

