/***************************************************************************
                          javaprojectspace.cpp  -  description
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

#include "javaprojectspace.h"
#include <iostream.h>
#include <kiconloader.h>

JavaProjectSpace::JavaProjectSpace(QObject *parent, const char *name )
				: ProjectSpace(parent,name) {
	cerr << "JavaProjectSpacePlugin:I'm a plugin";
	
	// init, every plugin should do this
	m_plugin_author = "KDevelop Team";
	m_plugin_name = "Java";
	m_plugin_copyright = "(C) 2000 by KDevelop Team";
	m_plugin_version = "0.1";
	m_plugin_description = "kk";
	m_plugin_homepage = "http://www.kdevelop.org";
	m_plugin_icon = DesktopIcon("java_src");
}
JavaProjectSpace::~JavaProjectSpace(){
}

