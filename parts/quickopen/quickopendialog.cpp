/*
 *  Copyright (C) KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2004
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

#include <klistbox.h>
#include <kcompletion.h>

#include "doclineedit.h"

#include "quickopendialog.h"

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    connect(nameEdit, SIGNAL(upPressed()), this, SLOT(moveUpInList()));
    connect(nameEdit, SIGNAL(downPressed()), this, SLOT(moveDownInList()));
    connect(nameEdit, SIGNAL(pgupPressed()), this, SLOT(scrollUpInList()));
    connect(nameEdit, SIGNAL(pgdownPressed()), this, SLOT(scrollDownInList()));
    connect(nameEdit, SIGNAL(homePressed()), this, SLOT(goToBegin()));
    connect(nameEdit, SIGNAL(endPressed()), this, SLOT(goToEnd()));
}

QuickOpenDialog::~QuickOpenDialog()
{
}

void QuickOpenDialog::slotTextChanged(const QString & text)
{
    itemList->clear();
    itemList->insertStringList( m_completion->substringCompletion(text) );
    itemList->setCurrentItem(0);
}

void QuickOpenDialog::moveUpInList()
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::moveDownInList()
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollUpInList()
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollDownInList()
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenDialog::goToBegin()
{
	itemList->setCurrentItem(0);
}

void QuickOpenDialog::goToEnd()
{
	itemList->setCurrentItem(itemList->count()-1);
}

#include "quickopendialog.moc"

