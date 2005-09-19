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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

#include "phphtmlview.h"
#include "phperrorview.h"

#include "phpfile.h"

using namespace std;

PHPFile::PHPFile(PHPSupportPart *phpSupport, const QString& fileName)
{
   m_fileinfo = new QFileInfo(fileName);
   m_phpSupport = phpSupport;
   m_model = m_phpSupport->codeModel();
   m_errorview = m_phpSupport->ErrorView();
   m_parser = m_phpSupport->Parser();
   modified = true;

   nClass = NULL;
   nMethod = NULL;
   ns = NULL;
   m_file = NULL;
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

//  delete phpCheckProc;
}

QStringList PHPFile::getContents()
{
   return m_contents;
}

QString PHPFile::fileName() {
   return m_fileinfo->filePath();
}

QStringList PHPFile::readFromEditor()
{
   QStringList contents;

   kapp->lock();
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
   kapp->unlock();

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
   m_contents = readFromEditor();

   if (m_contents.isEmpty())
      m_contents = readFromDisk();

   m_errorview->removeAllProblems( this->fileName() );

   ns = m_model->globalNamespace();
   m_file = m_model->fileByName( this->fileName() );
   if (!m_file) {
      m_file = m_model->create<FileModel>();
      m_file->setName( this->fileName() );
      m_model->addFile( m_file );
   }

   ParseSource();

   PHPCheck();
   modified = false;
}

bool PHPFile::ParseClass(QString line, int lineNo) {
   if (line.find("class ", 0, FALSE) == -1)
      return FALSE;

   QRegExp Class("^[ \t]*(abstract|)[ \t]*class[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*(extends[ \t]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*))?.*$");
   Class.setCaseSensitive(FALSE);

   if (Class.search(line) != -1) {
      if (AddClass(Class.cap(2), Class.cap(4), lineNo) == FALSE)
         return FALSE;

/// @fixme Activate when it exists in ClassModel
/*
      if (Class.cap(1).lower() == "abstract")
         nClass->isAbstract(true);
*/
      return TRUE;
   }

   return FALSE;
}

bool PHPFile::ParseFunction(QString line, int lineNo) {
   if (line.find("function", 0, FALSE) == -1)
      return FALSE;

   QRegExp function("^[ \t]*(final|abstract|)[ \t]*(public|private|protected|)[ \t]*(static|)[ \t]*function[ \t&]*([_a-zA-Z\x7f-\xff][_a-zA-Z0-9\x7f-\xff]*)[ \t]*\\(([_a-zA-Z\x7f-\xff]*[_$, &'\\\"0-9A-Za-z\x7f-\xff\t-=]*)\\).*$");
   function.setCaseSensitive(FALSE);

   if (function.search(line) != -1) {
      if (AddFunction(function.cap(4), function.cap(5), lineNo) == FALSE)
         return FALSE;

      if (function.cap(3).lower() == "static")
         nMethod->setStatic(true);

      if (function.cap(1).lower() == "abstract")
         nMethod->setAbstract(true);

/// @fixme Activate when it exists in FunctionModel
/*
      if (function.cap(1).lower() == "final")
         nMethod->isFinal(true);
*/

      if (function.cap(2).lower() == "private")
         nMethod->setAccess(FunctionModel::Private);

      if (function.cap(2).lower() == "public" || function.cap(2).isEmpty())
         nMethod->setAccess(FunctionModel::Public);

      if (function.cap(2).lower() == "protected")
         nMethod->setAccess(FunctionModel::Protected);

      return TRUE;
   }

   return FALSE;
}

bool PHPFile::ParseVariable(QString line, int lineNo) {
   if (line.find("var") == -1 && line.find("public") == -1 && line.find("private") == -1 && line.find("protected") == -1)
      return FALSE;

   QRegExp variable("^[ \t]*(var|public|private|protected|static)[ \t]*\\$([a-zA-Z_\x7f-\xff][0-9A-Za-z_\x7f-\xff]*)[ \t;=].*$");
   variable.setCaseSensitive(FALSE);

   if (variable.search(line) != -1) {
      bool inClass = FALSE;

      if (nClass != NULL && nMethod == NULL)
         inClass = TRUE;

      if (AddVariable(variable.cap(2), "", lineNo, inClass) == FALSE)
         return FALSE;

      if (variable.cap(1).lower() == "private")
         nVariable->setAccess(FunctionModel::Private);

      if (variable.cap(1).lower() == "public" || variable.cap(1).lower() == "var")
         nVariable->setAccess(FunctionModel::Public);

      if (variable.cap(1).lower() == "protected")
         nVariable->setAccess(FunctionModel::Protected);

      if (variable.cap(1).lower() == "static")
         nVariable->setStatic(true);

      return TRUE;
   }

   return FALSE;
}

