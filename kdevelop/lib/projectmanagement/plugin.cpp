/***************************************************************************
                          plugin.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier (KDevelop Team)
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

#include "plugin.h"

Plugin::Plugin(){
}
Plugin::~Plugin(){
}
QString Plugin::getPluginName(){
	  return m_plugin_name;
}
QString Plugin::getPluginAuthor(){
  	return m_plugin_author;
}
QString Plugin::getPluginDescription(){
  	return m_plugin_description;
}
QString Plugin::getPluginCopyright(){
  	return m_plugin_copyright;
}
QString Plugin::getPluginHomepage(){
  	return m_plugin_homepage;
}
QString Plugin::getPluginVersion(){
	 	return m_plugin_version;
}
QPixmap Plugin::getPluginIcon(){
	return m_plugin_icon;
}
QString Plugin::getPluginEmail(){
 return m_plugin_email;
}
