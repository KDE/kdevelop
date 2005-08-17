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
#include "kdevdeepcopy.h"

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
	FileParsedEvent( const QString& fileName, const Q3ValueList<Problem>& problems )
			: QCustomEvent( Event_FileParsed ), m_fileName( deepCopy( fileName ) )
	{
		// the members are deep copies
		Q3ValueListConstIterator<Problem> it = problems.begin();
		while ( it != problems.end() )
		{
			Problem p = *it;
			m_problems.append( Problem( deepCopy( p.text() ), p.line(), p.column(), p.level() ) );
			++it;
		}
	}

	QString fileName() const
	{
		return m_fileName;
	}
	Q3ValueList<Problem> problems() const
	{
		return m_problems;
	}

private:
	QString m_fileName;
	Q3ValueList<Problem> m_problems;

private:
	FileParsedEvent( const FileParsedEvent& source );
	void operator = ( const FileParsedEvent& source );
};


#endif // __cppsupport_events_h 
// kate: indent-mode csands; tab-width 4;

