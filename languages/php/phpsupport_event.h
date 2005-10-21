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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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
    Event_StartParse =  QEvent::User + 1000,
    Event_EndParse =    QEvent::User + 1001
};

class FileParseEvent: public QCustomEvent
{
public:
   FileParseEvent(long event, const QString& fileName )
   : QCustomEvent(event), m_fileName( fileName )
   {
   }

   ~FileParseEvent()
   {
   }

    QString fileName() const { return m_fileName; }

private:
    QString m_fileName;

private:
    FileParseEvent( const FileParseEvent& source );
    void operator = ( const FileParseEvent& source );
};


#endif // __phpsupport_events_h
