/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "klistviewaction.h"
#include "qcomboview.h"

KListViewAction::~KListViewAction()
{
    delete m_view;
}

KListViewAction::KListViewAction(QComboView *view, const QString & text, const KShortcut & cut,
    const QObject * receiver, const char * slot, KActionCollection * parent, const char * name ):
    KWidgetAction(view, text, cut, receiver, slot, parent, name), m_view(view)
{
    m_view->setDuplicatesEnabled(false);
    m_view->setInsertionPolicy(QComboView::NoInsertion);
}

QComboView * KListViewAction::view( ) const
{
    return m_view;
}


