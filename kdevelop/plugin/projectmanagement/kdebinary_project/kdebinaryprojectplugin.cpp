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
#include <kiconloader.h>

KDEBinaryProjectPlugin::KDEBinaryProjectPlugin(QObject *parent, const char *name) 
  : Project(parent,name){
  cerr  << "enter KDEBinaryProjectPlugin::KDEBinaryProjectPlugin" << endl;
  // init, every plugin should do this
  // Plugin infos
  m_plugin_author = "KDevelop Team";
  m_plugin_name = "KDEBinaryProject";
  m_plugin_copyright = "(C) 2000 by KDevelop Team";
  m_plugin_version = "0.1";
  m_plugin_description = "KDE Binary plugin";
  m_plugin_homepage = "http://www.kdevelop.org";
  m_plugin_icon = DesktopIcon("java_src");
 
}
KDEBinaryProjectPlugin::~KDEBinaryProjectPlugin(){
}
