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
#include <qtextstream.h>
#include <qstrlist.h>
#include "../component.h"

class CProject;
class TDialogFileInfo;
class KDlgItem_QWidget;
class CTabCtl;
class KDlgEditWidget;
class KDlgPropWidget;
class KDlgWidgets;
class KDlgDialogs;
class KDlgItems;
class CKDevelop;

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */

class KDlgEdit : public QObject, public Component
{
  Q_OBJECT
public: 
  KDlgEdit(CKDevelop *ckdevelop, QWidget *parent, const char *name=0);
  ~KDlgEdit();
  QString getRelativeName(QString abs_filename);
  void helpForTopic(const QString &str);
  void newDialog();
  void initWhatsThis();
  void showPropWidget(bool b);
  void showTabWidget(bool b);
  void enableDialogsTab(bool b);

public slots:
/** New Dialog - this is not for an extra dialog but is called if the kdevelop file new selects it for creating a new dialog */
void slotFileNew();
  
 bool  slotFileCloseForceSave(); 
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
  void generateCommon(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateWidget(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);

  void generateQLCDNumber(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQPushButton(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQLineEdit(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQRadioButton(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQCheckBox(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQLabel(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQWidget(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQComboBox(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQListBox(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQMultiLineEdit(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQProgressBar(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQGroupBox(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQSpinBox(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQSlider(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQScrollBar(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateQListView(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKColorButton(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKDatePicker(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKDateTable(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKKeyButton(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKLed(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKProgress(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKRestrictedLine(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
  void generateKSeparator(KDlgItem_QWidget *wid, QTextStream *stream,QString parent);
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

protected:
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    
public:
  CTabCtl* kdlg_get_tabctl()             { return  kdlg_tabctl;}
  KDlgEditWidget* kdlg_get_edit_widget() { return kdlg_edit_widget; }
  KDlgPropWidget* kdlg_get_prop_widget() { return kdlg_prop_widget; }
  KDlgWidgets* kdlg_get_widgets_view()   { return kdlg_widgets_view; }
  KDlgDialogs* kdlg_get_dialogs_view()   { return kdlg_dialogs_view; }
  KDlgItems*   kdlg_get_items_view()     { return kdlg_items_view; }
  CKDevelop *kdlg_get_ckdevelop()        { return ckdev; }
    
private:  
  /** The tabbar for the kdlg view. */
  CTabCtl* kdlg_tabctl;
  /** The editing view of kdlg. */
  KDlgEditWidget* kdlg_edit_widget;
  /** The properties window of kdlg. */
  KDlgPropWidget* kdlg_prop_widget;
  /** The first tab of kdlg_tabctl. */
  KDlgWidgets* kdlg_widgets_view;
  /** The second tab of kldg_tabctl. */
  KDlgDialogs* kdlg_dialogs_view;
  /** the third tab of kldg_tabctl. */
  KDlgItems*   kdlg_items_view;
  CKDevelop *ckdev;
};

#endif
