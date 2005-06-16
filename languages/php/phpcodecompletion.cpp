/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>
   Copyright (C) 2001 by smeier@kdevelop.org
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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

#include "phpfile.h"

using namespace std;

PHPCodeCompletion::PHPCodeCompletion(PHPSupportPart *phpSupport, PHPConfigData *config) {

   m_phpSupport = phpSupport;
   m_config = config;
   m_model = phpSupport->codeModel();
   m_argWidgetShow = false;
   m_completionBoxShow = false;

   readGlobalPHPFunctionsFile();
}

PHPCodeCompletion::~PHPCodeCompletion(){
}

void PHPCodeCompletion::readGlobalPHPFunctionsFile(){
   KStandardDirs *dirs = PHPSupportFactory::instance()->dirs();
   QString phpFuncFile = dirs->findResource("data","kdevphpsupport/phpfunctions");
   QRegExp lineReg(":([0-9A-Za-z_]+) ([0-9A-Za-z_]+)\\((.*)\\)");
   FunctionCompletionEntry e;
   
   QFile f(phpFuncFile);
   if ( f.open(IO_ReadOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream
      QString s;
      while ( !t.eof() ) {        // until end of file...
         s = t.readLine();       // line of text excluding '\n'
	      if (lineReg.search(s.local8Bit()) != -1) {
   	      e.prefix = lineReg.cap(1);
	         e.text = lineReg.cap(2);
            e.postfix = "(" + QString(lineReg.cap(3)) + ")";
            e.prototype = QString(lineReg.cap(1)) + " " + QString(lineReg.cap(2)) + "(" + QString(lineReg.cap(3)) + ")";
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
   m_completionBoxShow = false;
}

void PHPCodeCompletion::setActiveEditorPart(KParts::Part *part)
{
   if (!part || !part->widget())
      return;

   kdDebug(9018) << "PHPCodeCompletion::setActiveEditorPart"  << endl;

   if (!(m_config->getCodeCompletion() || m_config->getCodeHinting()))
      return; // no help

   m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
   if (!m_editInterface) {
      kdDebug(9018) << "editor doesn't support the EditDocumentIface" << endl;
      return;
   }

   m_cursorInterface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
   if (!m_cursorInterface) {
      kdDebug(9018) << "editor does not support the ViewCursorInterface" << endl;
      return;
   }

   m_codeInterface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
   if (!m_codeInterface) { // no CodeCompletionDocument available
      kdDebug(9018) << "editor doesn't support the CodeCompletionDocumentIface" << endl;
      return;
   }

   m_selectionInterface = dynamic_cast<KTextEditor::SelectionInterface*>(part);
   if (!m_selectionInterface) {
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
  
   kdDebug(9018) << "cursorPositionChanged:" << line << ":" << col  << endl;

   m_currentLine = line;
   QString lineStr = m_editInterface->textLine(line);
   if (lineStr.isNull() || lineStr.isEmpty()) {
      return;
   }
     
   if (m_selectionInterface->hasSelection()) {
      kdDebug(9018) << "No CodeCompletion/ArgHinting at the moment, because text is selected" << endl;
      return;
   }
  
   if (m_config->getCodeHinting()) {
      int pos1 = lineStr.findRev("(", col - 1);
      int pos2 = lineStr.findRev(QRegExp("[ \\t=;\\$\\.\\(\\)]"), pos1 - 1);
      int pos3 = lineStr.findRev(")", col);
      
      if (pos1 > pos2 && pos1 != -1 && pos3 < pos1) {
         QString line = lineStr.mid(pos2 + 1, pos1 - pos2 - 1).stripWhiteSpace();
         checkForArgHint(line, col);
      }
      
      /*
      if (checkForArgHint(lineStr, col)) {
         return;
      }
      */
   }

   if (m_config->getCodeCompletion()) {
      if (m_completionBoxShow == true) {
         return;
      }
      
      int pos = lineStr.findRev(QRegExp("[ \\t=;\\$\\.\\(\\)]"), col - 1);
      QString line = lineStr.mid(pos + 1, col - pos).stripWhiteSpace();

      if (checkForVariable(line, col))
         return;

      if (checkForStaticFunction(line, col))
         return;

      if(checkForGlobalFunction(line, col))
         return;


      pos = lineStr.stripWhiteSpace().findRev(QRegExp("[ \\t=;\\$\\.\\(\\)]"), col - 1);
      line = lineStr.mid(pos + 1, col - pos);

      if (checkForNew(line, col))
         return;

      if (checkForExtends(line, col))
         return;
  }
}

bool PHPCodeCompletion::showCompletionBox(QValueList<KTextEditor::CompletionEntry> list, unsigned long max) {
   if (list.count() > 0) {
      if (list.count() == 1) {
         KTextEditor::CompletionEntry e = list.first();
         if (e.text.length() == max)
            return false;
      }
      m_completionBoxShow = true;
      m_codeInterface->showCompletionBox(list, max, FALSE);
      return true;
   }
   return false;
}

bool PHPCodeCompletion::checkForStaticFunction(QString line, int col) {
   kdDebug(9018)   << "checkForStaticFunction" << endl;
   QValueList<KTextEditor::CompletionEntry> list;

   if (line.find("::") == -1)
      return false;

   QRegExp Class("([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)::([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*|)");
   Class.setCaseSensitive(FALSE);

   if (Class.search(line) != -1) {
      QString classname = Class.cap(1);
      QString function = Class.cap(2);
      
      ClassList classList = getClassByName(classname);
      
      ClassList::Iterator classIt;
      for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
         ClassDom nClass = *classIt;
         FunctionList funcList = nClass->functionList();
         FunctionList::Iterator funcIt;
         
         for (funcIt = funcList.begin(); funcIt != funcList.end(); ++funcIt) {
            FunctionDom nFunc = *funcIt;
            if ((function.isEmpty() || nFunc->name().startsWith(function, FALSE)) && nFunc->isStatic()) {
               KTextEditor::CompletionEntry e;
               e.prefix = nClass->name() + " ::";
               e.text = nFunc->name();
               ArgumentDom pArg = (*funcIt)->argumentList().first();
               if (pArg) 
                  e.postfix = "(" + pArg->type() +")";
               else
                  e.postfix = "()";
               list.append(e);
            }
         }

         if (nClass->baseClassList().count() != 0) {
            QStringList base = nClass->baseClassList();
            QStringList::Iterator nameIt;
            for (nameIt = base.begin(); nameIt != base.end(); ++nameIt) {
               ClassList baseList = getClassByName(*nameIt);
               ClassList::Iterator baseIt;
               for (baseIt = baseList.begin(); baseIt != baseList.end(); ++baseIt)
                  classList.append(*baseIt);
            }
         }
      }
      return showCompletionBox(list, Class.cap(2).length());
   }
   return false;
}

bool PHPCodeCompletion::checkForNew(QString line, int col){
   kdDebug(9018)   << "checkForNew" << endl;
   QValueList<KTextEditor::CompletionEntry> list;

   if (line.find("new ", 0, FALSE) == -1)
      return false;

   QRegExp New("[& \t]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*|)");
   New.setCaseSensitive(FALSE);

   if (New.search(line) != -1) {
      list = getClasses( New.cap(1) );
      
      if (New.cap(1).lower() == "ob") {
         KTextEditor::CompletionEntry e;
         e.text = "object";
         list.append(e);
      }

      if (New.cap(1).lower() == "ar") {
         KTextEditor::CompletionEntry e;
         e.text = "array";
         list.append(e);
      }
      return showCompletionBox(list, New.cap(1).length());
   }

   return false;
}

bool PHPCodeCompletion::checkForExtends(QString line, int col){
   kdDebug(9018)   << "checkForExtends" << endl;
   QValueList<KTextEditor::CompletionEntry> list;

   if (line.find("extends", 0, FALSE) == -1)
      return false;
   
   QRegExp extends("[ \t]*extends[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*|)");
   extends.setCaseSensitive(FALSE);

   if (extends.search(line) != -1) {
      list = getClasses(extends.cap(1));
      return showCompletionBox(list, extends.cap(1).length());
   }
  
   return false;
}

bool PHPCodeCompletion::checkForVariable(QString line, int col){
   kdDebug(9018)  << "checkForVariable" << endl;
   QValueList<KTextEditor::CompletionEntry> list;
   QString args;

   if (line.find("->") == -1) {
         return false;
   }

   if (line.left(2) != "->") {
      int pos = line.findRev("->");
      args = line.mid(pos + 2, line.length() - pos);
      line = line.mid(0, pos);
   }
         
   QStringList vars = QStringList::split("->", line);
   QString classname;

   for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
       classname = getClassName(*it, classname);
   }

   if (classname.isEmpty()) {
      return false;
   }

   this->setStatusBar(line, classname);
   
   list = this->getFunctionsAndVars(classname, args);
   return showCompletionBox(list, args.length());
}

bool PHPCodeCompletion::checkForGlobalFunction(QString line, int col) {
   kdDebug(9018)   << "checkForGlobalFunction(" + line + "," << col << endl;
   QValueList<KTextEditor::CompletionEntry> list;

   if (line.length() < 3)
      return false;

   list = this->getFunctionsAndVars("", line);
   return showCompletionBox(list, line.length());
}

QValueList<KTextEditor::CompletionEntry> PHPCodeCompletion::getClasses(QString name) {
   QValueList<KTextEditor::CompletionEntry> list;
   QStringList added;

   ClassList classList = m_model->globalNamespace()->classList();
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom nClass = *classIt;
      if (name == NULL || name.isEmpty() || nClass->name().startsWith(name, FALSE)) {
         KTextEditor::CompletionEntry e;
     
         QStringList::Iterator it = added.find(nClass->name());
         if (it == added.end()) {
            e.text = nClass->name();
            list.append(e);
            added.append(nClass->name());
         }
      }
   }
   return list;
}

QValueList<KTextEditor::CompletionEntry> PHPCodeCompletion::getFunctionsAndVars(QString classname, QString function) {
   kdDebug(9018)  << "getFunctionsAndVars " << classname << endl;
   QValueList<KTextEditor::CompletionEntry> list;

   if (classname.isEmpty()) {
      QValueList<FunctionCompletionEntry>::Iterator it;
      for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ) {
         if((*it).text.startsWith(function, FALSE)){
            KTextEditor::CompletionEntry e;
            e = (*it);
            list.append(e);
         }
      }

      FunctionList methodList = m_model->globalNamespace()->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
         if ((*methodIt)->name().startsWith(function, FALSE)){
            KTextEditor::CompletionEntry e;
            e.text = (*methodIt)->name();
            ArgumentDom pArg = (*methodIt)->argumentList().first();
            if (pArg)
               e.postfix = "(" + pArg->type() +")";
            else
               e.postfix = "()";
            list.append(e);
         }
      }
      return list;
   }

   ClassList classList = getClassByName(classname);
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom nClass = *classIt;

      FunctionList methodList = nClass->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
         FunctionDom pMethod = *methodIt;
         if (function.isEmpty() || pMethod->name().startsWith(function, FALSE)) {
            KTextEditor::CompletionEntry e;
            ArgumentDom arg = pMethod->argumentList().first();
         
            e.prefix = nClass->name() + " ::";
            e.text = pMethod->name();
            e.postfix = "(" + arg->type() + ")";
            list.append(e);
         }
      }
      VariableList attrList = nClass->variableList();
      VariableList::Iterator attrIt;
      for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
         VariableDom pVar = *attrIt;
         if (function.isEmpty() || pVar->name().startsWith(function, FALSE)) {
            KTextEditor::CompletionEntry e;
            e.prefix = nClass->name() + " ::";
            e.text = pVar->name();
            e.postfix = "";
            list.append(e);
         }  
      }

      if (nClass->baseClassList().count() != 0) {
         QStringList base = nClass->baseClassList();
         QStringList::Iterator nameIt;
         for (nameIt = base.begin(); nameIt != base.end(); ++nameIt) {
            ClassList baseList = getClassByName(*nameIt);
            ClassList::Iterator baseIt;
            for (baseIt = baseList.begin(); baseIt != baseList.end(); ++baseIt)
               classList.append(*baseIt);
         }
      }
   }
   return list;
}

