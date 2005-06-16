/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>

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

#include <kapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include <urlutil.h>
#include <kprocess.h>
#include <kdebug.h>

#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include "phpfile.h"

using namespace std;

PHPFile::PHPFile(PHPSupportPart *phpSupport, const QString& fileName)
{
  m_fileinfo = new QFileInfo(fileName);
  m_phpSupport = phpSupport;
  m_model = m_phpSupport->codeModel();
  modified = true;
  
  nClass = 0;
  nMethod = 0;
  nArgument = 0;
  nVariable = 0;

  /*   
  phpCheckProc = new KShellProcess("/bin/sh");
  connect(phpCheckProc, SIGNAL(receivedStdout (KProcess*, char*, int)), this, SLOT(slotReceivedPHPCheckStdout (KProcess*, char*, int)));
  connect(phpCheckProc, SIGNAL(receivedStderr (KProcess*, char*, int)), this, SLOT(slotReceivedPHPCheckStderr (KProcess*, char*, int)));
  connect(phpCheckProc, SIGNAL(processExited(KProcess*)), this, SLOT(slotPHPCheckExited(KProcess*)));
  */
}

PHPFile::~PHPFile()
{
   if (m_fileinfo)
      delete m_fileinfo;
     
   QValueListConstIterator<Action *> it = m_actions.begin();
   while (it != m_actions.end()) {
      Action *p = *it;
      delete p;
      ++it;
   }
//  delete phpCheckProc;
}

QStringList PHPFile::getContents()
{
   return m_contents;
}

QString PHPFile::fileName() {
   return m_fileinfo->filePath();
}

QValueList<Action *> PHPFile::getActions() {
   return m_actions;
}

QStringList PHPFile::readFromEditor()
{
   QStringList contents;
            
   QPtrList<KParts::Part> parts( *m_phpSupport->partController()->parts() );
   QPtrListIterator<KParts::Part> it( parts );
   while( it.current() ){
      KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( it.current() );
      ++it;
                
      KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
      if ( !doc || !editIface || doc->url().path() != fileName() )
         continue;
                    
      contents = QStringList::split("\n", editIface->text().ascii(), true);
      break;
   }
   return contents;
}

QStringList PHPFile::readFromDisk()
{
   QStringList contents;
   QFile f( fileName() );
   
   if (f.open(IO_ReadOnly)) {
      QTextStream stream( &f );
      QStringList list;
      QString rawline;
      while (!stream.eof()) {
         rawline = stream.readLine();
         contents.append(rawline.stripWhiteSpace().local8Bit());
      }
      f.close();
   }
   return contents;
}

bool PHPFile::isModified() {
   return modified;
}

void PHPFile::setModified(bool value) {
   modified = value;
}

void PHPFile::Analyse() {
   m_actions.clear();
   
   m_contents = readFromEditor();
   if (m_contents.isEmpty())
      m_contents = readFromDisk();
      
   ParseSource();
   PHPCheck();

   modified = false;
}
   
Action *PHPFile::ParseClass(QString line, int lineNo) {
   if (line.find("class ", 0, FALSE) == -1)
      return NULL;

   QRegExp Class("^[ \t]*(abstract|)[ \t]*class[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*(extends[ \t]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*))?.*$");
   Class.setCaseSensitive(FALSE);

   if (Class.search(line) != -1)
      return new Action(Add_Class, Class.cap(2), "", Class.cap(4), lineNo);

   return NULL;
}

