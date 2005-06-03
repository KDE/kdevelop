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

QValueList<Action> PHPFile::getActions() {
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

void PHPFile::ParseSource() {
   QRegExp todore("^[ \\t]*/([/]+|(\\*.*))([ \\t]*|)(@|)todo([ \\t]*|)(:|)(.*)$");
   QRegExp fixmere("^[ \\t]*/([/]+|(\\*.*))([ \\t]*|)(@|)fixme([ \\t]*|)(:|)(.*)$");

   QRegExp includere("^[ \t]*(include|require|include_once|require_once)[ \t]*(\\(|)[ \t]*[\"'](.*)[\"'][ \t]*(\\)|)[ \t]*;$");
   QRegExp classre("^[ \t]*class[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*(extends[ \t]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*))?.*$");
   QRegExp methodre("^[ \t]*function[ \t&]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*\\(([a-zA-Z_\x7f-\xff]*[0-9A-Za-z_\x7f-\xff\\$\\, \t-=&\\'\\\"]*)\\).*$");
   QRegExp varre("^[ \t]*var[ \t]*\\$([a-zA-Z_\x7f-\xff][0-9A-Za-z_\x7f-\xff]*)[ \t;=].*$");
   QRegExp creatememberre("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");
  
   QString line;
   int lineNo = 0;
   int bracketOpen = 0;
   int bracketClose = 0;
   bool inClass = false;

   todore.setCaseSensitive(FALSE);
   fixmere.setCaseSensitive(FALSE);
   includere.setCaseSensitive(FALSE);
   classre.setCaseSensitive(FALSE);
   methodre.setCaseSensitive(FALSE);
   varre.setCaseSensitive(FALSE);
   creatememberre.setCaseSensitive(FALSE);
   
   for ( QStringList::Iterator it = m_contents.begin(); it != m_contents.end(); ++it ) {
      line = (*it).local8Bit();

      if (!line.isNull()){
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
                  m_actions.append( Action(abso, "", lineNo, 0, Action::Level_Include) );
               }
            }
         }

         bracketOpen += line.contains("{");
         bracketClose += line.contains("}");
         if (bracketOpen == bracketClose && bracketOpen != 0 && bracketClose != 0)
            inClass = false;
      
         if ( classre.search(line) != -1 ) {
            inClass = true;
            bracketOpen = line.contains("{");
            bracketClose = line.contains("}");
            m_actions.append( Action(classre.cap(1), classre.cap(3), lineNo, 0, Action::Level_Class) );
         }
         if ( creatememberre.search(line) != -1 ) {
            m_actions.append( Action(creatememberre.cap(1), creatememberre.cap(2), lineNo, inClass, Action::Level_VarType) );
         }
         if ( methodre.search(line) != -1 ) {
            m_actions.append( Action(methodre.cap(1), methodre.cap(2), lineNo, inClass, Action::Level_Method) );
         } 
         if (varre.search(line) != -1)  {
            m_actions.append( Action(varre.cap(1), "", lineNo, inClass, Action::Level_Var ) );
         }
         if (todore.search(line) != -1)  {
            m_actions.append( Action(todore.cap(7), "", lineNo, 0, Action::Level_Todo ) );
         }
         if (fixmere.search(line) != -1)  {
            m_actions.append( Action(fixmere.cap(7), "", lineNo, 0, Action::Level_Fixme ) );
         }
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
   char buf[255];
   FILE *fd = popen(QString(m_phpSupport->getExePath() + " -l -f " + KShellProcess::quote(fileName())).ascii(), "r");
   while (!feof(fd)) {
      memset(buf, 0, 255);
      fgets(buf, 255, fd);
      m_phpCheckOutput += buf;
   }
   pclose(fd);
   
   ParseStdout(m_phpCheckOutput);
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
         m_actions.append( Action( parseError.cap(5),  parseError.cap(3), QString(parseError.cap(8)).toInt(), 0, Action::Level_Error));
      }
      if(parseError.search(*it) >= 0){
         m_actions.append( Action( parseError.cap(5), parseError.cap(3), QString(parseError.cap(8)).toInt(), 0, Action::Level_ErrorParse));
      }
      if(undefFunctionError.search(*it) >= 0){
         m_actions.append( Action( parseError.cap(5), parseError.cap(3), QString(parseError.cap(8)).toInt(), 0, Action::Level_ErrorNoSuchFunction));
      }
      if (warning.search(*it) >= 0){
         m_actions.append( Action( parseError.cap(6),  parseError.cap(4), QString(parseError.cap(8)).toInt(), 0, Action::Level_Warning));
      }
   }
}
#include "phpfile.moc"