QStringList PHPCodeCompletion::getArguments(QString classname, QString function) {
   kdDebug(9018)  << "getArguments " << function << endl;
   QStringList list;

   if (classname.isEmpty()) {
      QValueList<FunctionCompletionEntry>::Iterator it;
      for( it = m_globalFunctions.begin(); it != m_globalFunctions.end(); ++it ) {
         if((*it).text.lower() == function.lower()){
            KTextEditor::CompletionEntry e = (*it);
            list.append(e.text + e.postfix);
         }
      }

      FunctionList methodList = m_model->globalNamespace()->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
         if ((*methodIt)->name().lower() == function.lower()){
            KTextEditor::CompletionEntry e;
            ArgumentDom pArgs;
            QString args = "()";
            
            ArgumentDom pArg = (*methodIt)->argumentList().first();
            if (pArgs)
               args = "(" + pArg->type() +")";

            list.append((*methodIt)->name() + "(" + args +")");
         }
      }
      return list;
   }
         
   ClassList classList = getClassByName(classname);
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom nClass = *classIt;

      FunctionList methodList = nClass->functionList();
      FunctionList::Iterator methodIt;
      for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
         if ((*methodIt)->name().lower() == function.lower()) {
            ArgumentDom pArg = (*methodIt)->argumentList().first();
            if (pArg)
               list.append(nClass->name() + "::" + function + "(" + pArg->type() +")");
         }
      }

      if (nClass->baseClassList().count() != 0) {
         QStringList base = nClass->baseClassList();
         QStringList::Iterator nameIt;
         for (nameIt = base.begin(); nameIt != base.end(); ++nameIt) {
            ClassList baseList = getClassByName(*nameIt);
            ClassList::Iterator baseIt;
            for (baseIt = baseList.begin(); baseIt != baseList.end(); ++baseIt)
               classList.append(*baseIt);
         }
      }

   }
   return list;
}

