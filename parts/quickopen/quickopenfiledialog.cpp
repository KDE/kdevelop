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

QuickOpenFileDialog::QuickOpenFileDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialog( part, parent, name, modal, fl ), m_hasFullPaths( false )
{
    nameLabel->setText( i18n("File &name:") );
    itemListLabel->setText( i18n("File &list:") );

    m_items = m_part->project()->allFiles();

    nameEdit->setFocus();

    itemList->setSelectionMode( QListBox::Extended );
    itemList->insertStringList( m_items );
    setFirstItemSelected();
}

QuickOpenFileDialog::QuickOpenFileDialog(QuickOpenPart* part, const KURL::List & urls, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialog( part, parent, name, modal, fl ), m_hasFullPaths( true )
{
    nameLabel->setText( i18n("File &name:") );
    itemListLabel->setText( i18n("File &list:") );

    m_items = urls.toStringList();
    QStringList_unique( m_items );

    if (m_part->project())
    {
        for (unsigned int i = 0; i < m_items.count(); ++i)
        {
            QString url = m_items[i];
            QString projectUrl = "file://" + m_part->project()->projectDirectory();
            if (url.startsWith(projectUrl))
                m_items[i] = url.mid(projectUrl.length() + 1);
        }
    }

    nameEdit->setFocus();

    itemList->setSelectionMode( QListBox::Extended );
    itemList->insertStringList( m_items );
    setFirstItemSelected();
}

QuickOpenFileDialog::~QuickOpenFileDialog()
{
}

void QuickOpenFileDialog::slotExecuted( QListBoxItem* item )
{
    if ( !item ) return;

    if ( m_hasFullPaths )
    {
        m_part->partController()->editDocument( KURL::fromPathOrURL( item->text() ) );
    }
    else
    {
        m_part->partController()->editDocument( KURL::fromPathOrURL( m_part->project()->projectDirectory() + "/" + item->text() ) );
    }
    accept();
}

void QuickOpenFileDialog::slotReturnPressed( )
{
    maybeUpdateSelection();

    for (int i = 0; i < itemList->count(); ++i)
    {
        if (itemList->isSelected(i))
        {
            if (m_hasFullPaths)
            {
                m_part->partController()->editDocument(KURL::fromPathOrURL(itemList->item(i)->text()));
            }
            else
            {
                m_part->partController()->editDocument(KURL::fromPathOrURL(m_part->project()->projectDirectory() + "/" + itemList->item(i)->text()));
            }
        }
    }
    accept();
}

#include "quickopenfiledialog.moc"

