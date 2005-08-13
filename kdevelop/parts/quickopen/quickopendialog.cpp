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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include <qapplication.h>
 
#include <klistbox.h>
#include <klineedit.h>
#include <kcompletion.h>

#include "quickopendialog.h"

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    nameEdit->installEventFilter(this);
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

bool QuickOpenDialog::eventFilter( QObject * watched, QEvent * e )
{
    if (!watched || !e)
        return true;

    if ((watched == nameEdit) && (e->type() == QEvent::KeyPress))
    {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Key_Up)
        {
            int i = itemList->currentItem();
            if (--i >= 0)
            {
                itemList->setCurrentItem(i);
                nameEdit->blockSignals(true);
                nameEdit->setText(itemList->currentText());
                nameEdit->blockSignals(false);
            }
            return true;
        } else if (ke->key() == Key_Down)
        {
            int i = itemList->currentItem();
            if ( ++i < int(itemList->count()) ) 
            {
                itemList->setCurrentItem(i);
                nameEdit->blockSignals(true);
                nameEdit->setText(itemList->currentText());
                nameEdit->blockSignals(false);
            }
            return true;
        } else if ((ke->key() == Key_Next) || (ke->key() == Key_Prior))
        {
            QApplication::sendEvent(itemList, e);
            nameEdit->blockSignals(true);
            nameEdit->setText(itemList->currentText());
            nameEdit->blockSignals(false);
        }
    }

    return QWidget::eventFilter(watched, e);    
}

#include "quickopendialog.moc"

