/***************************************************************************
                          kdebinaryprojectplugin.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEBINARYPROJECTPLUGIN_H
#define KDEBINARYPROJECTPLUGIN_H

#include "project.h"

/**
  *@author Sandy Meier
  */

class KDEBinaryProjectPlugin : public Project  {
  Q_OBJECT
public: 
	KDEBinaryProjectPlugin( QObject *parent=0, const char *name=0);
	~KDEBinaryProjectPlugin();
};

#endif
