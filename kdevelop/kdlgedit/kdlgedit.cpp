/***************************************************************************
                          kdlgedit.cpp  -  description                              
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

#include "qdir.h"
#include "kdlgedit.h"
#include "../ckdevelop.h"
#include "kdlgeditwidget.h"
#include "kdlgitems.h"
#include "kdlgpropwidget.h"
#include "items.h"
#include "kdlgotherdlgs.h"
#include "kdlgnewdialogdlg.h"
#include "kdlgdialogs.h"

KDlgEdit::KDlgEdit(QObject *parentz, const char *name) : QObject(parentz,name)
{
   
   connect(((CKDevelop*)parent())->kdlg_get_dialogs_view(),SIGNAL(kdlgdialogsSelected(QString)),
	   SLOT(slotOpenDialog(QString)));
}

KDlgEdit::~KDlgEdit()
{
}


void KDlgEdit::slotFileNew(){
  CProject* prj = ((CKDevelop*)parent())->getProject(); 
  TDialogFileInfo info;
  if(prj != 0){
    KDlgNewDialogDlg dlg(((QWidget*) parent()),"I",prj);
    if( dlg.exec()){
      // get the location
      QString location = dlg.getLocation();
      info.rel_name = prj->getSubDir() + dlg.getClassname().lower() + ".kdevdlg";
      info.dist = true;
      info.install = false;
      info.classname = dlg.getClassname();
      info.baseclass = dlg.getBaseClass();
      info.header_file = getRelativeName(location + dlg.getHeaderName());
      info.source_file = getRelativeName(location + dlg.getSourceName());
      info.data_file = getRelativeName(location + dlg.getDataName());
      info.is_toplevel_dialog = true;

      dialog_file = prj->getProjectDir() + info.rel_name;

      if(prj->addDialogFileToProject(info.rel_name,info)){
	((CKDevelop*)parent())->newSubDir();
      }
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(dialog_file);
      ((CKDevelop*)parent())->refreshTrees();
    }
  }
}

void KDlgEdit::slotFileOpen()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->openFromFile("/tmp/dialog.kdevdlg");
}
void KDlgEdit::slotOpenDialog(QString file){
  slotFileSave();
  dialog_file = file;
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->openFromFile(file);
  ((CKDevelop*)parent())->setCaption(i18n("KDevelop Dialog Editor: ")+file); 
}

void KDlgEdit::slotFileClose()
{
}

void KDlgEdit::slotFileSave()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(dialog_file);
}
	
void KDlgEdit::slotEditUndo()
{
}

void KDlgEdit::slotEditRedo()
{
}

void KDlgEdit::slotEditCut()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_cutSelected();
}

void KDlgEdit::slotEditDelete()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_deleteSelected();
}

void KDlgEdit::slotEditCopy()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_copySelected();
}

void KDlgEdit::slotEditPaste()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_pasteSelected();
}

void KDlgEdit::slotEditProperties()
{
} 	

void KDlgEdit::slotViewRefresh()
{
  ((CKDevelop*)parent())->kdlg_get_items_view()->refreshList();

  KDlgItem_Base* sel = ((CKDevelop*)parent())->kdlg_get_edit_widget()->selectedWidget();

  if (sel)
    ((CKDevelop*)parent())->kdlg_get_prop_widget()->refillList(sel);

  ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->recreateItem();
}

void KDlgEdit::slotBuildGenerate()
{
}

void KDlgEdit::slotViewGrid()
{
  KDlgGridDialog dlg((QWidget*)parent());

  if (dlg.exec())
    {
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->setGridSize(dlg.getGridX(), dlg.getGridY());
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->repaintItem();
    }
}

QString KDlgEdit::getRelativeName(QString abs_filename){
  CProject* prj = ((CKDevelop*)parent())->getProject();
  // normalize it a little bit
  abs_filename.replace(QRegExp("///"),"/"); // remove ///
  abs_filename.replace(QRegExp("//"),"/"); // remove //
  abs_filename.replace(QRegExp(prj->getProjectDir()),"");
  return abs_filename;
}
  
