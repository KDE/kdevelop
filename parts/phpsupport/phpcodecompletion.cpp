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
  
  FunctionCompletionEntry e;
  e.prefix = "int";
  e.text = "mysql_affected_rows";
  e.postfix ="()";
  e.comment = "Get number of affected rows in previous MySQL operation.";
  e.prototype = "int mysql_affected_rows ([resource link_identifier])";
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
  m_cursorInterface = KEditor::CursorDocumentIface::interface(doc);
  if (!m_cursorInterface) { // no CursorDocument available
    cerr << endl << "editor doesn't support the CursorDocumentIface";
    return;
  } 
  disconnect(m_cursorInterface, 0, this, 0 ); // to make sure that it is't connected twice
  connect(m_cursorInterface,SIGNAL(cursorPositionChanged(KEditor::Document*, int, int)),
	  this,SLOT(cursorPositionChanged(KEditor::Document*, int, int))); 
}

void PHPCodeCompletion::cursorPositionChanged(KEditor::Document *doc, int line, int col){

  //  cerr << endl << "PHPCodeCompletion::cursorPositionChanged:" << line << ":" << col;
  m_editInterface = KEditor::EditDocumentIface::interface(doc);
  if (!m_editInterface) { 
    cerr << endl << "editor doesn't support the EditDocumentIface";
    return;
  }
  m_codeInterface = KEditor::CodeCompletionDocumentIface::interface(doc);
  if (!m_codeInterface) { // no CodeCompletionDocument available
    cerr << endl << "editor doesn't support the CodeCompletionDocumentIface";
    return;
  }
  m_currentLine = line;
  QString lineStr = m_editInterface->line(line);
  QString restLine = lineStr.mid(col);
  if(restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull()){
    cerr << endl << "no codecompletion because no empty character after cursor:" << restLine << ":";    
    return;
  }
  if(checkForVariable(doc,lineStr,col,line)){
    return;
  }
  // $test = new XXX
  if(checkForNewInstance(doc,lineStr,col,line)){
    return;
  }
  if(checkForGlobalFunction(doc,lineStr,col)) {
    return;
  }
  //   if(checkForArgHint(doc,lineStr,col,line)){
  //    return;
  //   }
  
  /*
    QString lineStr = e_iface->line(line);
    }
    
    
  
    if(checkForClassMember(doc,lineStr,col,line)) {
    return;
    }

  */
}

bool PHPCodeCompletion::checkForVariable(KEditor::Document *doc,QString lineStr,int col,int line){
  QString methodStart = lineStr.left(col);
  if(methodStart.right(2) != "->"){
    cerr << endl << "checkForVariable: no '->' found";
    return false;
  }
  int varStart = methodStart.findRev("$");
  if(varStart ==-1){
    cerr << endl << "checkForVariable: no '$' (variable start) found";
    return false;
  }
  QString variableLine = methodStart.mid(varStart+1,methodStart.length() - 3);
  cerr << endl << variableLine;
  QString className ="";
  QStringList vars = QStringList::split("->",variableLine);
  for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
    className = this->getClassName("$" + (*it),className);
  }
  cerr << endl << "Classname:" << className;

  QValueList<KEditor::CompletionEntry> list = this->getClassMethodsAndVariables(className);
  if(list.count()>0){
    m_codeInterface->showCompletionBox(list);
    return true;
  }
  return false;
}

QString PHPCodeCompletion::getClassName(QString varName,QString maybeInstanceOf){
  if(varName == "$this"){
    return this->searchCurrentClassName();
  }
  if(maybeInstanceOf == ""){
    // ok, we need to search it
    return this->searchClassNameForVariable(varName);
  }
  ParsedClass* pClass =  m_classStore->getClassByName(maybeInstanceOf);
  if(pClass !=0){
    QList<ParsedAttribute>* attList = pClass->getSortedAttributeList();
    for ( ParsedAttribute *pAttribut = attList->first();
	  pAttribut != 0;
	  pAttribut = attList->next() ) {
      if(pAttribut->name() == varName){
	return pAttribut->type();
      }
    }
  }
  return "";
}

QString PHPCodeCompletion::searchClassNameForVariable(QString varName){
  KRegExp createVarRe("\\$" + varName.mid(1) + "[ \t]*=[ \t]*new[ \t]+([0-9A-Za-z_]+)");
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->line(i);  
    if(createVarRe.match(lineStr)) { // ok found
      cerr << endl << "match in searchClassNameForVariable:";
      return createVarRe.group(1);
    }
  }
  return "";
}

QString PHPCodeCompletion::searchCurrentClassName(){
  KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->line(i);  
    if(classre.match(lineStr)) { // ok found
      return classre.group(1);
    }
  }
}

bool PHPCodeCompletion::checkForArgHint(KEditor::Document *doc,QString lineStr,int col,int line){
  int leftBracket = lineStr.findRev("(",col);
  int rightBracket = lineStr.find(")",col);
  KRegExp functionre("([A-Za-z_]+)[ \t]*\\(");
  if(functionre.match(lineStr)){ // check for global functions
    QString name = functionre.group(1);
    int startMethod = lineStr.findRev(name,col);
    QString startString = lineStr.mid(0,startMethod);
    if(startString.right(2) != "->"){
      QValueList <QString> functionList;
      cerr << endl << "found global function";
      QValueList<FunctionCompletionEntry>::Iterator it;
      for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
	if((*it).text == name){
	  functionList.append((*it).prototype);
	}
      }
      QList<ParsedMethod>* methodList = m_classStore->globalContainer.getSortedMethodList();
      for ( ParsedMethod *pMethod = methodList->first(); pMethod != 0;pMethod = methodList->next() ) {
	if(pMethod->name() == name){
	  functionList.append("test");
	}
      }
      if(functionList.count() >0){
	KEditor::CodeCompletionDocumentIface* compl_iface = KEditor::CodeCompletionDocumentIface::interface(doc);
	compl_iface->showArgHint ( functionList, "()", "," );
	return true;
      }
    }
  }
  return false;
}
bool PHPCodeCompletion::checkForGlobalFunction(KEditor::Document *doc,QString lineStr,int col){
  cerr << endl << "enter checkForGlobalFunction";
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
    QValueList<FunctionCompletionEntry>::Iterator it;
    for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
      if((*it).text.startsWith(methodStart)){
	KEditor::CompletionEntry e;
	e = (*it);
	list.append(e);
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
      m_codeInterface->showCompletionBox(list,2);
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
	m_codeInterface->showCompletionBox(list,2);
	return true;
      }
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
