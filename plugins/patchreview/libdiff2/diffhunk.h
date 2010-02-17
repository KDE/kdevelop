/***************************************************************************
                                diffhunk.h
                                ----------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef DIFFHUNK_H
#define DIFFHUNK_H

#include "difference.h"


namespace Diff2
{

class Difference;

class DiffHunk
{
public:
	enum Type { Normal, AddedByBlend };

public:
	DiffHunk( int sourceLine, int destinationLine, QString function = QString(), Type type = Normal );
	~DiffHunk();

	const DifferenceList& differences() const { return m_differences; };
	const QString& function() const           { return m_function; };

	int sourceLineNumber() const      { return m_sourceLine; };
	int destinationLineNumber() const { return m_destinationLine; };

	int sourceLineCount() const;
	int destinationLineCount() const;

	Type type() const         { return m_type; }
	void setType( Type type ) { m_type = type; }

	void add( Difference* diff );

	QString recreateHunk() const;

private:
	int            m_sourceLine;
	int            m_destinationLine;
	DifferenceList m_differences;
	QString        m_function;
	Type           m_type;
};

typedef QList<DiffHunk*> DiffHunkList;
typedef QList<DiffHunk*>::iterator DiffHunkListIterator;
typedef QList<DiffHunk*>::const_iterator DiffHunkListConstIterator;

} // End of namespace Diff2

#endif
