/***************************************************************************
                    cgeneratenewfile.h -
                             -------------------                                         

    begin                : 1 Oct  1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef CGENERATENEWFILE_H
#define CGENERATENEWFILE_H

#include <kapp.h>
#include <qregexp.h>

//#include "cproject.h"
class CProject;

/**
  * tool class for generating new files with templates
  *@author Sandy Meier
  */
class CGenerateNewFile  {

public: 
  /** construtor */
  CGenerateNewFile(); 
  /** destructor */
  ~CGenerateNewFile();

  /** specify file_name for other FILENAME in template. abs_name still is real filename */
  QString genCPPFile(QString abs_name,CProject* prj, QString file_name="");
  /** specify file_name for other FILENAME in template. abs_name still is real filename */
  QString genHeaderFile(QString abs_name,CProject* prj, QString file_name="");
  QString genEngHandbook(QString abs_name,CProject* prj);
  QString genEngDocbook(QString abs_name,CProject* prj);
  QString genLEXICALFile(QString abs_name,CProject* prj);
  QString genKDELnkFile(QString abs_name,CProject* prj);
  QString genLSMFile(QString abs_name,CProject* prj);
  QString genIcon(QString abs_name);
  QString genNifFile(QString abs_name);
  
};
#endif



