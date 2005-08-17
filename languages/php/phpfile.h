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
//Added by qt3to4:
#include <Q3ValueList>

#include <urlutil.h>
#include <kprocess.h>
#include <kdebug.h>

#include <kdevproject.h>
#include <kdevlanguagesupport.h>
#include <kdevpartcontroller.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>

#include "phpsupportpart.h"

class PHPSupportPart;

enum
{
   Flags_None =       0x000,
   Flags_Class =      0x010,
   Flags_Method =     0x020,
   Flags_Public =     0x040,
   Flags_Private =    0x080,
   Flags_Protected =  0x100,
   Flags_Static =     0x200,
   Flags_Final =      0x400,
   Flags_Abstract =   0x800
};

enum Actions
{
   Add_Error = 0,
   Add_ErrorParse = 1,
   Add_ErrorNoSuchFunction = 2,
   Add_Warning = 3,
   Add_Todo = 4,
   Add_Fixme = 5,
   Add_Class = 6,
   Add_Function = 7,
   Add_Var = 8,
   Add_Include = 9,
   Set_ClassEnd = 10,
   Set_FuncEnd = 11
};
        
class Action
{

public:
    Action() {}
    
    Action( const Actions what, const QString& name, const QString& parent, const QString &args, int start, int flags = 0)
        : m_what ( what ), m_name( name ), m_parent ( parent ), m_args( args ), m_start( start ), m_flags( flags ) { m_end = 0; }

    bool isPrivate()
    {
       if (m_flags & Flags_Private)
          return true;
       return false;
    }

    bool isPublic()
    {
       if (m_flags & Flags_Public)
          return true;
       return false;
    }
        
    bool isProtected()
    {
       if (m_flags & Flags_Protected)
          return true;
       return false;
    }
    
    bool isStatic()
    {
       if (m_flags & Flags_Static)
          return true;
       return false;
    }

    bool isFinal()
    {
       if (m_flags & Flags_Final)
          return true;
       return false;
    }

    bool isAbstract()
    {
       if (m_flags & Flags_Abstract)
          return true;
       return false;
    }

    bool isClass()
    {
       if (m_flags & Flags_Class)
          return true;
       return false;
    }

    void inClass()
    {
       if (m_flags & Flags_Class)
          return;
       m_flags |= Flags_Class;
    }

    void setEnd(int end)
    {
       m_end = end;
    }

    void setResult(QString result)
    {
       m_result = result;
    }
    
    Actions quoi() const { return m_what; }
    QString parent() const { return m_parent; }
    QString name() const { return m_name; }
    QString args() const { return m_args; }
    QString result() const { return m_result; }
    int start() const { return m_start; }
    int end() const { return m_end; }
    int flags() const { return m_flags; }
    int level() const { return m_level; }

    void dump() { printf("%d Name: %s Parent : %s Args : %s Start : %d End : %d Flags : %d\n", m_what, m_name.ascii(), m_parent.ascii(), m_args.ascii(), m_start, m_end, m_flags); return; };
private:
    Actions m_what;
    QString m_name;
    QString m_parent;
    QString m_args;
    QString m_result;
    int m_start;
    int m_end;
    int m_flags;
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
   Q3ValueList<Action *> getActions();
   
   bool isModified();
   void setModified(bool value);
   QString fileName();
   void Analyse();
   void ParseStdout(QString phpOutput);

   static Action *ParseClass(QString line, int lineNo = 0);
   static Action *ParseFunction(QString current, QString line, int lineNo = 0);
   static Action *ParseVariable(QString current, QString line, int lineNo = 0);
   static Action *ParseThisMember(QString current, QString line, int lineNo = 0);
   static Action *ParseMember(QString current, QString line, int lineNo = 0);
   QString ParseReturn(QString current, QString line, int lineNo = 0);
   
   static Action *ParseTodo(QString line, int lineNo = 0);
   static Action *ParseFixme(QString line, int lineNo = 0);

   ClassDom classByName(QString filename, QString classname);
   Q3ValueList<ClassDom> classByName(QString classname);
   
   bool doAction(QString filename, Action *p);
   
/*   
private slots:
    void slotReceivedPHPCheckStderr (KProcess* proc, char* buffer, int buflen);
    void slotReceivedPHPCheckStdout (KProcess* proc, char* buffer, int buflen);
    void slotPHPCheckExited (KProcess* proc);
*/         
private:
   QStringList readFromEditor();
   QStringList readFromDisk();

   QString buildParent(Action *Class, Action *Func);

   void ParseSource();
   void PHPCheck();
   
   bool modified;
   PHPSupportPart *m_phpSupport;
   CodeModel *m_model;

   ClassDom nClass;
   FunctionDom nMethod;
   ArgumentDom nArgument;
   VariableDom nVariable;
   
   QFileInfo* m_fileinfo;
   QStringList m_contents;
   QString m_phpCheckOutput;
//   KShellProcess* phpCheckProc;
   
   Q3ValueList<Action *> m_actions;
   Q3ValueList<Action *> m_vars;
};

#endif
