/*
   Copyright (C) 2003 by Roberto Raggi <roberto@kdevelop.org>
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

#ifndef __phpsupport_events_h
#define __phpsupport_events_h

#include <qevent.h>
#include <qvaluelist.h>

#if QT_VERSION < 0x030100
#include <kdevmutex.h>
#else
#include <qmutex.h>
#endif

enum
{
   Event_AddFile =  QEvent::User + 1000,
   Event_StartParse,
   Event_EndParse,
   Event_AddClass,
   Event_SetClass,
   Event_CloseClass,
   Event_AddFunction,
   Event_SetFunction,
   Event_CloseFunction,
   Event_AddVariable,
   Event_SetVariable,
   Event_AddTodo,
   Event_AddFixme
};

class FileParseEvent: public QCustomEvent
{
public:
   FileParseEvent(long event, const QString& fileName )
   : QCustomEvent(event), m_fileName( fileName )
   {
      m_name = "";
      m_arguments = "";
      m_position = 0;
      m_global = FALSE;
   }

   FileParseEvent(long event, const QString& fileName, int position )
   : QCustomEvent(event), m_fileName( fileName ), m_position( position )
   {
      m_name = "";
      m_arguments = "";
      m_global = FALSE;
   }

   FileParseEvent(long event, const QString& fileName, const QString& name, const QString& arguments )
   : QCustomEvent(event), m_fileName( fileName ), m_name( name ), m_arguments( arguments )
   {
      m_position = 0;
      m_global = FALSE;
   }

   FileParseEvent(long event, const QString& fileName, const QString& name, const QString& arguments, int position )
   : QCustomEvent(event), m_fileName( fileName ), m_name( name ), m_arguments( arguments ), m_position( position )
   {
      m_global = FALSE;
   }

   FileParseEvent(long event, const QString& fileName, const QString& name, const QString& arguments, int position, bool global )
   : QCustomEvent(event), m_fileName( fileName ), m_name( name ), m_arguments( arguments ), m_position( position ), m_global( global )
   {
   }

   ~FileParseEvent()
   {
   }

    QString fileName() const { return m_fileName; }
    QString name() const { return m_name; }
    QString arguments() const { return m_arguments; }
    int posititon() const { return m_position; }
    bool global() const { return m_global; }

private:
    QString m_fileName;
    QString m_name;
    QString m_arguments;
    QString m_accesstype;
    int m_position;
    bool m_global;

private:
    FileParseEvent( const FileParseEvent& source );
    void operator = ( const FileParseEvent& source );
};

#endif // __phpsupport_events_h
