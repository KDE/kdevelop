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


#include <kconfig.h>
#include <qobject.h>
#include <qlist.h>

class KDevelopCoreBase : public QObject
{
    Q_OBJECT

public:
    // Constructor/Destructor.
    KDevelopCoreBase (QObject * parent=0, const char * name=0);
    ~KDevelopCoreBase ();

    // Make sure they define some form of session management.
    virtual void writeProperties (KConfig *pConfig) = 0;
    virtual void readProperties (KConfig *pConfig) = 0;
};

#endif
