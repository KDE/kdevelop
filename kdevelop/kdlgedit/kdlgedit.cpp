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


#include "kdlgedit.h"
#include "../ckdevelop.h"
#include "kdlgeditwidget.h"
#include "kdlgitems.h"
#include "kdlgpropwidget.h"
#include "items.h"
#include "kdlgotherdlgs.h"

KDlgEdit::KDlgEdit(QObject *parent, const char *name ) : QObject(parent,name)
{
}

KDlgEdit::~KDlgEdit()
{
}


void KDlgEdit::slotFileNew()
{
}

void KDlgEdit::slotFileOpen()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->openFromFile("/tmp/dialog.kdevdlg");
}

void KDlgEdit::slotFileClose()
{
}

void KDlgEdit::slotFileSave()
{
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile("/tmp/dialog.kdevdlg");
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



