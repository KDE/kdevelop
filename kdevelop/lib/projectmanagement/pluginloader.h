/***************************************************************************
                          pluginloader.h  -  description
                             -------------------
    begin                : Sat Jun 3 2000
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <qstring.h>
#include <qlist.h>

class ProjectSpace;
class AppWizard;
class Project;
class QObject;
class ProjectSpace;

/**
  *@author Sandy Meier
  */

class PluginLoader {
public: 
	PluginLoader();
	~PluginLoader();
	// static
	static ProjectSpace* getNewProjectSpace(QString library_filename);
	/** returns a new projectspace, based on the pluginname from the configfile*/
	static ProjectSpace* getNewProjectSpaceFromConfigfile(QString filename);
	static AppWizard* getNewAppwizardPlugin(QString library_filename);
	static Project* getNewProject(QString projecttype_name,QObject* parent=0);
	static QString findLibrary(QString library_name);
	static QList<ProjectSpace>* getAllProjectSpaces(QObject* parent);
	static QList<AppWizard>* getAllAppWizards(QObject* parent);
};

#endif
