/***************************************************************************
          kdevviewhandler.h  -  abstract base class for view handlers
                             -------------------
    begin                : Wed Jul 26 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVVIEWHANDLER_H_
#define _KDEVVIEWHANDLER_H_

#include "kdevcomponent.h"


class KDevViewHandler : public KDevComponent
{
    Q_OBJECT

public:
    KDevViewHandler( QObject *parent=0, const char *name=0 ) : KDevComponent( parent, name) {};
    ~KDevViewHandler() {};

public slots:
    /**
     * Adds a view to the view handler system.
     */
    virtual void addView( QWidget* w) = 0;
    /**
     * Remove a view from the view handler system.
     */
    virtual void removeView( QWidget* w) = 0;
};

#endif  // _KDEVVIEWHANDLER_H_
