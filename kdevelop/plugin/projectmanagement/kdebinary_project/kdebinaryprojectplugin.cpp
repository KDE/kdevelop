/***************************************************************************
                          kdebinaryprojectplugin.cpp  -  description
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

#include "kdebinaryprojectplugin.h"
#include <iostream.h>


KDEBinaryProjectPlugin::KDEBinaryProjectPlugin(QObject *parent, const char *name) 
  : Project(parent,name){
  cerr  << "enter KDEBinaryProjectPlugin::KDEBinaryProjectPlugin" << endl;
}
KDEBinaryProjectPlugin::~KDEBinaryProjectPlugin(){
}
