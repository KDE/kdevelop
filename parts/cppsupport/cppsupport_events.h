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

#include "driver.h"

#include <qevent.h>
#include <qvaluelist.h>

#if QT_VERSION < 0x030100
#include <kdevmutex.h>
#else
#include <qmutex.h>
#endif

enum
{
    Event_FoundProblems = QEvent::User + 1000,
    Event_FileParsed
};

class FoundProblemsEvent: public QCustomEvent
{
public:
    FoundProblemsEvent( const QString& fileName, const QValueList<Problem>& problems )
    : QCustomEvent(Event_FoundProblems)
    {
        // the members are deep copies
        m_fileName = QString(fileName.ascii());
        QValueListConstIterator<Problem> it = problems.begin();
        while (it != problems.end()) {
            Problem p = *it;
            m_problems.append(Problem(p.text().ascii(), p.line(), p.column()));
            ++it;
        }
    }

    QString fileName() const { return m_fileName; }
    QValueList<Problem> problems() const { return m_problems; }

private:
    QString m_fileName;
    QValueList<Problem> m_problems;

private:
    FoundProblemsEvent( const FoundProblemsEvent& source );
    void operator = ( const FoundProblemsEvent& source );
};

class FileParsedEvent: public QCustomEvent
{
public:
    FileParsedEvent( const QString& fileName )
    : QCustomEvent( Event_FileParsed )
    {
        m_fileName = QString(fileName.ascii());
    }

    QString fileName() const { return m_fileName; }

private:
    QString m_fileName;

private:
    FileParsedEvent( const FileParsedEvent& source );
    void operator = ( const FileParsedEvent& source );
};


#endif // __cppsupport_events_h
