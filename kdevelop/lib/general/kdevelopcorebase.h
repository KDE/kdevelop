/***************************************************************************
                          kdevelopcorebase.h  -  description
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

#ifndef _KDEVELOPCOREBASE_H_
#define _KDEVELOPCOREBASE_H_


#include <qobject.h>
#include <qlist.h>

// Need to forward declare this baby to prevent recursive inclusion.
class KDevComponent;


class KDevelopCoreBase : public QObject
{
    Q_OBJECT

public:
    // Constructor/Destructor.
    KDevelopCoreBase (QObject * parent=0, const char * name=0);
    ~KDevelopCoreBase ();

    /** They have changed the subproject they are using, we should notify
        all the components, to give them a chance to react. */
    void changeProjectSpace ();

protected:
    /** A list of all components. */
    QList<KDevComponent> m_components;

    /** A list of loaded components. */
    QList<KDevComponent> m_runningComponents;
};

#endif
