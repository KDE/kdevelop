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

#include "doclineedit.h"

#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qregexp.h>
#include <qlabel.h>

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
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

    connect(nameEdit, SIGNAL(upPressed()), this, SLOT(moveUpInList()));
    connect(nameEdit, SIGNAL(downPressed()), this, SLOT(moveDownInList()));
    connect(nameEdit, SIGNAL(pgupPressed()), this, SLOT(scrollUpInList()));
    connect(nameEdit, SIGNAL(pgdownPressed()), this, SLOT(scrollDownInList()));
    connect(nameEdit, SIGNAL(homePressed()), this, SLOT(goToBegin()));
    connect(nameEdit, SIGNAL(endPressed()), this, SLOT(goToEnd()));
}

QuickOpenDialog::~QuickOpenDialog()
{
    delete( m_completion );
    m_completion = 0;
}

/*$SPECIALIZATION$*/
void QuickOpenDialog::slotExecuted( QListBoxItem* item )
{
    m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + item->text() );
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
/*    if( m_fileList.contains(nameEdit->text()) ) {
        m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + nameEdit->text() );
        accept();
    }*/
    if( itemList->currentItem() != -1 ) {
        m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + itemList->currentText() );
        accept();
    }
}

void QuickOpenDialog::slotTextChanged( const QString & text )
{
    itemList->clear();
    itemList->insertStringList( m_completion->substringCompletion(text) );
    itemList->setCurrentItem(0);
}

void QuickOpenDialog::moveUpInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::moveDownInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollUpInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollDownInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::goToBegin( )
{
    itemList->setCurrentItem(0);
}

void QuickOpenDialog::goToEnd( )
{
    itemList->setCurrentItem(itemList->count()-1);
}


#include "quickopendialog.moc"

