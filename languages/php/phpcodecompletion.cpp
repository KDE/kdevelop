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
#include "phpsupportpart.h"
#include "phpconfigdata.h"

#include <kdevcore.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <iostream>

using namespace std;

PHPCodeCompletion::PHPCodeCompletion(PHPConfigData *config,KDevCore* core,CodeModel* model){
  m_config = config;
  m_core = core;
  m_model = model;
  m_argWidgetShow = false;
  m_completionBoxShow=false;

  readGlobalPHPFunctionsFile();
}

PHPCodeCompletion::~PHPCodeCompletion(){
}

void PHPCodeCompletion::readGlobalPHPFunctionsFile(){
  KStandardDirs *dirs = PHPSupportFactory::instance()->dirs();
  QString phpFuncFile = dirs->findResource("data","kdevphpsupport/phpfunctions");
  QRegExp lineReg(":([0-9A-Za-z_]+) ([0-9A-Za-z_]+)(\\(.*\\))");
  FunctionCompletionEntry e;
  QFile f(phpFuncFile);
  if ( f.open(IO_ReadOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream
      QString s;
      while ( !t.eof() ) {        // until end of file...
	  s = t.readLine();       // line of text excluding '\n'
	  if(lineReg.search(s.local8Bit()) != -1){
	      e.prefix = lineReg.cap(1);
	      e.text = lineReg.cap(2);
	      //	      if(QString(lineReg.cap(3)) == "void"){
		e.postfix ="()";
		//	      }
		//	      else{
		//	      e.postfix ="(...)";
		//	      }
	      e.prototype = QString(lineReg.cap(1)) + " " + QString(lineReg.cap(2)) +
		  "(" + QString(lineReg.cap(3)) + ")";
	      m_globalFunctions.append(e);
	  }

      }
      f.close();
  }

}
void PHPCodeCompletion::argHintHided(){
  kdDebug(9018) << "PHPCodeCompletion::argHintHided"  << endl;
  m_argWidgetShow = false;
}
void PHPCodeCompletion::completionBoxHided(){
  kdDebug(9018) << "PHPCodeCompletion::completionBoxHided()" << endl;
  m_completionBoxShow=false;
}

void PHPCodeCompletion::setActiveEditorPart(KParts::Part *part)
{
  if (!part || !part->widget())
    return;

  kdDebug(9018) << "PHPCodeCompletion::setActiveEditorPart"  << endl;

  if(!(m_config->getCodeCompletion() || m_config->getCodeHinting())){
    return; // no help
  }

  m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
  if (!m_editInterface)
  {
    kdDebug(9018) << "editor doesn't support the EditDocumentIface" << endl;
    return;
  }

  m_cursorInterface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  if (!m_cursorInterface)
  {
    kdDebug(9018) << "editor does not support the ViewCursorInterface" << endl;
    return;
  }

  m_codeInterface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
  if (!m_codeInterface) { // no CodeCompletionDocument available
    kdDebug(9018) << "editor doesn't support the CodeCompletionDocumentIface" << endl;
    return;
  }

  m_selectionInterface = dynamic_cast<KTextEditor::SelectionInterface*>(part);
  if(!m_selectionInterface) {
	kdDebug(9018) << "editor doesn't support the SelectionInterface" << endl;
	return;
  }


  disconnect(part->widget(), 0, this, 0 ); // to make sure that it is't connected twice
//  connect(part->widget(), SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
  connect( part, SIGNAL(textChanged()), this, SLOT(cursorPositionChanged()) );
  connect(part->widget(), SIGNAL(argHintHidden()), this, SLOT(argHintHided()));
  connect(part->widget(), SIGNAL(completionAborted()), this, SLOT(completionBoxHided()));
  connect(part->widget(), SIGNAL(completionDone()), this, SLOT(completionBoxHided()));

}

void PHPCodeCompletion::cursorPositionChanged(){
  uint line, col;
  m_cursorInterface->cursorPositionReal(&line, &col);
  kdDebug(9018) << "PHPCodeCompletion::cursorPositionChanged:" << line << ":" << col  << endl;

  m_currentLine = line;
  QString lineStr = m_editInterface->textLine(line);
  if(lineStr.isNull() || lineStr.isEmpty()) return; // nothing to do
  //kdDebug(9018) << "ZEILE:" << lineStr <<":" << endl;
  //  kdDebug(9018) << "Length:" << lineStr.length() <<":" << endl;
  //  lineStr.replace(QRegExp("\t"),"_");
  //  kdDebug(9018) << "ZEILEohneTAB:" << lineStr <<":" << endl;

  if(m_selectionInterface->hasSelection()){
	kdDebug(9018) << "No CodeCompletion/ArgHinting at the moment, because text is selected" << endl;
	return;
  }

  if(m_config->getCodeHinting()){
    if(checkForNewInstanceArgHint(lineStr,col,line)){
      return;
    }

    if(checkForMethodArgHint(lineStr,col,line)){
      return;
    }

    if(checkForGlobalFunctionArgHint(lineStr,col,line)){
      return;
    }
  }

  if(m_config->getCodeCompletion()){
    QString restLine = lineStr.mid(col);
    if(restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull()){
      kdDebug(9018) << "no codecompletion because no empty character after cursor:" << restLine << ":" << endl;
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


}

bool PHPCodeCompletion::checkForMethodArgHint(QString lineStr,int col,int /*line*/){
  kdDebug(9018) << "enter checkForMethodArgHint" << endl;
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

  if( m_model->globalNamespace()->hasClass(className) ){
    ClassDom pClass =  m_model->globalNamespace()->classByName(className)[ 0 ];
    FunctionList methodList = pClass->functionList();
    FunctionList::Iterator methodIt;

    for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
      if ((*methodIt)->name() == methodName){
	ArgumentDom pArg = (*methodIt)->argumentList().first();
	m_argWidgetShow = true;
	QValueList <QString> functionList;
	if(pArg){
	  functionList.append(methodName + "(" + pArg->type() +")");
	}
	m_codeInterface->showArgHint ( functionList, "()", "," );
	return true;
      }
    }
  }

  return false;
}
bool PHPCodeCompletion::checkForVariable(QString lineStr,int col,int /*line*/){
  kdDebug(9018)  << "enter checkForVariable()" << endl;
  QString methodStart = lineStr.left(col);
  if(methodStart.right(2) != "->"){
    kdDebug(9018)  << "checkForVariable: no '->' found" << endl;
    return false;
  }
  int varStart = methodStart.findRev("$");
  if(varStart ==-1){
    kdDebug(9018) << "checkForVariable: no '$' (variable start) found" << endl;
    return false;
  }
  QString variableLine = methodStart.mid(varStart+1);
  kdDebug(9018) << "VarLine:" << variableLine << ":" << endl;
  QString className ="";
  QStringList vars = QStringList::split("->",variableLine);
  for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
    className = this->getClassName("$" + (*it),className);
  }
  kdDebug(9018) << "Classname:" << className << endl;

  QValueList<KTextEditor::CompletionEntry> list = this->getClassMethodsAndVariables(className);
  if(list.count()>0){
    m_completionBoxShow=true;
    m_codeInterface->showCompletionBox(list);
    return true;
  }
  return false;
}

QString PHPCodeCompletion::getClassName(QString varName,QString maybeInstanceOf){
  kdDebug(9018) << "enter PHPCodeCompletion::getClassName:" << varName << ":" << maybeInstanceOf << ":" << endl;
  if(varName == "$this"){
    return this->searchCurrentClassName();
  }
  if(maybeInstanceOf.isEmpty()){
    // ok, we need to search it
    return this->searchClassNameForVariable(varName);
  }
  if(m_model->globalNamespace()->hasClass(maybeInstanceOf) !=0){
    ClassDom pClass = m_model->globalNamespace()->classByName(maybeInstanceOf)[ 0 ];
    VariableList attrList = pClass->variableList();
    VariableList::Iterator attrIt;

    for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
      if ((*attrIt)->name() == varName){
	return (*attrIt)->type();
      }
    }
  }
  return "";
}

