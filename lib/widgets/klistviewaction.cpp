/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif
#include "klistviewaction.h"
#include "kcomboview.h"
#include "resizablecombo.h"

#include <qtooltip.h>
#include <qwhatsthis.h>

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
    KWidgetAction(view, text, cut, receiver, slot, parent, name), m_view(view)
{
    m_view->setDuplicatesEnabled(false);
    m_view->setInsertionPolicy(KComboView::NoInsertion);

    loadComboWidth();
}

KListViewAction::KListViewAction( KComboView * view, const QString & text, const KShortcut & cut,
    const QObject * receiver, const char * slot, KActionCollection * parent, const char * name, const bool /*dummy*/ ):
    KWidgetAction(new ResizableCombo(view), text, cut, receiver, slot, parent, name), m_view(view)
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
    QToolTip::add(m_view, str);
}

void KListViewAction::setWhatsThis( const QString & str )
{
    QWhatsThis::add(m_view, str);
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

