/***************************************************************************
                          cproject.h  -  description
                             -------------------
    begin                : Sun Oct 10 1999
    copyright            : (C) 1999 by ian geiser
    email                : geiseri@msoe.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPROJECT_H
#define CPROJECT_H

#include <qstring.h>

/**This is the container that will hold all of the data that is found in the KDevelop project file.
  *@author ian geiser
  *We are going to look for the following information in between the tags
  *[General] and [
  *<PRE>
  *[General]
  *author=
  *project_name=
  *version=
  *short_info=
  *email=
  *</PRE>
  We will need to get the entire line.
  */

class cProject {
public: 
	cProject();
	~cProject();
  /** This will load the selected project file and populate the internal data structure. */
  bool loadProject(QString qsFileName);
  /** Returns the short info from the current project. */
  QString getInfo();
  /** Returns the author & email of the current project.
It will be formatted as AuthorName <emailaddress> */
  QString getAuthor();
  /** Return the version number of the current project. */
  QString getVersion();
  /** Returns the current project name. */
  QString getProjectName();
  /** The configure options */
  QString getConfig();
private: // Private attributes
  /** The version number from the current project */
  QString qsVersion;
  /** The author of the current project */
  QString qsAuthor;
  /** The address of the current projects author */
  QString qsEmailAddress;
  /** The short info from the current project */
  QString qsInfo;
  /** The name of the current project */
  QString qsProjectName;
  /** The configure commands to build the application. */
  QString qsConfig;
};

#endif













