/***************************************************************************
                    cdocbrowser.h - a htmlview for kdevelop
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
#ifndef CDOCBROWSER_H
#define CDOCBROWSER_H


#include <qwidget.h>
#include <htmlview.h>
#include <kiconloader.h>
#include <kapp.h>
#include "structdef.h"

/** 
  *the documentation browser, attention!: only a prototype
  *@author Sandy Meier
  */
class CDocBrowser : public KHTMLView {
  Q_OBJECT 
public: 
  /** construtor */
  CDocBrowser(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CDocBrowser();
  /**show a html in a htmlview*/
  void showURL(QString url);
};
#endif
