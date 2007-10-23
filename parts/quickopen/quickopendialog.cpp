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

#include <qapplication.h>
#include <qregexp.h>

#include <klistbox.h>
#include <klineedit.h>
#include <kdebug.h>

#include "quickopendialog.h"
#include "quickopen_part.h"

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    nameEdit->installEventFilter(this);
    connect( &m_typeTimeout, SIGNAL(timeout()), this, SLOT(slotTextChangedDelayed()) );
}

QuickOpenDialog::~QuickOpenDialog()
{
}

void QuickOpenDialog::slotTextChanged(const QString &)
{
    m_typeTimeout.start( 100, true );
}

void QuickOpenDialog::maybeUpdateSelection() {
    if( m_typeTimeout.isActive() )
    {
        m_typeTimeout.stop();
        slotTextChangedDelayed();
    }
}

void QuickOpenDialog::setFirstItemSelected()
{
    // Make sure the list has a current item or our event will not be handled properly.
    itemList->setCurrentItem(0);
    // We are doing this indirectly because the event handler does things for multiple
    // selections we cannot do through the public interface.
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Home, 0, 0);
    QApplication::sendEvent(itemList, &e);
}

void QuickOpenDialog::slotTextChangedDelayed()
{
    itemList->clear();
    itemList->insertStringList( wildCardCompletion( nameEdit->text() ) );
    setFirstItemSelected();
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
            QApplication::sendEvent(itemList, e);
            nameEdit->blockSignals(true);
            itemSelectionChanged();
            nameEdit->blockSignals(false);
            return true;
        } else if (ke->key() == Key_Down)
        {
            QApplication::sendEvent(itemList, e);
            nameEdit->blockSignals(true);
            itemSelectionChanged();
            nameEdit->blockSignals(false);
            return true;
        } else if ((ke->key() == Key_Next) || (ke->key() == Key_Prior))
        {
            QApplication::sendEvent(itemList, e);
            nameEdit->blockSignals(true);
            itemSelectionChanged();
            nameEdit->blockSignals(false);
        }
    }

    return QWidget::eventFilter(watched, e);
}

void QuickOpenDialog::selectClassViewItem(ItemDom item)
{
    m_part->selectItem( item );
}

QStringList QuickOpenDialog::wildCardCompletion(const QString & text)
{
    if ( text.isEmpty() ) return m_items;

    QRegExp re( text, false, true );
    QStringList matches;
    QStringList::const_iterator it = m_items.begin();
    while( it != m_items.end() )
    {
        if ( (*it).find( re ) != -1 )
        {
            matches << *it;
        }
        ++it;
    }

    return matches;
}

void QuickOpenDialog::QStringList_unique( QStringList & list )
{
    if ( list.size() < 2 ) return;

    list.sort();

    QStringList::iterator it = list.begin();
    QStringList::iterator it2 = it;
    while ( it2 != list.end() )
    {
        ++it2;
        while ( it2 != list.end() && *it2 == *it )
        {
            it2 = list.remove( it2 );
        }
        it = it2;
    }
}

void QuickOpenDialog::itemSelectionChanged() {
  nameEdit->setText(itemList->currentText());
}

#include "quickopendialog.moc"

// kate: space-indent on; indent-width 4; tab-width 4; show-tabs on;
