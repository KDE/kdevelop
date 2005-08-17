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
#include "klistviewaction.h"
#include "kcomboview.h"
#include "resizablecombo.h"

#include <qtooltip.h>
#include <q3whatsthis.h>

#include <kconfig.h>
#include <kglobal.h>

KListViewAction::~KListViewAction()
{
    KConfig *config = KGlobal::config();
    if (config && m_view->name())
    {
        config->setGroup("KListViewAction");
        config->writeEntry(m_view->name(), m_view->width());
    }
    delete m_view;
}

KListViewAction::KListViewAction(KComboView *view, const QString & text, const KShortcut & cut,
    const QObject * receiver, const char * slot, KActionCollection * parent, const char * name ):
    KWidgetAction(view, text, cut, receiver, slot, parent), m_view(view)
{
    m_view->setDuplicatesEnabled(false);
    m_view->setInsertionPolicy(KComboView::NoInsertion);

    loadComboWidth();
}

KListViewAction::KListViewAction( KComboView * view, const QString & text, const KShortcut & cut,
    const QObject * receiver, const char * slot, KActionCollection * parent, const char * name, const bool /*dummy*/ ):
    KWidgetAction(new ResizableCombo(view), text, cut, receiver, slot, parent), m_view(view)
{
    m_view->setDuplicatesEnabled(false);
    m_view->setInsertionPolicy(KComboView::NoInsertion);

    loadComboWidth();
}

KComboView * KListViewAction::view( ) const
{
    return m_view;
}

void KListViewAction::setToolTip( const QString & str )
{
    QToolTip::remove(m_view);
    QToolTip::add(m_view, str);
}

void KListViewAction::setWhatsThis( const QString & str )
{
    Q3WhatsThis::remove(m_view);
    Q3WhatsThis::add(m_view, str);
}

void KListViewAction::loadComboWidth( )
{
    KConfig *config = KGlobal::config();
    if (config && m_view->name())
    {
        config->setGroup("KListViewAction");
        m_view->setMinimumWidth(config->readNumEntry(m_view->name(), m_view->defaultWidth()));
    }
}

