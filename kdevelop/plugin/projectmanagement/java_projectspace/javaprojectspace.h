/***************************************************************************
                          javaprojectspace.h  -  description
                             -------------------
    begin                : Thu May 25 2000
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

#ifndef JAVAPROJECTSPACE_H
#define JAVAPROJECTSPACE_H

#include <qwidget.h>
#include "projectspace.h"

/**
  *@author Sandy Meier
  */

class JavaProjectSpace : public ProjectSpace  {
   Q_OBJECT
public: 
	JavaProjectSpace(QObject *parent=0, const char *name=0);
	~JavaProjectSpace();

};

#endif
