/***************************************************************************                      
    crealfileview.h - display all files and dirs in the projectdir
                             -------------------                                         

    version              :                                   
    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Bartel                         
    email                : bartel@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef KPROJECTDIRTREELIST_H
#define KPROJECTDIRTREELIST_H

#include <kapp.h>
#include <qwidget.h>
#include <ktreelist.h>
#include <kiconloader.h>

/**
  *display all files and dirs in the projectdir, a little filemanager
  @author Stefan Bartel 
  */
class CRealFileView : public KTreeList {
  Q_OBJECT 
public: 
  /** construtor */
  CRealFileView(QWidget*parent=0,const char* name=0); 
  /** destructor */
  ~CRealFileView();
  /** starts the scan */
  void refresh(QString projectdir);

protected:
  /** scans the project directory and fills the treelist */
  void scanDir(const QString& directory,KPath& path);
private:
KIconLoader* loader;

};
#endif
