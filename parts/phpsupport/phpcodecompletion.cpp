/***************************************************************************
                          phpcodecompletion.cpp  -  description
                             -------------------
    begin                : Tue Jul 17 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "phpcodecompletion.h"
#include "keditor/cursor_iface.h"
#include "keditor/codecompletion_iface.h"
#include <iostream.h>

PHPCodeCompletion::PHPCodeCompletion(KEditor::Editor* editor){
  m_editor = editor;
  connect(editor, SIGNAL(documentActivated(KEditor::Document*)),
	  this, SLOT(documentActivated(KEditor::Document*)));
  
}

PHPCodeCompletion::~PHPCodeCompletion(){
}

void PHPCodeCompletion::documentActivated(KEditor::Document* doc){
  cerr << endl << "PHPCodeCompletion::documentActivated";
  KEditor::CursorDocumentIface *c_iface = KEditor::CursorDocumentIface::interface(doc);
  if (!c_iface) { // no CursorDocument available
    cerr << endl << "editor doesn't support the CursorDocumentIface";
    return;
  } 
  disconnect( c_iface, 0, this, 0 ); // to make sure that it is't connected twice
  connect(c_iface,SIGNAL(cursorPositionChanged(KEditor::Document*, int, int)),
	  this,SLOT(cursorPositionChanged(KEditor::Document*, int, int))); 
}

void PHPCodeCompletion::cursorPositionChanged(KEditor::Document *doc, int line, int col){
  cerr << endl << "PHPCodeCompletion::cursorPositionChanged:" << line << ":" << col;
  KEditor::EditDocumentIface *e_iface = KEditor::EditDocumentIface::interface(doc);
  if (!e_iface) { // no CursorDocument available
    cerr << endl << "editor doesn't support the EditDocumentIface";
    return;
  }
  KEditor::CodeCompletionDocumentIface* compl_iface = KEditor::CodeCompletionDocumentIface::interface(doc);
  if (!compl_iface) { // no CodeCompletionDocument available
    cerr << endl << "editor doesn't support the CodeCompletionDocumentIface";
    return;
  }
  if(e_iface->line(line) == "test"){
    QValueList<KEditor::CompletionEntry> list;
    KEditor::CompletionEntry e1;
    e1.prefix = "resource";
    e1.text = "mysql_connect";
    e1.postfix ="()";
    KEditor::CompletionEntry e2;
    e2.prefix = "int";
    e2.text = "mysql_create_db";
    e2.postfix ="()";
    KEditor::CompletionEntry e3;
    e3.prefix = "array";
    e3.text = "mysql_fetch_assoc";
    e3.postfix ="()";
    
    list.append(e1);
    list.append(e2);
    list.append(e3);
    compl_iface->showCompletionBox(&list);
  }
}
