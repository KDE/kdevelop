/***************************************************************************
                          plugin.h  -  description
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qpixmap.h>

/**Base Plugin for all KDevelop plugins,
    (AppWizardPlugins,ProjectPlugins,ComponentPlugin,PrintPlugin)
  *@author Sandy Meier
  */

class Plugin  {

public: 
	Plugin();
	virtual ~Plugin();
	QString getPluginName();
	 QString getPluginAuthor();
	 QString getPluginDescription();
	 QString getPluginCopyright();
   QString getPluginHomepage();
   QString getPluginEmail();
   QString getPluginVersion();
 	 QPixmap getPluginIcon();

 protected:
  QString m_plugin_name;
  QString m_plugin_author;
  QString m_plugin_description;
  QString m_plugin_copyright;
  QString m_plugin_homepage;
  QString m_plugin_email;
  QString m_plugin_version;
  QPixmap m_plugin_icon;
  QString m_plugin_libraryname;
};

#endif
