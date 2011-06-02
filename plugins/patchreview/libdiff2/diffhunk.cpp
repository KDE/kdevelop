/***************************************************************************
                                diffhunk.cpp
                                ------------
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

#include "diffhunk.h"

#include "difference.h"

using namespace Diff2;

DiffHunk::DiffHunk( int sourceLine, int destinationLine, QString function, Type type ) :
	m_sourceLine( sourceLine ),
	m_destinationLine( destinationLine ),
	m_function( function ),
	m_type( type )
{
}

DiffHunk::~DiffHunk()
{
}

void DiffHunk::add( Difference* diff )
{
	m_differences.append( diff );
}

int DiffHunk::sourceLineCount() const
{
	DifferenceListConstIterator diffIt = m_differences.begin();
	DifferenceListConstIterator dEnd   = m_differences.end();

	int lineCount = 0;

	for ( ; diffIt != dEnd; ++diffIt )
		lineCount += (*diffIt)->sourceLineCount();

	return lineCount;
}

int DiffHunk::destinationLineCount() const
{
	DifferenceListConstIterator diffIt = m_differences.begin();
	DifferenceListConstIterator dEnd   = m_differences.end();

	int lineCount = 0;

	for ( ; diffIt != dEnd; ++diffIt )
		lineCount += (*diffIt)->destinationLineCount();

	return lineCount;
}

QString DiffHunk::recreateHunk() const
{
	QString hunk;
	QString differences;

	// recreate body
	DifferenceListConstIterator diffIt = m_differences.begin();
	DifferenceListConstIterator dEnd   = m_differences.end();

	int slc = 0; // source line count
	int dlc = 0; // destination line count
	for ( ; diffIt != dEnd; ++diffIt )
	{
		switch ( (*diffIt)->type() )
		{
		case Difference::Unchanged:
		case Difference::Change:
			slc += (*diffIt)->sourceLineCount();
			dlc += (*diffIt)->destinationLineCount();
			break;
		case Difference::Insert:
			dlc += (*diffIt)->destinationLineCount();
			break;
		case Difference::Delete:
			slc += (*diffIt)->sourceLineCount();
			break;
		}
		differences += (*diffIt)->recreateDifference();
	}

	// recreate header
	hunk += QString::fromLatin1( "@@ -%1,%3 +%2,%4 @@" )
	        .arg( m_sourceLine )
	        .arg( m_destinationLine )
	        .arg( slc )
	        .arg( dlc ); 

	if ( !m_function.isEmpty() )
		hunk += ' ' + m_function;

	hunk += QString::fromLatin1( "\n" );

	hunk += differences;

	kDebug( 8101 ) << hunk << endl;
	return hunk;
}