Action *PHPFile::ParseFunction(QString current, QString line, int lineNo) {
   if (line.find("function", 0, FALSE) == -1)
      return NULL;
   
   QRegExp function("^[ \t]*(final|abstract|)[ \t]*(public|private|protected|)[ \t]*(static|)[ \t]*function[ \t&]*([_a-zA-Z\x7f-\xff][_a-zA-Z0-9\x7f-\xff]*)[ \t]*\\(([_a-zA-Z\x7f-\xff]*[_$, &'\\\"0-9A-Za-z\x7f-\xff\t-=]*)\\).*$");
   function.setCaseSensitive(FALSE);
   
   if (function.search(line) != -1) {
      int flags = Flags_None;
      if (function.cap(1).lower() == "abstract")
         flags |= Flags_Abstract;
      if (function.cap(1).lower() == "final")
         flags |= Flags_Final;
      if (function.cap(2).lower() == "private")
         flags |= Flags_Private;
      if (function.cap(2).lower() == "public" || function.cap(2).isEmpty())
         flags |= Flags_Public;
      if (function.cap(2).lower() == "protected")
         flags |= Flags_Protected;
      if (function.cap(3).lower() == "static")
         flags |= Flags_Static;

      return new Action(Add_Function, function.cap(4), current, function.cap(5), lineNo, flags);
   }
      
   return NULL;
}

Action *PHPFile::ParseVariable(QString current, QString line, int lineNo) {
   if (line.find("var") == -1 && line.find("public") == -1 && line.find("private") == -1 && line.find("protected") == -1)
      return NULL;
   
   QRegExp variable("^[ \t]*(var|public|private|protected|static)[ \t]*\\$([a-zA-Z_\x7f-\xff][0-9A-Za-z_\x7f-\xff]*)[ \t;=].*$");
   variable.setCaseSensitive(FALSE);
            
   if (variable.search(line) != -1) {
      int flags = Flags_None;
      if (variable.cap(1).lower() == "private")
         flags |= Flags_Private;
      if (variable.cap(1).lower() == "public" || variable.cap(1).lower() == "var")
         flags |= Flags_Public;
      if (variable.cap(1).lower() == "protected")
         flags |= Flags_Protected;
      if (variable.cap(1).lower() == "static")
         flags |= Flags_Static;

      return new Action(Add_Var, variable.cap(2), current, "", lineNo, flags);
   }

   return NULL;
}

