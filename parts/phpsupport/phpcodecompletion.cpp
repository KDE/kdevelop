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
#include <iostream>
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "kdevcore.h"
#include <qfile.h>
#include <qtextstream.h>
#include "phpsupportfactory.h"
#include "phpsupportpart.h"
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>

using namespace std;

PHPCodeCompletion::PHPCodeCompletion(PHPSupportPart *part,KDevCore* core,ClassStore* store){
  m_core = core;
  m_classStore = store;
  m_argWidgetShow = false;
  m_completionBoxShow=false;

  QObject::connect(part, SIGNAL(activePartChanged(KParts::Part*)),
		   this, SLOT(activePartChanged(KParts::Part*)));

  readGlobalPHPFunctionsFile();
}

PHPCodeCompletion::~PHPCodeCompletion(){
}

void PHPCodeCompletion::readGlobalPHPFunctionsFile(){
  KStandardDirs *dirs = PHPSupportFactory::instance()->dirs();
  QString phpFuncFile = dirs->findResource("data","kdevphpsupport/phpfunctions");
  KRegExp lineReg(":([0-9A-Za-z_]+) ([0-9A-Za-z_]+)(\\(.*\\))");
  FunctionCompletionEntry e;
  QFile f(phpFuncFile);
  if ( f.open(IO_ReadOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream
      QString s;
      while ( !t.eof() ) {        // until end of file...
	  s = t.readLine();       // line of text excluding '\n'
	  if(lineReg.match(s.latin1())){
	      e.prefix = lineReg.group(1);
	      e.text = lineReg.group(2);
	      //	      if(QString(lineReg.group(3)) == "void"){
		e.postfix ="()";
		//	      }
		//	      else{
		//	      e.postfix ="(...)";
		//	      }
	      e.prototype = QString(lineReg.group(1)) + " " + QString(lineReg.group(2)) +
		  "(" + QString(lineReg.group(3)) + ")";
	      m_globalFunctions.append(e);
	  }

      }
      f.close();
  }

}
void PHPCodeCompletion::argHintHided(){
  //  cerr << "PHPCodeCompletion::argHintHided" << endl ;
  m_argWidgetShow = false;
}
void PHPCodeCompletion::completionBoxHided(){
  //  cerr << "PHPCodeCompletion::completionBoxHided()" << endl ;
  m_completionBoxShow=false;
}

void PHPCodeCompletion::activePartChanged(KParts::Part *part)
{
  if (!part || !part->widget())
    return;

  kdDebug() << endl << "PHPCodeCompletion::documentActivated";

  m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
  if (!m_editInterface)
  {
    kdDebug() << "editor doesn't support the EditDocumentIface" << endl;
    return;
  }

  m_cursorInterface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  if (!m_cursorInterface)
  {
    kdDebug() << "editor does not support the ViewCursorInterface" << endl;
    return;
  }
  
  m_codeInterface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
  if (!m_codeInterface) { // no CodeCompletionDocument available
    kdDebug() << "editor doesn't support the CodeCompletionDocumentIface" << endl;
    return;
  }

  disconnect(part, 0, this, 0 ); // to make sure that it is't connected twice
  connect(part, SIGNAL(cursorPositionChanged()),
          this, SLOT(cursorPositionChanged()));
  connect(part, SIGNAL(argHintHided()), this, SLOT(argHintHided()));
  connect(part, SIGNAL(completionAborted()), this, SLOT(completionBoxHided()));
  connect(part, SIGNAL(completionDone()), this, SLOT(completionBoxHided()));

}

void PHPCodeCompletion::cursorPositionChanged(){
  //  cerr << endl << "PHPCodeCompletion::cursorPositionChanged:" << line << ":" << col;

  uint line, col;
  m_cursorInterface->cursorPosition(&line, &col);
  m_currentLine = line;
  QString lineStr = m_editInterface->textLine(line);
  if(lineStr.isNull() || lineStr.isEmpty()) return; // nothing to do


  if(checkForNewInstanceArgHint(lineStr,col,line)){
    return;
  }

  if(checkForMethodArgHint(lineStr,col,line)){
    return;
  }

  if(checkForGlobalFunctionArgHint(lineStr,col,line)){
    return;
  }

  QString restLine = lineStr.mid(col);
  if(restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull()){
    //cerr << "no codecompletion because no empty character after cursor:" << restLine << ":" << endl;
    return;
  }
  if(checkForVariable(lineStr,col,line)){
    return;
  }

  // $test = new XXX
  if(checkForNewInstance(lineStr,col,line)){
    return;
  }

  if(checkForGlobalFunction(lineStr,col)) {
    return;
  }



}

bool PHPCodeCompletion::checkForMethodArgHint(QString lineStr,int col,int line){
  //cerr << "enter checkForMethodArgHint" << endl;
  if(m_argWidgetShow){
    return false; //nothing to do
  }
  QString methodStart = lineStr.left(col);
  int leftBracket = methodStart.findRev("(");
  methodStart = methodStart.left(leftBracket);
  int varStart = methodStart.findRev("$");
  if(varStart ==-1){
    //cerr << "checkForMethodArgHint: no '$' (variable start) found" << endl;
    return false;
  }
  QString variableLine = methodStart.mid(varStart+1);
  if(variableLine.isNull()){ return false;}
  //  cerr << "VarLine:" << variableLine << endl;
  QString className = "";
  QStringList vars = QStringList::split("->",variableLine);
  QString methodName = vars.last();
  //cerr << "methodname:" << methodName << endl;
  vars.remove(vars.fromLast()); // remove the methodname
  for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
    className = this->getClassName("$" + (*it),className);
  }
  //  cerr << "Classname:" << className << endl;

  ParsedClass* pClass =  m_classStore->getClassByName(className);
  if(pClass !=0){
    QValueList<ParsedMethod*> methodList = pClass->getSortedMethodList();
    QValueList<ParsedMethod*>::ConstIterator methodIt;
    
    for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
      if ((*methodIt)->name() == methodName){
	ParsedArgument* pArg = (*methodIt)->arguments.first();
	m_argWidgetShow = true;
	QValueList <QString> functionList;
	if(pArg){
	  functionList.append(methodName+"("+ pArg->type()+")");
	}
	m_codeInterface->showArgHint ( functionList, "()", "," );
	return true;
      }
    }
  }

  return false;
}
bool PHPCodeCompletion::checkForVariable(QString lineStr,int col,int line){
  //cerr  << "enter checkForVariable()" << endl;
  QString methodStart = lineStr.left(col);
  if(methodStart.right(2) != "->"){
    cerr  << "checkForVariable: no '->' found" << endl;
    return false;
  }
  int varStart = methodStart.findRev("$");
  if(varStart ==-1){
    //cerr << "checkForVariable: no '$' (variable start) found" << endl;
    return false;
  }
  QString variableLine = methodStart.mid(varStart+1);
  //cerr << "VarLine:" << variableLine << endl;
  QString className ="";
  QStringList vars = QStringList::split("->",variableLine);
  for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
    className = this->getClassName("$" + (*it),className);
  }
  //cerr << "Classname:" << className << endl;

  QValueList<KTextEditor::CompletionEntry> list = this->getClassMethodsAndVariables(className);
  if(list.count()>0){
    m_completionBoxShow=true;
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
    QValueList<ParsedAttribute*> attrList = pClass->getSortedAttributeList();
    QValueList<ParsedAttribute*>::ConstIterator attrIt;

    for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
      if ((*attrIt)->name() == varName){
	return (*attrIt)->type();
      }
    }
  }
  return "";
}

