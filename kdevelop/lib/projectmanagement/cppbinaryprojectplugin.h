/***************************************************************************
                          cppbinaryprojectplugin.h  -  description
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

#ifndef CPPBINARYPROJECTPLUGIN_H
#define CPPBINARYPROJECTPLUGIN_H

#include "project.h"

/** support maybe? multiple targets (client,server)
  *@author Sandy Meier
  */

class CPPBinaryProjectPlugin : public Project  {
public: 
	CPPBinaryProjectPlugin();
	~CPPBinaryProjectPlugin();
	
		QStringList getSources(QString binary_name="");

  /** Fetch the commandline execution arguments for the project binary
      if no binary_name was given, the first binary is returned
   */
  virtual QString getExecuteArgs(QString args,QString binary_name="");

  /** Fetch the commandline execution arguments for debugging
      if no binary_name was given, the first binary is returned
  */
  virtual QString getDebugArgs(QString args,QString binary_name="");

  /** Store the commandline execution arguments for the project binary
      if no binary_name was given, the first binary is meant
  */
  virtual void setExecuteArgs(QString args,QString binary_name="");

  /** Store the commandline execution arguments for debugging
      if no binary_name was given, the first binary is meant
  */
  virtual void setDebugArgs(QString args,QString binary_name="");

};

#endif
