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

#ifndef PHPFILE_H
#define PHPFILE_H

#include <kapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qthread.h>

#include <urlutil.h>
#include <kprocess.h>
#include <kdebug.h>

#include <kdevproject.h>
#include <kdevlanguagesupport.h>
#include <kdevpartcontroller.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>

#include "phpsupportpart.h"
#include "phperrorview.h"
#include "phpparser.h"
#include "phpsupport_event.h"

/**
@author Escuder Nicolas
*/
class PHPFile : public QObject {
Q_OBJECT

public:
   PHPFile(PHPSupportPart *phpSupport, const QString& fileName);
   ~PHPFile();

   QStringList getContents();

   bool isModified();
   void setModified(bool value);
   QString fileName();
   void Analyse();
   void ParseStdout(QString phpOutput);
/*
   ClassDom classByName(QString filename, QString classname);
   QValueList<ClassDom> classByName(QString classname);
*/

/*
private slots:
    void slotReceivedPHPCheckStderr (KProcess* proc, char* buffer, int buflen);
    void slotReceivedPHPCheckStdout (KProcess* proc, char* buffer, int buflen);
    void slotPHPCheckExited (KProcess* proc);
*/
private:
   QStringList readFromEditor();
   QStringList readFromDisk();

   bool ParseClass(QString line, int lineNo);
   bool ParseFunction(QString line, int lineNo);
   bool ParseVariable(QString line, int lineNo);

   bool ParseThisMember(QString line, int lineNo);
   bool ParseMember(QString line, int lineNo);
   bool ParseReturn(QString line, int lineNo);
   bool ParseTodo(QString line, int lineNo);
   bool ParseFixme(QString line, int lineNo);

   void ParseSource();
   void PHPCheck();

   void postEvent(FileParseEvent *event);

   bool AddClass(QString name, QString extends, int start);
   bool SetClass(QString arguments);
   bool CloseClass(int end);

   bool AddFunction(QString name, QString arguments, int start);
   bool SetFunction(QString name, QString arguments = "");
   bool CloseFunction(int end);

   bool AddVariable(QString name, QString type, int position, bool classvar = FALSE);
   bool SetVariable(QString arguments);

   bool AddTodo(QString arguments, int position);
   bool AddFixme(QString arguments, int position);

   PHPSupportPart *m_part;

   bool modified;

   bool inClass;
   bool inMethod;

   QFileInfo* m_fileinfo;
   QStringList m_contents;
   QString m_phpCheckOutput;
//   KShellProcess* phpCheckProc;
};

#endif
