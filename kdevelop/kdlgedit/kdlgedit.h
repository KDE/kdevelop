/***************************************************************************
                          kdlgedit.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGEDIT_H
#define KDLGEDIT_H

#include <qobject.h>

/**
  *@author 
  */

class KDlgEdit : public QObject  {
   Q_OBJECT
public: 
	KDlgEdit(QObject *parent=0, const char *name=0);
	~KDlgEdit();

public slots:
  void slotFileNew();
	void slotFileOpen();
	void slotFileClose();
  void slotFileSave();
		
	void slotEditUndo();
	void slotEditRedo();
	void slotEditCut();
	void slotEditCopy();
	void slotEditPaste();
	void slotEditProperties();
	
	void slotViewRefresh();
};

#endif




