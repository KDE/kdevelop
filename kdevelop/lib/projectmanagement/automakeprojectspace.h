/***************************************************************************
                          automakeprojectspace.h  -  description
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

#ifndef AUTOMAKEPROJECTSPACE_H
#define AUTOMAKEPROJECTSPACE_H

#include "projectspace.h"

/**
  *@author Sandy Meier
  */

class AutomakeProjectSpace : public ProjectSpace  {
Q_OBJECT
  public: 
 AutomakeProjectSpace(QObject* parent=0,const char* name=0);
 virtual ~AutomakeProjectSpace();
 
 virtual void setupGUI();
 virtual void modifyDefaultFiles();
 /** Store the additional arguments for configure,stored in the *_user files */
  void setConfigureArgs(QString args);
  /** Fetch the commandline additional arguments for configure */
  QString configureArgs();

  /** add the data to the psElement (Projectspace)*/
  virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool writeUserConfig(QDomDocument& doc,QDomElement& psElement);

  virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool readUserConfig(QDomDocument& doc,QDomElement& psElement);

  protected slots:
  virtual void updateMakefilesAm();

 protected:
  QString  m_configureArgs;

};

#endif
