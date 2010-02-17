/***************************************************************************
                                diffmodel.cpp
                                -------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2009 Otto Bruggeman <bruggie@gmail.com>
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

#include "diffmodel.h"

#include <QtCore/QRegExp>

#include <kdebug.h>
#include <klocale.h>

#include "difference.h"
#include "diffhunk.h"

using namespace Diff2;

/**  */
DiffModel::DiffModel( const QString& source, const QString& destination ) :
	m_source( source ),
	m_destination( destination ),
	m_sourcePath( "" ),
	m_destinationPath( "" ),
	m_sourceFile( "" ),
	m_destinationFile( "" ),
	m_sourceTimestamp( "" ),
	m_destinationTimestamp( "" ),
	m_sourceRevision( "" ),
	m_destinationRevision( "" ),
	m_appliedCount( 0 ),
	m_diffIndex( 0 ),
	m_selectedDifference( 0 ),
	m_blended( false )
{
	splitSourceInPathAndFileName();
	splitDestinationInPathAndFileName();
}

DiffModel::DiffModel() :
	m_source( "" ),
	m_destination( "" ),
	m_sourcePath( "" ),
	m_destinationPath( "" ),
	m_sourceFile( "" ),
	m_destinationFile( "" ),
	m_sourceTimestamp( "" ),
	m_destinationTimestamp( "" ),
	m_sourceRevision( "" ),
	m_destinationRevision( "" ),
	m_appliedCount( 0 ),
	m_diffIndex( 0 ),
	m_selectedDifference( 0 ),
	m_blended( false )
{
}

/**  */
DiffModel::~DiffModel()
{
	m_selectedDifference = 0;

	qDeleteAll( m_hunks );
}

void DiffModel::splitSourceInPathAndFileName()
{
	int pos;

	if( ( pos = m_source.lastIndexOf( "/" ) ) >= 0 )
		m_sourcePath = m_source.mid( 0, pos+1 );

	if( ( pos = m_source.lastIndexOf( "/" ) ) >= 0 )
		m_sourceFile = m_source.mid( pos+1, m_source.length() - pos );
	else
		m_sourceFile = m_source;

	kDebug(8101) << m_source << " was split into " << m_sourcePath << " and " << m_sourceFile << endl;
}

void DiffModel::splitDestinationInPathAndFileName()
{
	int pos;

	if( ( pos = m_destination.lastIndexOf( "/" ) )>= 0 )
		m_destinationPath = m_destination.mid( 0, pos+1 );

	if( ( pos = m_destination.lastIndexOf( "/" ) ) >= 0 )
		m_destinationFile = m_destination.mid( pos+1, m_destination.length() - pos );
	else
		m_destinationFile = m_destination;

	kDebug(8101) << m_destination << " was split into " << m_destinationPath << " and " << m_destinationFile << endl;
}

DiffModel& DiffModel::operator=( const DiffModel& model )
{
	if ( &model != this ) // Guard from self-assignment
	{
		m_source = model.m_source;
		m_destination = model.m_destination;
		m_sourcePath = model.m_sourcePath;
		m_sourceFile = model.m_sourceFile;
		m_sourceTimestamp = model.m_sourceTimestamp;
		m_sourceRevision = model.m_sourceRevision;
		m_destinationPath = model.m_destinationPath;
		m_destinationFile = model.m_destinationFile;
		m_destinationTimestamp = model.m_destinationTimestamp;
		m_destinationRevision = model.m_destinationRevision;
		m_appliedCount = model.m_appliedCount;

		m_diffIndex = model.m_diffIndex;
		m_selectedDifference = model.m_selectedDifference;
	}

	return *this;
}

bool DiffModel::operator<( const DiffModel& model )
{
	if ( localeAwareCompareSource( model ) < 0 )
		return true;
	return false;
}

int DiffModel::localeAwareCompareSource( const DiffModel& model )
{
	kDebug(8101) << "Path: " << model.m_sourcePath << endl;
	kDebug(8101) << "File: " << model.m_sourceFile << endl;

	int result = m_sourcePath.localeAwareCompare( model.m_sourcePath );

	if ( result == 0 )
		return m_sourceFile.localeAwareCompare( model.m_sourceFile );

	return result;
}