QString PHPCodeCompletion::getCurrentClassName() {
   kdDebug(9018) << "getCurrentClassName" << endl;

   Action *action;
   for(int i = m_currentLine; i >= 0; i--){
      QString lineStr = m_editInterface->textLine(i);
      if (!lineStr.isNull()) {
         action = PHPFile::ParseClass(lineStr.local8Bit(), 0);
         if (action != NULL) {
            QString name = action->name();
            delete action;
            return name;
         }
      }
   }
   return QString::null;
}

QString PHPCodeCompletion::getClassName(QString varName, QString classname) {
   kdDebug(9018) << "getClassName " << varName << "::" << classname << endl;

   if (varName.find("$") == 0)
       varName = varName.mid(1);
   
   if (varName.lower() == "this")
       return this->getCurrentClassName();
   
   if (classname.isEmpty()) {
      VariableList attrList = m_model->globalNamespace()->variableList();
      VariableList::Iterator attrIt;

      for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
         if ((*attrIt)->name().lower() == varName.lower())
            return (*attrIt)->type();
      }
   }

   ClassList classList = getClassByName( classname );
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom pClass = *classIt;

      FunctionList funcList = pClass->functionList();
      FunctionList::Iterator funcIt;
      
      for (funcIt = funcList.begin(); funcIt != funcList.end(); ++funcIt) {
         if (QString((*funcIt)->name().lower() + "(") == varName.lower())
            return (*funcIt)->resultType();
      }

      VariableList attrList = pClass->variableList();
      VariableList::Iterator attrIt;

      for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
         if ((*attrIt)->name().lower() == varName.lower())
            return (*attrIt)->type();
      }
   }

   QRegExp createmember("\\" + varName + "[ \t]*=[ \t]*(.*)[ \t]*;");

   for(int i = m_currentLine; i >= 0; i--){
      QString line = m_editInterface->textLine(i);
      if (!line.isNull() && line.find(varName,0 , FALSE) != -1) {
         
         if (createmember.search(line) != -1) {
            QString right = createmember.cap(1).stripWhiteSpace();
            
            QStringList vars = QStringList::split("->", right);

            for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
               int pos = QString(*it).find("(");
               if (pos != -1) {
                  QString funcname = QString(*it).mid(0, pos + 1);
                  classname = getClassName(funcname, classname);
               } else {
                  classname = getClassName(*it, classname);
               }
            }
            return classname;
         }
      }
   }
      
   return "";
}

