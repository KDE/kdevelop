/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtl.h>
#include <ktoolbar.h>
#include "classstore.h"
#include "classactions.h"


ClassListAction::ClassListAction(const QString &text, int accel,
                                 const QObject *receiver, const char *slot,
                                 QObject *parent, const char *name)
    : KSelectAction(text, accel, receiver, slot, parent, name)
{
    m_store = 0;
}
 

void ClassListAction::setClassStore(ClassStore *store)
{
    m_store = store;
}


void ClassListAction::setCurrentItem(const QString &item)
{
    int idx = items().findIndex(currentText());
    if (idx != -1)
        KSelectAction::setCurrentItem(idx);
}


void ClassListAction::refresh()
{
    if (!m_store)
        return;
    
    QList<ParsedClass> *classList = m_store->getSortedClassList();

    QStringList list;
    QListIterator<ParsedClass> it(*classList);
    for ( ; it.current(); ++it)
        list << it.current()->name;

    qHeapSort(list);

    int idx = list.findIndex(currentText());
    setItems(list);
    if (idx != -1)
        KSelectAction::setCurrentItem(idx);

    delete classList;
}


MethodListAction::MethodListAction(const QString &text, int accel,
                                   const QObject *receiver, const char *slot,
                                   QObject *parent, const char *name)
    : KSelectAction(text, accel, receiver, slot, parent, name)
{
    m_store = 0;
}


void MethodListAction::setClassStore(ClassStore *store)
{
    m_store = store;
}


void MethodListAction::refresh(const QString &className)
{
    if (!m_store)
        return;

    ParsedClass *pc;
    QStringList list;

    if (!className.isEmpty() && (pc = m_store->getClassByName(className)) != 0) {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            QString str = pc->methodIterator.current()->asString();
            list << str;
        }
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            QString str = pc->slotIterator.current()->asString();
            list << str;
        }
        qHeapSort(list);
    }

    int idx = list.findIndex(currentText());
    setItems(list);
    if (idx != -1)
        setCurrentItem(idx);
}


DelayedPopupAction::DelayedPopupAction(const QString& text, const QString& pix, int accel,
                                       QObject *receiver, const char *slot,
                                       QObject* parent, const char* name )
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