QString DiffModel::recreateDiff() const
{
	// For now we'll always return a diff in the diff format
	QString diff;

	// recreate header
	QString tab = QString::fromLatin1( "\t" );
	QString nl  = QString::fromLatin1( "\n" );
	diff += QString::fromLatin1( "--- %1\t%2" ).arg( m_source ).arg( m_sourceTimestamp );
	if ( !m_sourceRevision.isEmpty() )
		diff += tab + m_sourceRevision;
	diff += nl;
	diff += QString::fromLatin1( "+++ %1\t%2" ).arg( m_destination ).arg( m_destinationTimestamp );
	if ( !m_destinationRevision.isEmpty() )
		diff += tab + m_destinationRevision;
	diff += nl;

	// recreate body by iterating over the hunks
	DiffHunkListConstIterator hunkIt = m_hunks.begin();
	DiffHunkListConstIterator hEnd   = m_hunks.end();

	for ( ; hunkIt != hEnd; ++hunkIt )
	{
		if ((*hunkIt)->type() != DiffHunk::AddedByBlend)
			diff += (*hunkIt)->recreateHunk();
	}

	return diff;
}

Difference* DiffModel::firstDifference()
{
	kDebug(8101) << "DiffModel::firstDifference()" << endl;
	m_diffIndex = 0;
	kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences[ m_diffIndex ];

	return m_selectedDifference;
}

Difference* DiffModel::lastDifference()
{
	kDebug(8101) << "DiffModel::lastDifference()" << endl;
	m_diffIndex = m_differences.count() - 1;
	kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences[ m_diffIndex ];

	return m_selectedDifference;
}

Difference* DiffModel::prevDifference()
{
	kDebug(8101) << "DiffModel::prevDifference()" << endl;
	if ( m_diffIndex > 0 && --m_diffIndex < m_differences.count() )
	{
		kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences[ m_diffIndex ];
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0;
		kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;
	}

	return m_selectedDifference;
}

Difference* DiffModel::nextDifference()
{
	kDebug(8101) << "DiffModel::nextDifference()" << endl;
	if (  ++m_diffIndex < m_differences.count() )
	{
		kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences[ m_diffIndex ];
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0; // just for safety...
		kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;
	}

	return m_selectedDifference;
}

const QString DiffModel::sourceFile() const
{
	return m_sourceFile;
}

const QString DiffModel::destinationFile() const
{
	return m_destinationFile;
}

const QString DiffModel::sourcePath() const
{
	return m_sourcePath;
}

const QString DiffModel::destinationPath() const
{
	return m_destinationPath;
}

void DiffModel::setSourceFile( QString path )
{
	m_source = path;
	splitSourceInPathAndFileName();
}

void DiffModel::setDestinationFile( QString path )
{
	m_destination = path;
	splitDestinationInPathAndFileName();
}

void DiffModel::setSourceTimestamp( QString timestamp )
{
	m_sourceTimestamp = timestamp;
}

void DiffModel::setDestinationTimestamp( QString timestamp )
{
	m_destinationTimestamp = timestamp;
}

void DiffModel::setSourceRevision( QString revision )
{
	m_sourceRevision = revision;
}

void DiffModel::setDestinationRevision( QString revision )
{
	m_destinationRevision = revision;
}

void DiffModel::addHunk( DiffHunk* hunk )
{
	m_hunks.append( hunk );
}

void DiffModel::addDiff( Difference* diff )
{
	m_differences.append( diff );
}

bool DiffModel::hasUnsavedChanges( void ) const
{
	DifferenceListConstIterator diffIt = m_differences.begin();
	DifferenceListConstIterator endIt  = m_differences.end();

	for ( ; diffIt != endIt; ++diffIt )
	{
		if ( (*diffIt)->isUnsaved() )
			return true;
	}

	return false;
}

void DiffModel::applyDifference( bool apply )
{
	if ( apply && !m_selectedDifference->applied() )
		m_appliedCount++;
	else if ( !apply && m_selectedDifference->applied() )
		m_appliedCount--;

	m_selectedDifference->apply( apply );
}

void DiffModel::applyAllDifferences( bool apply )
{
	if ( apply )
	{
		m_appliedCount = m_differences.count();
	}
	else
	{
		m_appliedCount = 0;
	}

	DifferenceListIterator diffIt = m_differences.begin();
	DifferenceListIterator dEnd   = m_differences.end();

	for ( ; diffIt != dEnd; ++diffIt )
	{
		(*diffIt)->apply( apply );
	}
}

bool DiffModel::setSelectedDifference( Difference* diff )
{
	kDebug(8101) << "diff = " << diff << endl;
	kDebug(8101) << "m_selectedDifference = " << m_selectedDifference << endl;

	if ( diff != m_selectedDifference )
	{
		if ( ( m_differences.indexOf( diff ) ) == -1 )
			return false;
		// Do not set m_diffIndex if it cant be found
		m_diffIndex = m_differences.indexOf( diff );
		kDebug(8101) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = diff;
	}

	return true;
}

/* vim: set ts=4 sw=4 noet: */
