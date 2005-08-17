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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __phpsupport_events_h
#define __phpsupport_events_h

#include <qevent.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <QCustomEvent>

#if QT_VERSION < 0x030100
#include <kdevmutex.h>
#else
#include <qmutex.h>
#endif

enum
{
    Event_FileParsed = QEvent::User + 1000
};

class FileParsedEvent: public QCustomEvent
{
public:
   FileParsedEvent( const QString& fileName, const Q3ValueList<Action *>& actions )
   : QCustomEvent(Event_FileParsed), m_fileName( fileName )
   {
       Q3ValueListConstIterator<Action *> it = actions.begin();
       while (it != actions.end()) {
           Action *p = *it;
           Action *a = new Action(p->quoi(), p->name(), p->parent(), p->args(), p->start(), p->flags());
           a->setEnd( p->end() );
           a->setResult( p->result() );
           m_actions.append(a);
           ++it;
       }
   }

   ~FileParsedEvent()
   {
      Q3ValueListConstIterator<Action *> it = m_actions.begin();
      while (it != m_actions.end()) {
         Action *p = *it;
         delete p;
         ++it;
      }
   }

    QString fileName() const { return m_fileName; }
    Q3ValueList<Action *> actions() const { return m_actions; }

private:
    QString m_fileName;
    Q3ValueList<Action *> m_actions;

private:
    FileParsedEvent( const FileParsedEvent& source );
    void operator = ( const FileParsedEvent& source );
};


#endif // __phpsupport_events_h
