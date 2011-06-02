/***************************************************************************
                                difference.cpp
                                --------------
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

#include "difference.h"
#include "differencestringpair.h"
#include "levenshteintable.h"

using namespace Diff2;

Difference::Difference( int sourceLineNo, int destinationLineNo, int type ) :
	QObject(),
	m_type( type ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo ),
	m_trackingDestinationLineNo( sourceLineNo ),    // The whole patch starts as unapplied
	m_applied( false ),
	m_conflicts( false ),
	m_unsaved( false )
{
}

Difference::~Difference()
{
	qDeleteAll( m_sourceLines );
	qDeleteAll( m_destinationLines );
}

void Difference::addSourceLine( QString line )
{
	m_sourceLines.append( new DifferenceString( line ) );
}

void Difference::addDestinationLine( QString line )
{
	m_destinationLines.append( new DifferenceString( line ) );
}

int Difference::sourceLineCount() const
{
	return m_sourceLines.count();
}

int Difference::destinationLineCount() const
{
	return m_destinationLines.count();
}

int Difference::sourceLineEnd() const
{
	return m_sourceLineNo + m_sourceLines.count();
}

int Difference::destinationLineEnd() const
{
	return m_destinationLineNo + m_destinationLines.count();
}

int Difference::trackingDestinationLineEnd() const
{
	return m_trackingDestinationLineNo + m_destinationLines.count();
}

void Difference::apply( bool apply )
{
	if ( apply != m_applied )
	{
		m_applied = apply;
		m_unsaved = !m_unsaved;
		emit differenceApplied(this);
	}
}

void Difference::applyQuietly(bool apply)
{
	if ( m_applied != apply )
	{
		m_unsaved = !m_unsaved;
		m_applied = apply;
	}
}

void Difference::determineInlineDifferences()
{
	if ( m_type != Difference::Change )
		return;

	// Do nothing for now when the slc != dlc
	// One could try to find the closest matching destination string for any
	// of the source strings but this is compute intensive
	int slc = sourceLineCount();

	if ( slc != destinationLineCount() )
		return;

	LevenshteinTable<DifferenceStringPair> table;

	for ( int i = 0; i < slc; ++i )
	{
		DifferenceString* sl = sourceLineAt( i );
		DifferenceString* dl = destinationLineAt( i );
		DifferenceStringPair* pair = new DifferenceStringPair(sl, dl);

		// return value 0 means something went wrong creating the table so dont bother finding markers
		if ( table.createTable( pair ) != 0 )
			table.createListsOfMarkers();
	}
}

QString Difference::recreateDifference() const
{
	QString difference;

	// source
	DifferenceStringListConstIterator stringIt = m_sourceLines.begin();
	DifferenceStringListConstIterator sEnd     = m_sourceLines.end();

	for ( ; stringIt != sEnd; ++stringIt )
	{
		switch ( m_type )
		{
		case Change:
		case Delete:
			difference += '-';
			break;
		default:
		// Insert but this is not possible in source
		// Unchanged will be handled in destination
		// since they are the same
//			kDebug( 8101 ) << "Go away, nothing to do for you in source...";
			continue;
		}
		difference += (*stringIt)->string();
	}

	//destination
	stringIt = m_destinationLines.begin();
	sEnd     = m_destinationLines.end();

	for ( ; stringIt != sEnd; ++stringIt )
	{
		switch ( m_type )
		{
		case Change:
		case Insert:
			difference += '+';
			break;
		case Unchanged:
			difference += ' ';
			break;
		default: // Delete but this is not possible in destination
//			kDebug( 8101 ) << "Go away, nothing to do for you in destination...";
			continue;
		}
		difference += (*stringIt)->string();
	}

	return difference;
}

