/***************************************************************************
                          kdlgedit.cpp  -  description                              
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


#include "kdlgedit.h"

KDlgEdit::KDlgEdit(QObject *parent, const char *name ) : QObject(parent,name) {
}
KDlgEdit::~KDlgEdit(){
}


void KDlgEdit::slotFileNew(){
}
void KDlgEdit::slotFileOpen(){
}
void KDlgEdit::slotFileClose(){
}
void KDlgEdit::slotFileSave(){
}

	
void KDlgEdit::slotEditUndo(){
}
void KDlgEdit::slotEditRedo(){
}
void KDlgEdit::slotEditCut(){
}
void KDlgEdit::slotEditCopy(){
}
void KDlgEdit::slotEditPaste(){
}
void KDlgEdit::slotEditProperties(){
} 	


void KDlgEdit::slotViewRefresh(){
}

void KDlgEdit::slotBuildGenerate(){
}