QString PHPCodeCompletion::searchClassNameForVariable(QString varName){
  KRegExp createVarRe(QString("\\$" + varName.mid(1) + "[ \t]*=[ \t]*new[ \t]+([0-9A-Za-z_]+)").latin1());
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->textLine(i);
    if(createVarRe.match(lineStr.latin1())) { // ok found
      //      cerr << endl << "match in searchClassNameForVariable:";
      return createVarRe.group(1);
    }
  }
  return "";
}

QString PHPCodeCompletion::searchCurrentClassName(){
  KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->textLine(i);
    if(classre.match(lineStr.latin1())) { // ok found
      return classre.group(1);
    }
  }
}

bool PHPCodeCompletion::checkForGlobalFunctionArgHint(QString lineStr,int col,int line){
  //  cerr  << "enter checkForGlobalFunctionArgHint" << endl;
  if(m_argWidgetShow){
    return false; //nothing to do
  }

  QString methodStart = lineStr.left(col);
  int leftBracket = methodStart.findRev("(");
  int rightBracket = methodStart.findRev(")");
  cerr << "col: " << col << endl;
  cerr << "leftBracket: " << leftBracket << endl;
  cerr << "rightBracket: " << rightBracket << endl;
  cerr << "methodStart: " << methodStart.latin1() << endl;
  if(leftBracket == -1) return false; // ok not found
  if(rightBracket>leftBracket) return false; // we are out of (..)
  methodStart = methodStart.left(leftBracket+1);
  //  cerr << methodStart << endl;
  KRegExp functionre("([A-Za-z_]+)[ \t]*\\(");
  if(functionre.match(methodStart.latin1())){ // check for global functions
    QString name = functionre.group(1);
    int startMethod = lineStr.findRev(name,col);
    QString startString = lineStr.mid(0,startMethod);
    if(startString.right(2) != "->"){
      QValueList <QString> functionList;
      //cerr << "PHPCodeCompletion::checkForArgHint() found global function" << endl ;
      QValueList<FunctionCompletionEntry>::Iterator it;
      for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
	if((*it).text == name){
	  functionList.append((*it).prototype);
	}
      }
      QValueList<ParsedMethod*> methodList = m_classStore->globalContainer.getSortedMethodList();
      QValueList<ParsedMethod*>::ConstIterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
	if((*methodIt)->name() == name){
	  ParsedArgument* pArg = (*methodIt)->arguments.first();
	  functionList.append(name+"("+ pArg->type()+")");
	}
      }
      if(functionList.count() >0){
	m_argWidgetShow = true;
	if (m_codeInterface)
	  m_codeInterface->showArgHint ( functionList, "()", "," );
	return true;
      }
    }
  }
  return false;
}
bool PHPCodeCompletion::checkForGlobalFunction(QString lineStr,int col){
  //  cerr  << "enter checkForGlobalFunction" << endl;
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
       startChar == "-" || startChar == "=" ||startChar == "/" || startChar == "*" || startChar == ";" ||
       startChar == ")" || startChar == "(" || startChar == "}" || startChar == "{"){
      methodStart = startStr.right(2);
    }
  }
  if(methodStart != ""){
    // ok it is an global function
    //    cerr << "Methodstart" << methodStart;
    QValueList<KTextEditor::CompletionEntry> list;
    QValueList<FunctionCompletionEntry>::Iterator it;
    for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
      if((*it).text.startsWith(methodStart)){
	KTextEditor::CompletionEntry e;
	e = (*it);
	list.append(e);
      }
    }

    QValueList<ParsedMethod*> methodList = m_classStore->globalContainer.getSortedMethodList();
    QValueList<ParsedMethod*>::ConstIterator methodIt;
    for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
      if ((*methodIt)->name().startsWith(methodStart)){
	KTextEditor::CompletionEntry e;
	e.text = (*methodIt)->name();
	e.postfix ="()";
	list.append(e);
      }
    }

    if(list.count() >0){
      m_completionBoxShow=true;
      m_codeInterface->showCompletionBox(list,2);
      return true;
    }
  }
  return false;
}

