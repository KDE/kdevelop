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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PHPPARSER_H
#define PHPPARSER_H

#include <qthread.h>

#if QT_VERSION < 0x030100
#include <kdevmutex.h>
#else
#include <qmutex.h>
#endif

#include <codemodel.h>

#include <qwaitcondition.h>
#include <qstring.h>
#include <qstringlist.h>

#include "phpfile.h"

class KDevCore;
class PHPSupportPart;

/**
  *@author Sandy Meier
  */

class PHPParser: public QThread {

public:
   PHPParser(PHPSupportPart *part);
   ~PHPParser();

   void addFile( const QString& fileName );
   bool hasFile( const QString& fileName );
   void reparseFile( const QString& fileName );
   void removeFile( const QString& fileName );
   void removeAllFiles();

   void run();
   void close();
   void startParse();

private:
   CodeModel* m_model;
   KDevCore* m_core;
   PHPSupportPart* m_part;

   QMutex m_mutex;
   QWaitCondition m_canParse;
   bool m_close;
   QMap<QString, PHPFile*> m_files;
};


#endif
