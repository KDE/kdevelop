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
}

void KDlgEdit::slotEditCopy()
{
}

void KDlgEdit::slotEditPaste()
{
}

void KDlgEdit::slotEditProperties()
{
} 	

void KDlgEdit::slotViewRefresh()
{
}

void KDlgEdit::slotBuildGenerate()
{
}



