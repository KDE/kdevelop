/***************************************************************************
                          KDevClassStore.h  -  description
                             -------------------
    begin                : Wed Feb 14 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVCLASSSTORE_H_
#define _KDEVCLASSSTORE_H_

#include <qstringlist.h>
#include "kdevcomponent.h"


class KDevClassStore : public KDevComponent{
    Q_OBJECT
    
public:

    KDevClassStore( QObject *parent=0, const char *name=0 );
    ~KDevClassStore();
    virtual void wipeout();
};

#endif