QString PHPCodeCompletion::searchClassNameForVariable(QString varName){
  kdDebug(9018) << "enter PHPCodeCompletion::searchClassNameForVariable:" << varName << ":" << endl;
  QRegExp createVarRe(QString("\\$" + varName.mid(1) + "[ \t]*=[& \t]*new[ \t]+([0-9A-Za-z_]+)").local8Bit());
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->textLine(i);
    if(!lineStr.isNull()){
      if(createVarRe.search(lineStr.local8Bit()) != -1) { // ok found
	//      cerr << endl << "match in searchClassNameForVariable:";
	return createVarRe.cap(1);
      }
    }
  }
  return QString::null;
}

QString PHPCodeCompletion::searchCurrentClassName(){
  kdDebug(9018) << "enter PHPCodeCompletion::searchCurrentClassName:" << endl;
  QRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  for(int i=m_currentLine;i>=0;i--){
    QString lineStr = m_editInterface->textLine(i);
    if(!lineStr.isNull()){
      if(classre.search(lineStr.local8Bit()) != -1) { // ok found
          return classre.cap(1);
      }
    }
  }
  return QString::null;
}

bool PHPCodeCompletion::checkForGlobalFunctionArgHint(QString lineStr,int col,int /*line*/){
  kdDebug(9018) << "enter checkForGlobalFunctionArgHint" << endl;
  if(m_argWidgetShow){
    return false; //nothing to do
  }

  QString methodStart = lineStr.left(col);
  int leftBracket = methodStart.findRev("(");
  int rightBracket = methodStart.findRev(")");
  kdDebug(9018)   << "col: " << col << endl;
  kdDebug(9018)   << "leftBracket: " << leftBracket << endl;
  kdDebug(9018)   << "rightBracket: " << rightBracket << endl;
  kdDebug(9018)   << "methodStart: " << methodStart.latin1() << endl;
  if(leftBracket == -1) return false; // ok not found
  if(rightBracket>leftBracket) return false; // we are out of (..)
  methodStart = methodStart.left(leftBracket+1);
  //  cerr << methodStart << endl;
  QRegExp functionre("([A-Za-z_]+)[ \t]*\\(");
  if(functionre.search(methodStart.local8Bit()) != -1){ // check for global functions
    QString name = functionre.cap(1);
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
      FunctionList methodList = m_model->globalNamespace()->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
	if((*methodIt)->name() == name){
	  ArgumentDom pArg = (*methodIt)->argumentList().first();
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
  kdDebug(9018)   << "enter checkForGlobalFunction(" + lineStr + "," << col << endl;
  QString methodStart ="";
  if(lineStr.length()==2){
    return doGlobalMethodCompletion(lineStr);
  }
  if(col==2){
    QString startStr =lineStr.mid(col-2,2);
    return doGlobalMethodCompletion(startStr);
  }

  // normal case
  QString startStr =lineStr.mid(col-3,3);
  if(startStr.isNull()){
    kdDebug(9018)   << "not enough letters" << endl;
    return false; // not enough letters
  }
  //dDebug(9018)  << "StartStr:" << startStr  << ":" << endl;
  QString extraChar = " \t+-=/*;)(}{";
  if(extraChar.find( startStr[0] ) != -1){
    methodStart = startStr.right(2);
  }

  //kdDebug(9018)  << "Methodstart:" << methodStart  << ":" << endl;
  if(!methodStart.isEmpty()){
    return doGlobalMethodCompletion(methodStart);
  }
  return false;
}

bool PHPCodeCompletion::doGlobalMethodCompletion(QString methodStart){
  //kdDebug(9018)  << "doGlobalMethodCompletion:" << methodStart  << ":" << endl;
  QValueList<KTextEditor::CompletionEntry> list;
  QValueList<FunctionCompletionEntry>::Iterator it;
  for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ){
    if((*it).text.startsWith(methodStart, FALSE)){
      KTextEditor::CompletionEntry e;
      e = (*it);
      list.append(e);
    }
  }

  FunctionList methodList = m_model->globalNamespace()->functionList();
  FunctionList::Iterator methodIt;
  for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
    if ((*methodIt)->name().startsWith(methodStart, FALSE)){
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
  return false;
}



bool PHPCodeCompletion::checkForNewInstanceArgHint(QString lineStr,int col,int /*line*/){
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
  QRegExp newre("=[& \t]*new[ \t]+([A-Za-z_]+)[ \t]*\\(");
  if(newre.search(start.local8Bit()) != -1){
    if( m_model->globalNamespace()->hasClass(newre.cap(1)) ){ // exists this class?
      ClassDom pClass = m_model->globalNamespace()->classByName(newre.cap(1))[ 0 ];
      FunctionList methodList = pClass->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
        if((*methodIt)->name() == newre.cap(1)){
	  ArgumentDom pArg = (*methodIt)->argumentList().first();
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
bool PHPCodeCompletion::checkForNewInstance(QString lineStr,int col,int /*line*/){
  //  cerr  << "enter checkForNewInstance" << endl;
  QString start = lineStr.left(col);
  QRegExp newre("=[ \t]*&[ \t]*new[ \t]+([A-Za-z_]+)");
  QStringList added;
  
  if(newre.search(start.local8Bit()) != -1){
    QString classStart = newre.cap(1);
    if(start.right(2) == classStart){
      QValueList<KTextEditor::CompletionEntry> list;

      ClassList classList = m_model->globalNamespace()->classList();
      ClassList::Iterator classIt;
      for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
	if((*classIt)->name().startsWith(classStart, FALSE)){
	  KTextEditor::CompletionEntry e;
     
   QString name = (*classIt)->name();
   QStringList::Iterator it = added.find(name);
   if (it == added.end())  {
	  e.text = name;
	  list.append(e);
      added.append(name);
   }
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
  QValueList<KTextEditor::CompletionEntry> list;
  ClassDom pClass;
      kdDebug(9018)  << "getClassMethodsAndVariables:" << className << ":" << endl;
      
  do {
    if(m_model->globalNamespace()->hasClass(className) ){
    kdDebug(9018)  << "hasClass:" << className << ":" << endl;
      pClass = m_model->globalNamespace()->classByName(className)[ 0 ];
      FunctionList methodList = pClass->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
	KTextEditor::CompletionEntry e;
   FunctionDom pMethod = (*methodIt);  
	e.text = pMethod->name();
   
   kdDebug(9018)  << "methode:" << e.text << ":" << endl;
	
   //	ParsedArgument* pArg = pMethod->arguments.first();
	//	if(pArg->type() == ""){
   ArgumentDom arg = pMethod->argumentList().first();

   e.postfix = "(" + arg->type() + ")";
	
	list.append(e);
      }
      VariableList attrList = pClass->variableList();
      VariableList::Iterator attrIt;
      for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
	KTextEditor::CompletionEntry e;
	QString name = (*attrIt)->name();
	e.text = name;
	e.postfix ="";
	list.append(e);
      }


      if(pClass->baseClassList().count() !=0){
	className = pClass->baseClassList().first();
      }
      else{
	className ="";
      }
    } else {
        pClass = 0;
    }
  } while (pClass != 0);
  return list;
}

void PHPCodeCompletion::setStatusBar() {
//   m_phpSupport->mainWindow()->statusBar()->message( i18n("Type of %1 is %2").arg(expr).arg(type.join("::")), 1000 );
}   

#include "phpcodecompletion.moc"