Action *PHPFile::ParseThisMember(QString current, QString line, int lineNo) {
   if (line.find("$this->", 0, FALSE) == -1)
      return NULL;
   
   QRegExp createthis;
   createthis.setCaseSensitive(FALSE);
   
   createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*([0-9]*)[ \t]*;");
   if (createthis.search(line) != -1)
      return new Action(Add_Var, createthis.cap(1), current, "integer", lineNo);

   if (line.find("true", 0, FALSE) != -1 || line.find("false", 0, FALSE) != -1) {
      createthis.setPattern("\\$(this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*(true|false)[ \t]*;");
      if (createthis.search(line) != -1)
         return new Action(Add_Var, createthis.cap(1), current, "boolean", lineNo);
   }
   
   if (line.find("new", 0, FALSE) != -1) {
      createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
      if (createthis.search(line) != -1)
         return new Action(Add_Var, createthis.cap(1), current, createthis.cap(2), lineNo);
   }

   if (line.find("array", 0, FALSE) != -1) {
      createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*(new|)[ \t&]*(array)[ \t]*[\\(;]+");
      if (createthis.search(line) != -1)
         return new Action(Add_Var, createthis.cap(1), current, "array", lineNo);
   }   
   return NULL;
}

Action *PHPFile::ParseMember(QString current, QString line, int lineNo) {
   if (line.find("$", 0, FALSE) == -1)
      return NULL;

   /// @todo Ajouter plus de test ....
   
   QRegExp createmember;
   createmember.setCaseSensitive(FALSE);

   createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*([0-9]*)[ \t]*;");
   if (createmember.search(line) != -1)
      return new Action(Add_Var, createmember.cap(1), current, "integer", lineNo);
      
   createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*[\"']+(.*)[\"']+[ \t]*;");
   if (createmember.search(line) != -1)
      return new Action(Add_Var, createmember.cap(1), current, "string", lineNo);

   if (line.find("true", 0, FALSE) != -1 || line.find("false", 0, FALSE) != -1) {
      createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*(true|false)[ \t]*;");
      if (createmember.search(line) != -1)
         return new Action(Add_Var, createmember.cap(1), current, "boolean", lineNo);
   }

   
   if (line.find("new", 0, FALSE) != -1) {
      createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
      if (createmember.search(line) != -1)
         return new Action(Add_Var, createmember.cap(1), current, createmember.cap(2), lineNo);
   }

   if (line.find("array", 0, FALSE) != -1) {
      createmember.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*(new|)[ \t&]*(array)[ \t]*[\\(;]+");
      if (createmember.search(line) != -1)
         return new Action(Add_Var, createmember.cap(1), current, "array", lineNo);
   }

   return NULL;
}


QString PHPFile::ParseReturn(QString current, QString line, int lineNo) {
   QString rettype;

   if (line.find("return", 0, FALSE) == -1)
      return NULL;

   QRegExp typeex;
   typeex.setCaseSensitive(FALSE);
   typeex.setPattern("return[ \t]*(\\(|)([a-zA-Z_\x7f-\xff$][a-zA-Z0-9_\x7f-\xff]*)(\\)|)[ \t]*;");

   if (typeex.search(line) != -1) {
      QString varname = typeex.cap(2).ascii();
      rettype = varname;
      
      if (varname.find("$") == 0) {
         varname = varname.mid(1);

         QValueList<Action *>::ConstIterator it = m_vars.begin();
         while ( it != m_vars.end() ) {
            Action *p = *it++;

            if (p->parent() == current && p->name() == varname) {
               rettype = p->args();
            }
         }
      } else if (varname == "true" || varname == "false") {
         rettype = "boolean";
      } else if (varname == "null") {
         rettype = "null";
      }

      if (rettype.find("$") == 0)
         kdDebug(9018) << "ParseReturn value" << current.latin1() << " " << rettype.latin1() << endl;
   }
   return rettype;
}
   
Action *PHPFile::ParseTodo(QString line, int lineNo) {
   if (line.find("todo", 0, FALSE) == -1)
      return NULL;
      
   QRegExp todo("/[/]+[ \t]*[@]*todo([ \t]*:[ \t]*|[ \t]*)[ \t]*(.*)$");
   todo.setCaseSensitive(FALSE);

   if (todo.search(line) != -1)
      return new Action(Add_Todo, todo.cap(2), "", "", lineNo);

   return NULL;
}

Action *PHPFile::ParseFixme(QString line, int lineNo) {
   if (line.find("fixme", 0, FALSE) == -1)
      return NULL;

   QRegExp fixme("/[/]+[ \t]*[@]*fixme([ \t]*:[ \t]*|[ \t]*)[ \t]*(.*)$");
   fixme.setCaseSensitive(FALSE);

   if (fixme.search(line) != -1)
      return new Action(Add_Fixme, fixme.cap(2), "", "", lineNo);

   return NULL;
}

void PHPFile::ParseSource() {
   QString line;
   int lineNo = 0;
   int bracketOpen = 0;
   int bracketClose = 0;
   int bracketFuncOpen = 0;
   int bracketFuncClose = 0;
   
   QRegExp includere("^[ \t]*(include|require|include_once|require_once)[ \t]*(\\(|)[ \t]*[\"'](.*)[\"'][ \t]*(\\)|)[ \t]*;$");

   includere.setCaseSensitive(FALSE);

   Action *Class = NULL;
   Action *Func = NULL;
   
   for ( QStringList::Iterator it = m_contents.begin(); it != m_contents.end(); ++it ) {
      line = (*it).local8Bit();

      if (!line.isNull()) {
         Action *action;
         
         if (line.find("include", 0, FALSE) != -1 || line.find("require", 0, FALSE) != -1)  {
            if (includere.search(line) != -1) {
               QStringList include_path;
               include_path = include_path.split(":", m_phpSupport->getIncludePath());
               include_path.append(URLUtil::directory(fileName()) + "/");
               include_path.append("");

               QStringList list = includere.capturedTexts();

               for ( QStringList::Iterator it = include_path.begin(); it != include_path.end(); ++it ) {
                  QString abso = URLUtil::canonicalPath(*it + "/" + list[3]);
                  if (!abso.isNull()) { 
                     QString rel = URLUtil::relativePathToFile (m_phpSupport->project()->projectDirectory(), abso);
                     m_actions.append( new Action(Add_Include, abso, "", "", lineNo) );
                  }
               }
            }
         }


         if (Func != NULL) {
            bracketFuncOpen += line.contains("{");
            bracketFuncClose += line.contains("}");
            if (bracketFuncOpen == bracketFuncClose && bracketFuncOpen != 0 && bracketFuncClose != 0) {
               //m_actions.append( new Action (Set_FuncEnd, Func->name(), "", "", Func->start(), lineNo ) );
               Func->setEnd(lineNo);
               Func = NULL;
            }
         }

         if (Func == NULL) {
            bracketOpen += line.contains("{");
            bracketClose += line.contains("}");
            if (bracketOpen == bracketClose && bracketOpen != 0 && bracketClose != 0 && Class != NULL) {
               Class->setEnd(lineNo);
               //m_actions.append( new Action (Set_ClassEnd, Class->name(), "", "", Class->start(), lineNo ) );
               Class = NULL;
            }
         }
         
         if (action == NULL) {
            action = ParseClass(line, lineNo);
            if (action != NULL) {
               Class = action;
               bracketOpen = line.contains("{");
               bracketClose = line.contains("}");
               m_actions.append(action);
            }
         }

         if (action == NULL) {
            action = ParseFunction(buildParent(Class, NULL), line, lineNo);
            if (action != NULL) {
               QRegExp fre("function[ \t&]*([_a-zA-Z\x7f-\xff][_a-zA-Z0-9\x7f-\xff]*)[ \t]*\\(([_a-zA-Z\x7f-\xff]*[_$, &'\"0-9A-Za-z\x7f-\xff\t-=]*)\\)[ \t]*(;)");
               fre.setCaseSensitive( FALSE );
               if (fre.search(line) == -1) {
                  Func = action;
                  bracketFuncOpen = line.contains("{");
                  bracketFuncClose = line.contains("}");
               }
               m_actions.append(action);
            }
         }

         if (action == NULL && Func == NULL) {
            action = ParseVariable(buildParent(Class, NULL), line, lineNo);
            if (action != NULL)
               m_actions.append(action);
         }

         if (action == NULL && Class != NULL) {
            action = ParseThisMember(buildParent(Class, NULL), line, lineNo);
            if (action != NULL)
               m_actions.append(action);
         }


         if (action == NULL && Func != NULL) {
            QString rettype = ParseReturn(buildParent(Class, Func), line, lineNo);
            if (!rettype.isEmpty()) {
               if (rettype.lower() == "$this")
                  rettype = Class->name();
                  
               if (Func->result().isEmpty() || Func->result() == "null")
                  Func->setResult(rettype);
            }
         }
         
         action = ParseMember(buildParent(Class, Func), line, lineNo);
         if (action != NULL) {
            if (Class == NULL && Func == NULL) {
               m_actions.append(action);
            } else {
               m_vars.append(action);
            }
         }

         action = ParseTodo(line, lineNo);
         if (action != NULL)
            m_actions.append(action);

         action = ParseFixme(line, lineNo);
         if (action != NULL)
            m_actions.append(action);
         
         ++lineNo;
      }
   }
}

QString PHPFile::buildParent(Action *Class, Action *Func) {
   QString parent = "";
   if (Class != NULL)
      parent = Class->name();
      
   if (Func != NULL) {
      parent = parent + "::" + Func->name();
   }
   
   return parent;
}

void PHPFile::PHPCheck() {
//   int status = 0;
   m_phpCheckOutput = "";

/// @todo try with kprocess in futur version actually this create zombie
/*   
   phpCheckProc->clearArguments();

   *phpCheckProc << m_phpSupport->getExePath();
   *phpCheckProc << "-l -f" << KShellProcess::quote(fileName());
   
   phpCheckProc->start(KProcess::DontCare, KProcess::All);
*/

/*
   char buf[255];
   FILE *fd = popen(QString(m_phpSupport->getExePath() + " -l -f " + KShellProcess::quote(fileName())).ascii(), "r");
   while (!feof(fd)) {
      memset(buf, 0, 255);
      fgets(buf, 255, fd);
      m_phpCheckOutput += buf;
   }
   pclose(fd);
   
   ParseStdout(m_phpCheckOutput);
*/
}

/* 
void PHPFile::slotReceivedPHPCheckStdout (KProcess* proc, char* buffer, int buflen) {
   kdDebug(9018) << "slotPHPExeStdout()" << endl;
   m_phpCheckOutput += QString::fromLocal8Bit(buffer,buflen+1);
}

void PHPFile::slotReceivedPHPCheckStderr (KProcess* proc, char* buffer, int buflen) {
   kdDebug(9018) << "slotPHPExeStderr()" << endl;
   m_phpCheckOutput += QString::fromLocal8Bit(buffer,buflen+1);
}

void PHPFile::slotPHPCheckExited (KProcess* proc) {
   kdDebug(v) << "slotPHPExeExited()" << endl;
}
*/

void PHPFile::ParseStdout(QString phpOutput) {
   kdDebug(9018) << "ParseStdout()" << endl;
   QRegExp parseError("^(<b>|)Parse error(</b>|): parse error, (.*) in (<b>|)(.*)(</b>|) on line (<b>|)(.*)(</b>|).*$");
   QRegExp undefFunctionError("^(<b>|)Fatal error(</b>|): Call to undefined function:  (.*) in (<b>|)(.*)(</b>|) on line (<b>|)(.*)(</b>|).*$");
   QRegExp warning("^(<b>|)Warning(</b>|): (<b>|)(.*)(</b>|) in (.*) on line (<b>|)(.*)(</b>|).*$");
   QRegExp generalFatalError("^(<b>|)Fatal error(</b>|): (.*) in (<b>|)(.*)(</b>|) on line (<b>|)(.*)(</b>|).*$");

   QStringList list = QStringList::split("\n", phpOutput);
   QStringList::Iterator it;
   for ( it = list.begin(); it != list.end(); ++it ) {
      if (generalFatalError.search(*it) >= 0) {
         m_actions.append( new Action(Add_Error, parseError.cap(5), "", parseError.cap(3), QString(parseError.cap(8)).toInt()) );
      }
      if(parseError.search(*it) >= 0){
         m_actions.append( new Action(Add_ErrorParse, parseError.cap(5), "", parseError.cap(3), QString(parseError.cap(8)).toInt()) );
      }
      if(undefFunctionError.search(*it) >= 0){
         m_actions.append( new Action(Add_ErrorNoSuchFunction, parseError.cap(5), "", parseError.cap(3), QString(parseError.cap(8)).toInt()) );
      }
      if (warning.search(*it) >= 0){
         m_actions.append( new Action(Add_Warning, parseError.cap(6),  "", parseError.cap(4), QString(parseError.cap(8)).toInt()) );
      }
   }
}

ClassDom PHPFile::classByName(QString filename, QString classname) {
   QValueList<ClassDom> CList;
   QString abso = URLUtil::canonicalPath(filename);
   ClassList classList = m_model->globalNamespace()->classList();
   
   ClassList::Iterator classIt;
   for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
      ClassDom nClass = *classIt;
      if (nClass->name().lower() == classname.lower() && nClass->fileName() == abso)
         return nClass;
   }
   return NULL;
}

QValueList<ClassDom> PHPFile::classByName(QString classname) {
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

bool PHPFile::doAction(QString filename, Action *p) {
   // ClassView work with absolute path not links
   QString abso = URLUtil::canonicalPath(filename);
   
   NamespaceDom ns = m_model->globalNamespace();
   NamespaceDom varns;
   
   QString arguments;

//   p->dump();
   
   FileDom m_file = m_model->fileByName(abso);
   if (!m_file) {
      m_file = m_model->create<FileModel>();
      m_file->setName( abso );
      m_model->addFile( m_file );
   }

   if (!m_model->globalNamespace()->hasNamespace("varsns")) {
      varns = m_model->create<NamespaceModel>();
      varns->setName("varns");
      ns->addNamespace(varns);
   }
   
   switch (p->quoi()) {
      case Add_Class:
         nClass = m_model->create<ClassModel>();
         nClass->setFileName(abso);
         nClass->setName(p->name());
         nClass->setStartPosition(p->start(), 0);
         nClass->setEndPosition(p->end(), 0);
         m_file->addClass(nClass);
         if (!p->args().isEmpty()) {
            nClass->addBaseClass(p->args());
         }

         ns->addClass(nClass);
      break;

      case Add_Var:
      {
         bool m_added = false;
         
         if (!p->parent().isEmpty()) {
            if (p->parent().find("::") == -1) {
               nClass = classByName(abso, p->parent());
               nVariable = nClass->variableByName(p->name());
            } else {
               QString name = p->parent() + "::" + p->name();
               nVariable = varns->variableByName(name);
            }
            
         } else {
            nVariable = ns->variableByName(p->name());
         }  

         if (!nVariable) {
            m_added = true;
            nVariable  = m_model->create<VariableModel>();
            nVariable->setFileName(abso);
            nVariable->setName(p->name());
            nVariable->setStartPosition( p->start(), 0 );
            nVariable->setAccess(VariableModel::Public);
            
            if (!p->parent().isEmpty()) {
               if (p->parent().find("::") == -1) {
                  nClass->addVariable( nVariable );
               } else {
                  QString name = p->parent() + "::" + p->name();
                  nVariable->setName(name);
                  varns->addVariable(nVariable);
               }
            } else {
               ns->addVariable( nVariable );
            }
            
         }

         if (p->isPrivate())
            nVariable->setAccess(VariableModel::Private);
         if (p->isPublic())
            nVariable->setAccess(VariableModel::Public);
         if (p->isProtected())
            nVariable->setAccess(VariableModel::Protected);

         nVariable->setStatic(p->isStatic());

//         if (nVariable->type().isEmpty())
         nVariable->setType( p->args() );

      }
      break;
/*            
      case Set_VarType:
         if (p->isClass()) {
            if ( !nClass->hasVariable(p->name()) ) {
               nVariable  = m_model->create<VariableModel>();
               nVariable->setAccess(VariableModel::Public);
               nVariable->setFileName(abso);
               nVariable->setName(p->name());
               nVariable->setStartPosition( p->start(), 0 );
               nClass->addVariable( nVariable );
            }
            nVariable = nClass->variableByName(p->name());
         } else {
            if ( !ns->hasVariable(p->name()) ) {
               nVariable = m_model->create<VariableModel>();
               nVariable->setAccess(VariableModel::Public);
               nVariable->setFileName(abso);
               nVariable->setName(p->name());
               nVariable->setStartPosition( p->start(), 0 );
               ns->addVariable( nVariable );
            }
            nVariable = ns->variableByName(p->name());
         }

         if (!nVariable)
            return false;
         
      break;
*/
      case Add_Function:
         nMethod = m_model->create<FunctionModel>();
         nMethod->setFileName( abso );
         nMethod->setName(p->name());
         nMethod->setStartPosition( p->start(), 0 );
         nMethod->setEndPosition( p->end(), 0 );
         nMethod->setResultType(p->result());

         if (p->isPrivate())
            nMethod->setAccess(FunctionModel::Private);
         if (p->isPublic())
            nMethod->setAccess(FunctionModel::Public);
         if (p->isProtected())
            nMethod->setAccess(FunctionModel::Protected);

         nMethod->setStatic(p->isStatic());
         nMethod->setAbstract(p->isAbstract());
         
         nArgument = m_model->create<ArgumentModel>();
         arguments = p->args();
         nArgument->setType(arguments.stripWhiteSpace().local8Bit());
         nMethod->addArgument( nArgument );
         
         if (!p->parent().isEmpty()) {
            nClass = m_file->classByName(p->parent())[0];
            nClass->addFunction(nMethod);
         } else {
            ns->addFunction(nMethod);
         }
            
      break;

      default:
      return false;
   }

   return true;
}            
#include "phpfile.moc"
