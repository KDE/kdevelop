/***************************************************************************
                          pmc.h  -  description
                             -------------------
    begin                : Sat May 13 2000
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

#ifndef PMC_H
#define PMC_H

#include <qwidget.h>
#include <qdialog.h>

/** project managment console, similar to KDEStudio or Visual C++
  *@author Sandy Meier
  */

class PMC : public QDialog  {
   Q_OBJECT
public: 
	PMC(QWidget *parent=0, const char *name=0);
	~PMC();
};

#endif
