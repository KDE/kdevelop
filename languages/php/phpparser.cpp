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

#include "phpparser.h"
#include "phpsupportpart.h"
#include <urlutil.h>

#include <kapplication.h>
#include <kdevcore.h>
#include <kdevproject.h>
#include <codemodel.h>

#include <qregexp.h>
#include <kdebug.h>

#include <qfileinfo.h>
#include <qtextstream.h>

#include <iostream>

#include "phpsupport_event.h"

using namespace std;

PHPParser::PHPParser(PHPSupportPart *part){
  m_part = part;
  m_close = false;
  m_parse = false;
}

PHPParser::~PHPParser(){
   removeAllFiles();
}

bool PHPParser::hasFile( const QString& fileName )
{       
//   kdDebug(9018) << "hasFile " << fileName.latin1() << endl;
   QString abso = URLUtil::canonicalPath(fileName);
   QMap<QString, PHPFile *>::Iterator it = m_files.find(abso);
   
   if ( it == m_files.end() )
       return false;
   return true;
}

void PHPParser::addFile( const QString& fileName )
{  
   QString abso = URLUtil::canonicalPath(fileName);

   if ( hasFile(abso) )
      return;

//   kdDebug(9018) << "addFile " << fileName.latin1() << endl;

   QFileInfo fi( abso );
   if ((fi.extension().contains("inc") || fi.extension().contains("php") || fi.extension().contains("html") || fi.extension().contains("php3") || !fi.extension()) && !fi.extension().contains("~")) {
      m_files.insert(abso, new PHPFile(m_part, abso));
   }
}

void PHPParser::removeFile( const QString& fileName )
{
//   kdDebug(9018) << "removeFile " << fileName.latin1() << endl;
   QString abso = URLUtil::canonicalPath(fileName);
   QMap<QString, PHPFile *>::Iterator it = m_files.find(abso);
  
   if ( it != m_files.end()) {
      PHPFile *file = it.data();
      m_files.remove( abso );
      delete( file );
      file = 0;
   }
}

void PHPParser::removeAllFiles()
{
   kdDebug(9018) << "removeAllFiles" << endl;
   QMap<QString, PHPFile *>::Iterator it = m_files.begin();

   while( it != m_files.end() ){
      PHPFile * file = it.data();
      ++it;
      delete( file );
      file = 0;
   }
   m_files.clear();
}

void PHPParser::reparseFile( const QString& fileName )
{
   kdDebug(9018) << "reparseFile" << endl;
   QString abso = URLUtil::canonicalPath(fileName);
   QMap<QString, PHPFile *>::Iterator it = m_files.find(abso);
  
   if ( it != m_files.end()) {
      PHPFile *file = it.data();
      file->setModified(true);
      file = 0;
   }
   m_canParse.wakeAll();
}

void PHPParser::run() {
   kdDebug(9018) << "run thread " << getpid() << endl;
   QMap<QString, PHPFile *>::Iterator it;

   while ( !m_close ) {
      m_parse = false;
      m_canParse.wait();
      m_parse = true;

      if ( m_close )
         break;

      it = m_files.begin();

      while( it != m_files.end() ){
         PHPFile * file = it.data();
         if (!m_close) {
            if ( file->isModified() ) {
//               m_mutex.lock();
               KApplication::sendEvent( m_part, new FileParseEvent(  Event_StartParse, file->fileName() ));
               KApplication::sendPostedEvents();
               file->Analyse();
               KApplication::sendEvent( m_part, new FileParseEvent( Event_EndParse, file->fileName() ));
               KApplication::sendPostedEvents();
//               m_mutex.unlock();
               it = m_files.begin();
            } else {
              ++it;
            }
            file = 0;
         } else { 
            it = m_files.end();
         }
      }

   }
}

void PHPParser::close()
{
   kdDebug(9018) << "closing thread " << getpid() << endl;
   while (m_parse)
      QThread::usleep(500);
      
   m_close = true;
   m_canParse.wakeAll();
   while (running())
      QThread::usleep(500);
   kdDebug(9018) << "closed thread " << getpid() << endl;
}

void PHPParser::startParse() {
   kdDebug(9018) << "startParse" << endl;
   m_canParse.wakeAll();
}
