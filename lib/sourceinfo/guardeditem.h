/***************************************************************************
                          guardeditem.h  -  description
                             -------------------
    begin                : Mon Jan 06 2003
    copyright            : (C) 2003 by Harald Fernengel
    email                : harry@bnro.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _GUARDEDITEM_H_
#define _GUARDEDITEM_H_

#include <qptrlist.h>

/**
 * Pure abstract interface a class has to implement
 * that wants to get the notification.
 */
class NotifyClient
{
public:
    virtual void notify() = 0;
};

/**
 * Lets classes register themselves to get notification
 * When the item gets deleted.
 *
 * For speed reasons, you can only register one class that
 * should get the notification.
 *
 * @author Harald Fernengel
 */
class GuardedItem
{
public:
    GuardedItem() {}
    GuardedItem( const GuardedItem& ) {}
    GuardedItem& operator= ( const GuardedItem& other ) { return *this; }
    virtual ~GuardedItem();

    virtual void registerNotifyClient( NotifyClient* nc );
    virtual void unregisterNotifyClient( NotifyClient* nc );

private:
    QPtrList<NotifyClient> n;
};

inline GuardedItem::~GuardedItem()
{
    for ( NotifyClient* cl = n.first(); cl; cl = n.next() ) {
        cl->notify();
    }
}

inline void GuardedItem::registerNotifyClient( NotifyClient* nc )
{
    n.append( nc );
}

inline void GuardedItem::unregisterNotifyClient( NotifyClient* nc )
{
    Q_ASSERT( n.removeRef( nc ) );
}

#endif 
