/***************************************************************************
                          qobjectplugin.h  -  description
                             -------------------
    begin                : Sun Jun 4 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QOBJECTPLUGIN_H
#define QOBJECTPLUGIN_H

#include "plugin.h"
#include <qobject.h>

/**
  *@author Sandy Meier
  */

class QObjectPlugin : public QObject, public Plugin  {
   Q_OBJECT
public: 
	QObjectPlugin(QObject *parent=0, const char *name=0);
	virtual ~QObjectPlugin();
};

#endif
