/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __cppsupport_events_h
#define __cppsupport_events_h

#include "parser.h"

#include <qevent.h>
#include <qvaluelist.h>

enum
{
    Event_FoundProblems = QEvent::User + 1000,
    Event_FileParsed
};

class FoundProblemsEvent: public QCustomEvent
{
public:
    FoundProblemsEvent( const QString& fileName, const QValueList<Problem>& problems )
	: QCustomEvent(Event_FoundProblems), m_fileName( fileName ), m_problems( problems ) {}
    
    QString fileName() const { return m_fileName; }
    QValueList<Problem> problems() const { return m_problems; }
    
private:
    QString m_fileName;
    QValueList<Problem> m_problems;
};

class FileParsedEvent: public QCustomEvent
{
public:
    FileParsedEvent( const QString& fileName )
	: QCustomEvent( Event_FileParsed ), m_fileName( fileName ) {}
    
    QString fileName() const { return m_fileName; }
    
private:
    QString m_fileName;
};
    

#endif // __cppsupport_events_h
