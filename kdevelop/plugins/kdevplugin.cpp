/***************************************************************************
                          kdevplugin.cpp  
                             -------------------
    begin                : Thu Sep 22 1999
    copyright            : (C) 1999 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "kdevplugin.h"
#include <iostream.h>


KDevPlugin::KDevPlugin(){
}
KDevPlugin::~KDevPlugin(){
}
void KDevPlugin::init(TImportantPtrInfo info){
    kdev = info.kdev;
    plugin_menu = info.plugin_menu;
}
void KDevPlugin::start(){
}
void KDevPlugin::stop(){
}
