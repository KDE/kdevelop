/***************************************************************************
                     structdef.h -- some important struct definitions
                             -------------------                                         

    version              :                                   
    begin                : 20 Jul 1998                                        
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

#ifndef STRUCTDEF_H
#define STRUCTDEF_H

#include <qlist.h>
#include <qstring.h>

/** (struct) for the classview
  *@author Sandy Meier
  */
struct TStreamedFile {
  QString filename;
  QString stream;
};
/** (struct) for the classview
  *@author Sandy Meier
  */
struct TMethodInfo {
  QString name;
  QString visibility; // protected,private or public
};
/** (struct) for the classview
  *@author Sandy Meier
  */
struct TVarInfo{
  QString name;
  QString visibility;
};
/** (struct) for the classview
  *@author Sandy Meier
  */
struct TClassInfo {
  QString filename; // in which file was the class declared
  /** struct or class*/
  QString type; 
  QString classname; // the name of the class
  /**the cursorpostion in the file, at Position {, only correct at parsing*/
  int begin;    
  /** the cursorpostion in the file, at Position }*/
  int end;     
  /** methods informations*/
  QList<TMethodInfo> method_infos;
  /** variables informations*/
  QList<TVarInfo> variable_infos;
};




/** (struct) contains the internal representation of a textfile
  *@author Sandy Meier
  */
struct TEditInfo {
  int id; // the ID in the buffer-menu
  QString filename;
  QString text; // 
    bool modified; // modified ?
};

#endif