bool PHPFile::ParseThisMember(QString line, int lineNo) {
   if (line.find("$this->", 0, FALSE) == -1)
      return FALSE;

   QRegExp createthis;
   createthis.setCaseSensitive(FALSE);

   createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*([0-9]*)[ \t]*;");
   if (createthis.search(line) != -1) {
      if (AddVariable(createthis.cap(1), "integer", lineNo, TRUE) == FALSE)
         return FALSE;
      return TRUE;
   }

   if (line.find("true", 0, FALSE) != -1 || line.find("false", 0, FALSE) != -1) {
      createthis.setPattern("\\$(this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*(true|false)[ \t]*;");
      if (createthis.search(line) != -1) {
         if (AddVariable(createthis.cap(1), "boolean", lineNo, TRUE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }

   if (line.find("new", 0, FALSE) != -1) {
      createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
      if (createthis.search(line) != -1) {
         if (AddVariable(createthis.cap(1), createthis.cap(2), lineNo, TRUE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }

   if (line.find("array", 0, FALSE) != -1) {
      createthis.setPattern("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*(new|)[ \t&]*(array)[ \t]*[\\(;]+");
      if (createthis.search(line) != -1) {
         if (AddVariable(createthis.cap(1), "array", lineNo, TRUE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }
   return FALSE;
}

bool PHPFile::ParseMember(QString line, int lineNo) {
   if (line.find("$", 0, FALSE) == -1)
      return FALSE;

   /// @todo Ajouter plus de test ....

   QRegExp createmember;
   createmember.setCaseSensitive(FALSE);

   createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*([0-9]*)[ \t]*;");
   if (createmember.search(line) != -1) {
      if (AddVariable(createmember.cap(1), "integer", lineNo, FALSE) == FALSE)
         return FALSE;
      return TRUE;
   }

   createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*[\"']+(.*)[\"']+[ \t]*;");
   if (createmember.search(line) != -1) {
      if (AddVariable(createmember.cap(1), "string", lineNo, FALSE) == FALSE)
         return FALSE;
      return TRUE;
   }

   if (line.find("true", 0, FALSE) != -1 || line.find("false", 0, FALSE) != -1) {
      createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t]*(true|false)[ \t]*;");
      if (createmember.search(line) != -1) {
         if (AddVariable(createmember.cap(1), "boolean", lineNo, FALSE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }

   if (line.find("new", 0, FALSE) != -1) {
      createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
      if (createmember.search(line) != -1) {
         if (AddVariable(createmember.cap(1), createmember.cap(2), lineNo, FALSE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }

   if (line.find("array", 0, FALSE) != -1) {
      createmember.setPattern("\\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*(new|)[ \t&]*(array)[ \t]*[\\(;]+");
      if (createmember.search(line) != -1) {
         if (AddVariable(createmember.cap(1), "array", lineNo, FALSE) == FALSE)
            return FALSE;
         return TRUE;
      }
   }

   return FALSE;
}

bool PHPFile::ParseReturn(QString line, int lineNo) {
   QString rettype;

   if (line.find("return", 0, FALSE) == -1)
      return FALSE;

   QRegExp typeex;
   typeex.setCaseSensitive(FALSE);
   typeex.setPattern("return[ \t]*(\\(|)([a-zA-Z_\x7f-\xff$][a-zA-Z0-9_\x7f-\xff]*)(\\)|)[ \t]*;");

   if (typeex.search(line) != -1) {
      QString varname = typeex.cap(2).ascii();
      rettype = varname;

      if (varname.find("$") == 0) {
         /// @todo search in variable
         /*
         varname = varname.mid(1);

         QValueList<Action *>::ConstIterator it = m_vars.begin();
         while ( it != m_vars.end() ) {
            Action *p = *it++;

            if (p->parent() == current && p->name() == varname) {
               rettype = p->args();
            }
         }
         */
      } else if (varname == "true" || varname == "false") {
         rettype = "boolean";
      } else if (varname == "null") {
         rettype = "null";
      }

      if (rettype.find("$") == 0)
         kdDebug(9018) << "ParseReturn value" << " " << rettype.latin1() << endl;
   }

   if (rettype.lower() == "$this")
      rettype = nClass->name();

   nMethod->setResultType(rettype);
   return TRUE;
}

bool PHPFile::ParseTodo(QString line, int lineNo) {
   if (line.find("todo", 0, FALSE) == -1)
      return FALSE;

   QRegExp todo("/[/]+[ \t]*[@]*todo([ \t]*:[ \t]*|[ \t]*)[ \t]*(.*)$");
   todo.setCaseSensitive(FALSE);

   if (todo.search(line) != -1) {
      m_errorview->reportProblem(Todo, this->fileName(), lineNo, todo.cap(2));
      return TRUE;
   }

   return FALSE;
}

bool PHPFile::ParseFixme(QString line, int lineNo) {
   if (line.find("fixme", 0, FALSE) == -1)
      return FALSE;

   QRegExp fixme("/[/]+[ \t]*[@]*fixme([ \t]*:[ \t]*|[ \t]*)[ \t]*(.*)$");
   fixme.setCaseSensitive(FALSE);

   if (fixme.search(line) != -1) {
      m_errorview->reportProblem(Fixme, this->fileName(), lineNo, fixme.cap(2));
      return TRUE;
   }

   return FALSE;
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

   for ( QStringList::Iterator it = m_contents.begin(); it != m_contents.end(); ++it ) {
      line = (*it).local8Bit();

      if (!line.isNull()) {
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
                     m_parser->addFile( abso );
                  }
               }
            }
         }


         if (nMethod != NULL) {
            bracketFuncOpen += line.contains("{");
            bracketFuncClose += line.contains("}");
            if (bracketFuncOpen == bracketFuncClose && bracketFuncOpen != 0 && bracketFuncClose != 0) {
               nMethod->setEndPosition(lineNo, 0);
               nMethod = NULL;
            }
         }

         if (nMethod == NULL) {
            bracketOpen += line.contains("{");
            bracketClose += line.contains("}");
            if (bracketOpen == bracketClose && bracketOpen != 0 && bracketClose != 0 && nClass != NULL) {
               nClass->setEndPosition(lineNo, 0);
               nClass = NULL;
            }
         }

         if (nClass == NULL) {
            if (ParseClass(line, lineNo) == TRUE) {
               bracketOpen = line.contains("{");
               bracketClose = line.contains("}");
            }
         }

         if (nClass != NULL) {
            ParseThisMember(line, lineNo);
         }

         if (ParseFunction(line, lineNo) == TRUE) {
            if (nMethod->isAbstract() == TRUE) {
               nMethod = NULL;
            } else {
               bracketFuncOpen = line.contains("{");
               bracketFuncClose = line.contains("}");
            }
         }

         if (nMethod != NULL)
            ParseReturn(line, lineNo);

         ParseVariable(line, lineNo);
         ParseMember(line, lineNo);
         ParseTodo(line, lineNo);
         ParseFixme(line, lineNo);

         ++lineNo;
      }
   }
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
         m_errorview->reportProblem(Error, parseError.cap(5), parseError.cap(8).toInt(), parseError.cap(3));
      }
      if(parseError.search(*it) >= 0){
         m_errorview->reportProblem(ErrorParse, parseError.cap(5), parseError.cap(8).toInt(), parseError.cap(3));
      }
      if(undefFunctionError.search(*it) >= 0){
         m_errorview->reportProblem(ErrorNoSuchFunction, parseError.cap(5), parseError.cap(8).toInt(), parseError.cap(3));
      }
      if (warning.search(*it) >= 0){
         m_errorview->reportProblem(ErrorNoSuchFunction, parseError.cap(6), parseError.cap(8).toInt(), parseError.cap(4));
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


bool PHPFile::AddClass(QString name, QString extends, int start) {
   nClass = m_model->create<ClassModel>();
   nClass->setFileName( this->fileName() );
   nClass->setName(name);
   nClass->setStartPosition(start, 0);

   m_file->addClass( nClass );
   if (extends.isEmpty() != TRUE)
      nClass->addBaseClass(extends);

   ns->addClass( nClass );
   return TRUE;
}

bool PHPFile::AddFunction(QString name, QString arguments, int start) {
   nMethod = m_model->create<FunctionModel>();
   nMethod->setFileName( this->fileName() );
   nMethod->setName(name);
   nMethod->setStartPosition( start, 0 );

   ArgumentDom nArgument;
   nArgument = m_model->create<ArgumentModel>();
   nArgument->setType(arguments.stripWhiteSpace().local8Bit());
   nMethod->addArgument( nArgument );

   if (nClass != NULL) {
      nClass->addFunction(nMethod);
   } else {
      ns->addFunction(nMethod);
   }

   return TRUE;
}

bool PHPFile::AddVariable(QString name, QString type, int start, bool ClassVar) {
   nVariable = m_model->create<VariableModel>();

   nVariable->setFileName( this->fileName() );
   nVariable->setName( name );
   nVariable->setStartPosition( start, 0 );
   nVariable->setAccess(VariableModel::Public);

   if (type.isEmpty() != TRUE)
      nVariable->setType( type );

   if (nClass != NULL && ClassVar == TRUE) {
      nClass->addVariable(nVariable);
   } else {
      if (nMethod != NULL) {
         kdDebug(9018) << "AddVariable " << nMethod->name() << " " << nVariable->name() << endl;
      } else
         ns->addVariable(nVariable);
   }
   return TRUE;
}

#include "phpfile.moc"