bool PHPCodeCompletion::checkForNewInstanceArgHint(QString lineStr,int col,int line){
  //  cerr  << "enter checkForNewInstanceArgHint" << endl;
  if(m_argWidgetShow){
    return false; //nothing to do
  }

  QString start = lineStr.left(col);
  int leftBracket = start.findRev("(");
  int rightBracket = start.findRev(")");
  int equal = start.findRev("=");
  if(equal == -1) return false; // ok not found
  if(leftBracket == -1) return false; // ok not found
  if(rightBracket>leftBracket) return false; // we are out of (..)
  start = start.mid(equal,leftBracket-equal+1);
  //  cerr << "NEW: " << start << endl;
  KRegExp newre("=[ \t]*new[ \t]+([A-Za-z_]+)[ \t]*\\(");
  if(newre.match(start.latin1())){
    ParsedClass* pClass=0;
    pClass =  m_classStore->getClassByName(newre.group(1));
    if(pClass !=0){ // exists this class?
      QValueList<ParsedMethod*> methodList = pClass->getSortedMethodList();
      QValueList<ParsedMethod*>::ConstIterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
        if((*methodIt)->name() == newre.group(1)){
	  ParsedArgument* pArg = (*methodIt)->arguments.first();
	  m_argWidgetShow = true;
	  QValueList <QString> functionList;
	  if(pArg){
	    functionList.append((*methodIt)->name()+"("+ pArg->type()+")");
	  }
	  m_codeInterface->showArgHint ( functionList, "()", "," );
	  return true;
	}
      }
    }
  }
  return false;
}
bool PHPCodeCompletion::checkForNewInstance(QString lineStr,int col,int line){
  //  cerr  << "enter checkForNewInstance" << endl;
  QString start = lineStr.left(col);
  KRegExp newre("=[ \t]*new[ \t]+([A-Za-z_]+)");
  if(newre.match(start.latin1())){
    QString classStart = newre.group(1);
    if(start.right(2) == classStart){
      QValueList<KTextEditor::CompletionEntry> list;

      QValueList<ParsedClass*> classList = m_classStore->globalContainer.getSortedClassList();
      QValueList<ParsedClass*>::ConstIterator classIt;
      for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
	if((*classIt)->name().startsWith(classStart)){
	  KTextEditor::CompletionEntry e;
	  e.text = (*classIt)->name();
	  list.append(e);
	}
      }
      if(classStart == "ob") {
	KTextEditor::CompletionEntry e;
	e.text = "object";
	list.append(e);
      }
      if(classStart == "ar") {
	KTextEditor::CompletionEntry e;
	e.text = "array";
	list.append(e);
      }
      if(list.count() >0){
	m_completionBoxShow=true;
	m_codeInterface->showCompletionBox(list,2);
	return true;
      }
    }
  }
  return false;
}

QValueList<KTextEditor::CompletionEntry> PHPCodeCompletion::getClassMethodsAndVariables(QString className){
  QList<ParsedParent> parents;
  QValueList<KTextEditor::CompletionEntry> list;
  ParsedClass* pClass=0;
  do {
    pClass =  m_classStore->getClassByName(className);
    if(pClass !=0){
      QValueList<ParsedMethod*> methodList = pClass->getSortedMethodList();
      QValueList<ParsedMethod*>::ConstIterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
	KTextEditor::CompletionEntry e;
	e.text = (*methodIt)->name();
	//	ParsedArgument* pArg = pMethod->arguments.first();
	//	if(pArg->type() == ""){
	e.postfix ="()";
	//	}else{
	//      e.postfix ="(...)";
	//	}
	list.append(e);
      }
      QValueList<ParsedAttribute*> attrList = pClass->getSortedAttributeList();
      QValueList<ParsedAttribute*>::ConstIterator attrIt;
      for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
	KTextEditor::CompletionEntry e;
	QString name = (*attrIt)->name();
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
