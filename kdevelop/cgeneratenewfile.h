/***************************************************************************
                    cgeneratenewfile.h -
                             -------------------                                         

    version              :                                   
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

#include "cproject.h"
#include <kapp.h>
#include <qregexp.h>

/**
  *
  *@author Sandy Meier
  */
class CGenerateNewFile  {

public: 
  /** construtor */
  CGenerateNewFile(); 
  /** destructor */
  ~CGenerateNewFile();

  QString genCPPFile(QString abs_name,CProject* prj);
  QString genHeaderFile(QString abs_name,CProject* prj);
  QString genEngHandbook(QString abs_name,CProject* prj);
  QString genKDELnkFile(QString abs_name,CProject* prj);
  QString genLSMFile(QString abs_name,CProject* prj);
  QString genGNUFile(QString abs_name,CProject* prj);
  
};
#endif
