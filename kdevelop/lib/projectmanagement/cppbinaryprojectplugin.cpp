/***************************************************************************
                          cppbinaryprojectplugin.cpp  -  description
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

#include "cppbinaryprojectplugin.h"

CPPBinaryProjectPlugin::CPPBinaryProjectPlugin(){
}
CPPBinaryProjectPlugin::~CPPBinaryProjectPlugin(){
}

QStringList CPPBinaryProjectPlugin::getSources(QString binary_name=""){
}

  /** Fetch the commandline execution arguments for the project binary
      if no binary_name was given, the first binary is returned
   */
  QString CPPBinaryProjectPlugin::getExecuteArgs(QString args,QString binary_name=""){
  }

  /** Fetch the commandline execution arguments for debugging
      if no binary_name was given, the first binary is returned
  */
  QString CPPBinaryProjectPlugin::getDebugArgs(QString args,QString binary_name=""){
  }

  /** Store the commandline execution arguments for the project binary
      if no binary_name was given, the first binary is meant
  */
  void CPPBinaryProjectPlugin::setExecuteArgs(QString args,QString binary_name=""){
  }

  /** Store the commandline execution arguments for debugging
      if no binary_name was given, the first binary is meant
  */
  void CPPBinaryProjectPlugin::setDebugArgs(QString args,QString binary_name=""){
  }
