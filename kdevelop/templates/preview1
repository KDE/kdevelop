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
#include <kpopmenu.h>

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
  bool isClass(int index);
  bool isMethod(int index);
  void CVRemoveAllComments(QString* str);
  // variables for the classview
  QList<TStreamedFile>* streamed_files;
  QList<TClassInfo>* class_infos;
  bool leftButton();
  bool rightButton();
protected:
protected slots:
  void slotSingleSelected(int index);
  void slotProjectOptions();
  void slotFileNew();
  void slotClassNew();
  void slotClassRemove();
  void slotClassDelete();
  void slotMethodNew();
  void slotVariableNew();
  void slotViewDeclaration();
  void slotViewDefinition();

  signals:
  void selectedFileNew();
  void selectedClassNew();
  void selectedProjectOptions();
  void selectedViewDeclaration(int index);
  void selectedViewDefinition(int index);

protected:
  void mousePressEvent(QMouseEvent* event);
  
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
  KPopupMenu*  class_pop;
  KPopupMenu*  member_pop;  
  KPopupMenu* project_pop;
  bool left_button;
  bool right_button;
  QPoint mouse_pos; // the position at the last mousepress-event

private:
  KIconLoader* icon_loader;  
};
#endif


















