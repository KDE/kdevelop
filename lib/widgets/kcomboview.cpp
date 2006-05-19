/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <klineedit.h>
#include <qlistview.h>
#include <kcompletionbox.h>
#include <qmap.h>

#include "kcomboview.h"


KComboView::KComboView( bool rw, int defaultWidth, QWidget* parent, const char* name , CustomCompleter* comp)
    :QComboView(rw, parent, name), m_defaultWidth(defaultWidth), m_comp( comp )
{
    if (rw)
    {
        KLineEdit *ed = new KLineEdit(this, "combo edit");
        ed->setCompletionMode(KGlobalSettings::CompletionPopup);
        ed->setCompletionObject(m_comp);
        ed->completionBox()->setHScrollBarMode(QListBox::Auto);
        setLineEdit(ed);
    }
    setMinimumWidth(defaultWidth);
}

KComboView::~KComboView() {
    delete m_comp;
}

void KComboView::addItem(QListViewItem *it)
{
    m_comp->addItem(it->text(0));
}

void KComboView::removeItem(QListViewItem *it)
{
    if (it == currentItem())
    {
        setCurrentItem(0);
        setCurrentText(m_defaultText);
    }
    m_comp->removeItem(it->text(0));
    delete it;
}

void KComboView::renameItem(QListViewItem *it, const QString &newName)
{
    m_comp->removeItem(it->text(0));
    it->setText(0, newName);
    m_comp->addItem(newName);
}

void KComboView::clear( )
{
    m_comp->clear();
    QComboView::clear();

    setCurrentText(m_defaultText);
}

int KComboView::defaultWidth( )
{
    return m_defaultWidth;
}

void KComboView::setDefaultText( const QString & text )
{
    m_defaultText = text;
}

#include "kcomboview.moc"
