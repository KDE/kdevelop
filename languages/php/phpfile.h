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

#ifndef PHPFILE_H
#define PHPFILE_H

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

#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>

#include "phpsupportpart.h"

class PHPSupportPart;

class Action
{
public:
    enum
    {
        Level_Error = 0,
        Level_ErrorParse,
        Level_ErrorNoSuchFunction,
        Level_Warning,
        Level_Todo,
        Level_Fixme,
        Level_Class,
        Level_Method,
        Level_Var,
        Level_VarType,
        Level_Include
    };

public:
    Action() {}
    Action( const Action& source )
        : m_text( source.m_text ), m_args( source.m_args ), m_line( source.m_line ),
          m_column( source.m_column ), m_level( source.m_level ) {}
    Action( const QString& text, const QString &args, int line, int column, int level=Level_Error )
        : m_text( text ), m_args( args ), m_line( line ), m_column( column ), m_level(level) {}

    Action& operator = ( const Action& source )
    {
        m_text = source.m_text;
        m_args = source.m_args;
        m_line = source.m_line;
        m_column = source.m_column;
        m_level = source.m_level;
        return( *this );
    }

    bool operator == ( const Action& p ) const
    {
        return m_text == p.m_text && m_args == p.m_args && m_line == p.m_line && m_column == p.m_column && m_level == p.m_level;
    }

    QString text() const { return m_text; }
    QString args() const { return m_args; }
    int line() const { return m_line; }
    int column() const { return m_column; }
    int level() const { return m_level; }

private:
    QString m_text;
    QString m_args;
    int m_line;
    int m_column;
    int m_level;
};


/**
@author Escuder Nicolas
*/
class PHPFile : public QObject {
Q_OBJECT

public:
   PHPFile(PHPSupportPart *phpSupport, const QString& fileName);
   ~PHPFile();

   QStringList getContents();
   QValueList<Action> getActions();
   
   bool isModified();
   void setModified(bool value);
   QString fileName();
   void Analyse();
   void ParseStdout(QString phpOutput);
/*   
private slots:
    void slotReceivedPHPCheckStderr (KProcess* proc, char* buffer, int buflen);
    void slotReceivedPHPCheckStdout (KProcess* proc, char* buffer, int buflen);
    void slotPHPCheckExited (KProcess* proc);
*/         
private:
   QStringList readFromEditor();
   QStringList readFromDisk();

   void ParseSource();
   void PHPCheck();
   
   bool modified;
   PHPSupportPart *m_phpSupport;
   CodeModel *m_model;
   QFileInfo* m_fileinfo;
   QStringList m_contents;
   QString m_phpCheckOutput;
//   KShellProcess* phpCheckProc;
   
   QValueList<Action> m_actions;
};

#endif
