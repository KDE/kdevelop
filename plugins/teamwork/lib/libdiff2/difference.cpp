/***************************************************************************
                                difference.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
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
#include "levenshteintable.h"

using namespace Diff2;

Difference::Difference( int sourceLineNo, int destinationLineNo, int type ) :
	m_type( type ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo ),
	m_applied( false ),
	m_table( new LevenshteinTable() )
{
}

Difference::~Difference()
{
	delete m_table;
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

void Difference::apply( bool apply )
{
	m_applied = apply;
}

void Difference::determineInlineDifferences()
{
	if ( m_type != Difference::Change )
		return;

	// Do nothing for now when the slc != dlc
	// One could try to find the closest matching destination string for any
	// of the source strings but this is compute intensive
	if ( sourceLineCount() != destinationLineCount() )
		return;

	int slc = sourceLineCount();

	for ( int i = 0; i < slc; ++i )
	{
		DifferenceString* sl = sourceLineAt( i );
		DifferenceString* dl = destinationLineAt( i );

		// FIXME: If the table cant be created dont do the rest
		m_table->createTable( sl, dl );

		m_table->createListsOfMarkers();
	}

	// No longer needed, if we ever need to recalculate the inline differences we should
	// simply recreate the table
	delete m_table;
	m_table = 0;
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
			difference += "-";
			break;
		default:
		// Insert but this is not possible in source
		// Unchanged will be handled in destination
		// since they are the same
//			kDebug( 8101 ) << "Go away, nothing to do for you in source..." << endl;
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
			difference += "+";
			break;
		case Unchanged:
			difference += " ";
			break;
		default: // Delete but this is not possible in destination
//			kDebug( 8101 ) << "Go away, nothing to do for you in destination..." << endl;
			continue;
		}
		difference += (*stringIt)->string();
	}

	return difference;
}