QValueList<ClassDom> PHPCodeCompletion::getClassByName(QString classname) {
   QValueList<ClassDom> CList;

   ClassList classList = m_model->globalNamespace()->classList();
   
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom nClass = *classIt;
      if (nClass->name().lower() == classname.lower())
         CList.append( nClass );
   }
   return CList;
}

bool PHPCodeCompletion::checkForArgHint(QString line, int col) {
   kdDebug(9018) << "checkForArgHint" <<  endl;
   QValueList<KTextEditor::CompletionEntry> list;
   QStringList argsList;

   if (m_argWidgetShow == true)
      return false;

   if (line.find("::") != -1) {
      QRegExp Static("([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)::([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
      Static.setCaseSensitive(FALSE);

      if (Static.search(line) != -1) {
         QString classname = Static.cap(1);
         QString function = Static.cap(2);

         argsList = getArguments(classname, function);

         if (argsList.count() > 0) {
            m_argWidgetShow = true;
            m_codeInterface->showArgHint ( argsList, "()", "," );
            return true;
         }
      }   
   }

   if (line.findRev("->") != -1) {
      int pos1 = line.findRev("->");

      QString classname;
      QString function = line.mid(pos1 + 2);
      
      line = line.mid(0, pos1);

      QStringList vars = QStringList::split("->", line);

      for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
         classname = getClassName(*it, classname);
      }
         
      argsList = getArguments(classname, function);
      if (argsList.count() > 0) {
         m_argWidgetShow = true;
         m_codeInterface->showArgHint ( argsList, "()", "," );
         return true;
      }
   }
   
   argsList = getArguments("", line);
   if (argsList.count() > 0) {
      m_argWidgetShow = true;
      m_codeInterface->showArgHint ( argsList, "()", "," );
      return true;
   }

   argsList = getArguments(line, line);
   if (argsList.count() > 0) {
      m_argWidgetShow = true;
      m_codeInterface->showArgHint ( argsList, "()", "," );
      return true;
   }

   return false;
}

void PHPCodeCompletion::setStatusBar(QString expr, QString type) {
   m_phpSupport->mainWindow()->statusBar()->message( i18n("Type of %1 is %2").arg(expr).arg(type), 1000 );
}   

#include "phpcodecompletion.moc"
