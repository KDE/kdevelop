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
#include "../cproject.h"
#include "items.h"
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
  
  /** closes the current dialog and dialogfile- including created sourcefiles */
  bool slotFileClose();
  /** save the dialog file -- request if files should be generated and saved */
  void slotFileSave();
  void slotFileSaveAs();
  
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
  /** uses kdlgloader to provide a wysiwyg preview. */
  void slotViewPreview();
  
  void buildGenerate(bool force_get_classname_dialog);
  /** generates the sources for the current edited widget */
  void slotBuildGenerate();
  void slotBuildCompleteGenerate();
  void  generateSourcecodeIfNeeded();

  void slotOpenDialog(QString file);
  void slotDeleteDialog(QString file);
  
  protected:
  void generateWidget(KDlgItem_Widget *wid, QTextStream *stream,QString parent);

  void generateQLCDNumber(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQPushButton(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQLineEdit(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQRadioButton(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQCheckBox(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQLabel(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQWidget(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQComboBox(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQListBox(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQMultiLineEdit(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQProgressBar(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQGroupBox(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQSpinBox(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQSlider(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQScrollBar(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateQListView(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKColorButton(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKCombo(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKDatePicker(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKDateTable(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKKeyButton(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKLed(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKLedLamp(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKProgress(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKRestrictedLine(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKSeparator(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateKTreeList(KDlgItem_Widget *wid, QTextStream *stream,QString parent);
  void generateInitialHeaderFile(TDialogFileInfo info,QString baseclass_header);
  void generateInitialSourceFile(TDialogFileInfo info);

  
  CProject* project;
  QString dialog_file;
  /** filled at the generation process, needed for the header*/
  QStrList variables;
  /** filled at the generation process, needed for the header*/
  QStrList includes;
  /** the includes for the data source file, f.e. QPixmap*/
  QStrList local_includes; 
};

#endif





