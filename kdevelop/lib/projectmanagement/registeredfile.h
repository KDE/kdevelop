/***************************************************************************
   - a registred file in the project-
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REGISTEREDFILE_H
#define REGISTEREDFILE_H

#include <qstring.h>

/**
  *@author Sandy Meier
  */

class RegisteredFile {
public: 
  RegisteredFile(QString rel_name,bool dist=false,bool install=false,QString install_file="");
  ~RegisteredFile();
  
  /** set the filename
      @param rel_name this should be the filename incl. the relative path, 
      relative to the project f.e. ../../test.cpp
  */	
  void setRelativeFile(QString rel_name);
  /* return the relative file*/
  QString getRelativeFile();
  
  void setInstallFile(QString abs_file);
  QString getInstallFile();
  
  void setToDist(bool yes=true);
  bool isInDist();
  
  void setInstall(bool yes=true);
  bool shouldBeInstalled();
  
 protected:
  QString m_file;
  bool m_dist;
  bool m_install;
  QString m_install_file;
};

#endif
