/***************************************************************************
                    cdoctree.h -
                             -------------------                                         

    version              :                                   
    begin                : 3 Oct 1998                                        
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
#ifndef CDOCTREE_H
#define CDOCTREE_H

#include <qwidget.h>
#include <ktreelist.h>
#include <kiconloader.h>
#include <kapp.h>
#include "cproject.h"


/** the documentation tree (manual,tutorial,KDE-libs...)
  *@author Sandy Meier
  */

class CDocTree : public KTreeList {
  Q_OBJECT 
public: 
  /** construtor */
  CDocTree(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CDocTree();
  /** starts the refresh */
  void refresh(CProject* prj);
  
private:
KIconLoader* icon_loader;

};
#endif
