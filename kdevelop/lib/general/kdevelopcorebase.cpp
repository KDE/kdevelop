/***************************************************************************
                          kdevelopcorebase.cpp  -  description
                             -------------------
    begin                : Sun Jan 7 2001
    copyright            : (C) 2001 by Ivan Hawkes
    email                : blackhawk@ivanhawkes.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevelopcorebase.h"
#include <kdebug.h>
#include <kdevcomponent.h>


KDevelopCoreBase::KDevelopCoreBase (QObject * parent=0, const char * name=0)
:QObject (parent, name)
{
}


KDevelopCoreBase::~KDevelopCoreBase ()
{
}

/*  They have changed the subproject they are using, we should notify
    all the components, to give them a chance to react. */

void KDevelopCoreBase::changeProjectSpace ()
{
    kdDebug(9000) << "KDevelopCore::changeProjectSpace" << endl;

    // Notification
    QListIterator<KDevComponent> it1(m_components);
    for (; it1.current(); ++it1)
        (*it1)->projectChanged();
}

