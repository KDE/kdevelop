/***************************************************************************
                    clogfileview.h - the logical file view,draw the contents of 
		           a projectfile into a tree
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
#ifndef CLOGFILEVIEW_H
#define CLOGFILEVIEW_H

#include <qwidget.h>
#include <kapp.h>
#include "cproject.h"
#include <kpopmenu.h>
#include "ctreeview.h"

/** the logical-file-view,draw the contents of 
  * a cprojectinfo into a tree
  *@author Sandy Meier
  */

class CLogFileView : public CTreeView {
  Q_OBJECT 
public: 

  /** construtor */
  CLogFileView(QWidget*parent=0,const char* name=0); 

  /** destructor */
  ~CLogFileView();
    
  /** starts the refresh */
  void refresh(CProject* prj);

protected: // Implementations of virtual methods.

  /** Initialize popupmenus. */
  void initPopups();

  /** Get the current popupmenu. */
  KPopupMenu *getCurrentPopup();

protected:
  void split(QString str,QStrList& filters);
 protected slots:
    void slotSelectionChanged( QListViewItem *);
    void slotNewClass();
    void slotNewFile();
    void slotNewGroup();
    void slotFileProp();
    void slotFileRemove();
    void slotFileDelete();
    void slotGroupProp();
    void slotGroupRemove();
    
 signals:
    void selectedNewClass();
    void selectedNewFile();
    void showFileProperties(QString);
    void selectedFileRemove();
    void selectedGroupProp();
    void logFileTreeSelected(QString);

protected:
  KPopupMenu file_pop;
  KPopupMenu group_pop;  
  KPopupMenu project_pop;

  CProject* project;
};
#endif
