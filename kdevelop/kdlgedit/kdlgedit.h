/***************************************************************************
                          kdlgedit.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
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
class CProject;

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */

class KDlgEdit : public QObject  {
  Q_OBJECT
public: 
  KDlgEdit(QObject *parent=0, const char *name=0);
  ~KDlgEdit();
  QString getRelativeName(QString abs_filename);
  

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
  /** cut out the selected widget*/
  void slotEditCut();
  /** delete the selected widget*/
  void slotEditDelete();
  /** copy the selected widget to the clipboard */
  void slotEditCopy();
  /** insert the widget which is on the clipboard */
  void slotEditPaste();
  /** show properties of the widget- opens the properties window */
  void slotEditProperties();
  /** refreshes the view */
  void slotViewRefresh();
  /** pops up a dialog allowing the user to change the grid sizes */
  void slotViewGrid();
  
  /** generates the sources for the current edited widget */
  void  slotBuildGenerate();

  void slotOpenDialog(QString file);
protected:
  CProject* project;
  QString dialog_file;
  
};

#endif





