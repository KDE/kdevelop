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
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "kdevcore.h"

PHPCodeCompletion::PHPCodeCompletion(KDevCore* core,ClassStore* store){
  m_editor = core->editor();
  m_core = core;
  m_classStore = store;

  connect(m_editor, SIGNAL(documentActivated(KEditor::Document*)),
	  this, SLOT(documentActivated(KEditor::Document*)));
  
  KEditor::CompletionEntry e;
  e.prefix = "int";
  e.text = "mysql_affected_rows";
  e.postfix ="()";
  e.comment = "Get number of affected rows in previous MySQL operation.";
  m_globalFunctions.append(e);
  
  e.prefix = "int";
  e.text = "mysql_change_user";
  e.postfix ="()";
  e.comment = "Change logged in user of the active connection.";
  m_globalFunctions.append(e);

  e.prefix = "resource";
  e.text = "mysql_db_query";
  e.postfix ="()";
  e.comment = "Send a MySQL query";
  m_globalFunctions.append(e);

  e.prefix = "array";
  e.text = "mysql_fetch_array";
  e.postfix ="()";
  m_globalFunctions.append(e);

  e.prefix = "array";
  e.text = "mysql_fetch_assoc";
  e.postfix ="()";
  m_globalFunctions.append(e);

  e.prefix = "int";
  e.text = "mysql_num_fields";
  e.postfix ="()";
  m_globalFunctions.append(e);
   
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
  
  QString lineStr = e_iface->line(line);
  QString restLine = lineStr.mid(col);
  restLine.replace( QRegExp("[ \t]*"), "" );
  if(!restLine.isEmpty()){
    cerr << endl << "no codecompletion because no empty line after cursor";
    return;
  }

  if(checkForGlobalFunction(doc,lineStr,col)) {
    return;
  }
  if(checkForClassMember(doc,lineStr,col,line)) {
    return;
  }
  // $test = new XXX
  if(checkForNewInstance(doc,lineStr,col,line)){
    return;
  }


  
}

bool PHPCodeCompletion::checkForGlobalFunction(KEditor::Document *doc,QString lineStr,int col){
  QString methodStart ="";
  if(lineStr.length()==2){
    methodStart = lineStr;
  }
  else{
    QString startStr =lineStr.mid(col-3);
    if(startStr.isNull()){
      return false; // not enough letters
    }
    QString startChar = startStr.left(1);
    if(startChar == " " || startChar == "\t" || startChar == "+" || 
       startChar == "-" || startChar == "=" ||startChar == "/" || startChar == "*" || startChar == ";"){
      methodStart = startStr.right(2);
    }
  }
  if(methodStart != ""){
    // ok it is an global function
    cerr << "Methodstart" << methodStart;
    QValueList<KEditor::CompletionEntry> list;
    QValueList<KEditor::CompletionEntry>::Iterator it;
    for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
      if((*it).text.startsWith(methodStart)){
	list.append((*it));
      }
    }

    QList<ParsedMethod>* methodList = m_classStore->globalContainer.getSortedMethodList();
     for ( ParsedMethod *pMethod = methodList->first(); pMethod != 0;pMethod = methodList->next() ) {
      if(pMethod->name().startsWith(methodStart)){
	KEditor::CompletionEntry e;
	e.text = pMethod->name();
	e.postfix ="()";
	list.append(e);
      }
    }

    if(list.count() >0){
      KEditor::CodeCompletionDocumentIface* compl_iface = KEditor::CodeCompletionDocumentIface::interface(doc);
      compl_iface->showCompletionBox(list,2);
      return true;
    }
  }
  return false;
}

bool PHPCodeCompletion::checkForNewInstance(KEditor::Document *doc,QString lineStr,int col,int line){
  KRegExp newre("=[ \t]*new[ \t]+([A-Za-z_]+)");
  if(newre.match(lineStr)){
    if(lineStr.right(2) == newre.group(1)){
      cerr << "CLASS: " << newre.group(1);
      QValueList<KEditor::CompletionEntry> list;
      QList<ParsedClass>* classList = m_classStore->globalContainer.getSortedClassList();
      for ( ParsedClass *pclass = classList->first(); pclass != 0;pclass =classList->next() ) {
	if(pclass->name().startsWith(newre.group(1))){
	  KEditor::CompletionEntry e;
	  e.text = pclass->name();
	  list.append(e);
	}
      }
      if(list.count() >0){
	KEditor::CodeCompletionDocumentIface* compl_iface = KEditor::CodeCompletionDocumentIface::interface(doc);
	compl_iface->showCompletionBox(list,2);
	return true;
      }
    }
    return false;
  }
}
bool PHPCodeCompletion::checkForClassMember(KEditor::Document *doc,QString lineStr,int col,int line){
  if(lineStr.right(7) != "$this->"){
    cerr << endl << "$this-> not found";
    return false;
  }
  // now search the current class
  KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  KEditor::EditDocumentIface *e_iface = KEditor::EditDocumentIface::interface(doc);
  for(int i=line;i>=0;i--){
    QString lineStr = e_iface->line(i);  
    if(classre.match(lineStr)) { // ok found
      QString className = classre.group(1);
      QValueList<KEditor::CompletionEntry> list = getClassMethodsAndVariables(className);
      KEditor::CodeCompletionDocumentIface* compl_iface = KEditor::CodeCompletionDocumentIface::interface(doc);
      compl_iface->showCompletionBox(list);
      return true;
    }
  }
  return false;
}

QValueList<KEditor::CompletionEntry> PHPCodeCompletion::getClassMethodsAndVariables(QString className){
  QList<ParsedParent> parents;
  QValueList<KEditor::CompletionEntry> list;
  ParsedClass* pClass=0;
  do {
    pClass =  m_classStore->getClassByName(className);
    if(pClass !=0){
      QList<ParsedMethod> *methodList = pClass->getSortedMethodList();
      for ( ParsedMethod *pMethod = methodList->first();
	    pMethod != 0;
	    pMethod = methodList->next() ) {
	KEditor::CompletionEntry e;
	e.text = pMethod->name();
	e.postfix ="()";
	list.append(e);
      }
      QList<ParsedAttribute>* attList = pClass->getSortedAttributeList();
      for ( ParsedAttribute *pAttribut = attList->first();
	    pAttribut != 0;
	    pAttribut = attList->next() ) {
	KEditor::CompletionEntry e;
	QString name = pAttribut->name();
	e.text = name.remove(0,1); // remove the trailing $
	e.postfix ="";
	list.append(e);
      }


      if(pClass->parents.count() !=0){
	ParsedParent* parent = pClass->parents.first();
	className = parent->name();
      }
      else{
	className ="";
      }
    }
  } while (pClass != 0);
  return list;
}
#include "phpcodecompletion.moc"
