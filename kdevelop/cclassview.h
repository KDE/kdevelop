/***************************************************************************
                 classview.h - the classview for kdevelop
                             -------------------                                         

    version              :                                   
    begin                : 13 Aug 1998                                        
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

#ifndef CCLASSVIEW_H
#define CCLASSVIEW_H

//forward declaration
class CClassView;

#include <qstrlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <iostream.h>
#include <qregexp.h>
#include <ktreelist.h>
#include <kiconloader.h>
#include "structdef.h"
#include "cproject.h"

/** 
  *the classview in kdevelop attention: only a prototype 
  *@author Sandy Meier
*/

class CClassView : public KTreeList {
  Q_OBJECT 
public: 
  /** construtor */
  CClassView(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CClassView();
  void refresh(CProject* prj);
  void CVRemoveAllComments(QString* str);
  // variables for the classview
  QList<TStreamedFile>* streamed_files;
  QList<TClassInfo>* class_infos;
 public slots:
 
protected:
  
  void CVReadAllFiles();
  void CVFindTheClasses();
  void CVFindTheMethodsAndVars();
  int CVFindClassDecEnd(QString stream,int startpos);
  
  
  bool CVIsItAMethod(QString str);
  bool CVIsItAVariable(QString str);
  QString CVGetMethod(QString str);
  QString CVGetVariable(QString str);
  
  // return public,protected,private or nothing
  QString CVGetVisibility(QString str);

  CProject* prj_info;
private:
  KIconLoader* icon_loader;  
};
#endif
