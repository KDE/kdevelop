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
  /** New Dialog - this is not for an extra dialog but is called if the kdevelop file new selects it for creating a new dialog */
	void slotFileNew();
	/** an open file dialog with the *.kdevdlg selector */
	void slotFileOpen();
	/** closes the current dialog and dialogfile- including created sourcefiles */
	void slotFileClose();
	/** save the dialog file -- request if files should be generated and saved */
  void slotFileSave();
		
  /** undo the last editing step --this maybe changing properties or movements */
	void slotEditUndo();
	/** redo the last editing step */
	void slotEditRedo();
	/** cut out the selected widget or parameter in the properties view */
	void slotEditCut();
	/** copy the selected widget to the clipboard or the parameter of the properties view*/
	void slotEditCopy();
	/** insert the widget which is on the clipboard or the parameter on the clipboard */
	void slotEditPaste();
	/** show properties of the widget- opens the properties window */
	void slotEditProperties();
	/** refreshes the view */
	void slotViewRefresh();

	/** generates the sources for the current edited widget */
	void  slotBuildGenerate();

};

#endif





