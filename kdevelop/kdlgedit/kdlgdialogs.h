/***************************************************************************
                          kdlgdialogs.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 Sandy Meier
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


#ifndef KDLGDIALOGS_H
#define KDLGDIALOGS_H

#include <qwidget.h>
#include "../ctreeview.h"
class CProject;

/**
  *@author Sandy Meier <smeier@rz.uni-potsdam.de>
  */

class KDlgDialogs : public CTreeView  {
  Q_OBJECT
public: 
  KDlgDialogs(QWidget *parent=0, const char *name=0);
  ~KDlgDialogs();
  void refresh(CProject* prj);
  /** Initialize popupmenus. */
  virtual void initPopups();
  /** Get the current popupmenu. */
  virtual KPopupMenu *getCurrentPopup();
  
  signals:
  void kdlgdialogsSelected(QString dialog_file);
  void newDialog();
public  slots:
void slotSelectionChanged( QListViewItem* item);
  void slotNewDialog();
protected:
CProject*  project;
  KPopupMenu dialog_pop;
  QString current_dialog;
};

#endif
