/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classactions.h"

#include <qpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolbar.h>

#include "kdevplugin.h"

DelayedPopupAction::DelayedPopupAction(const QString& text, const QString& pix, const KShortcut& accel,
                                       QObject *receiver, const char *slot,
                                       KActionCollection* parent, const char* name )
    : KAction(text, pix, accel, receiver, slot, parent, name)
{
    m_popup = 0;
}


DelayedPopupAction::~DelayedPopupAction()
{
    if ( m_popup )
        delete m_popup;
}


int DelayedPopupAction::plug(QWidget *widget, int index)
{
    if (widget->inherits("KToolBar")) {
        KToolBar *bar = (KToolBar *)widget;
        connect( bar, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

        int id = KAction::getToolButtonID();
        bar->insertButton(icon(), id, SIGNAL( clicked() ), this,
                          SLOT(slotActivated()), isEnabled(), plainText(),
                          index);
        addContainer(bar, id);
        bar->setDelayedPopup(id, popupMenu(), true);

        return containerCount()-1;
    }

    return KAction::plug(widget, index);
}


void DelayedPopupAction::unplug(QWidget *widget)
{
    if (widget->inherits( "KToolBar")) {
        KToolBar *bar = (KToolBar *)widget;
        
        int idx = findContainer(bar);
        if (idx == -1)
            return;
        
        bar->removeItem(menuId(idx));
        removeContainer(idx);
        
        return;
    }
    
    KAction::unplug(widget);
}


QPopupMenu *DelayedPopupAction::popupMenu()
{
    if (!m_popup)
        m_popup = new QPopupMenu();
    
    return m_popup;
}

#include "classactions.moc"
