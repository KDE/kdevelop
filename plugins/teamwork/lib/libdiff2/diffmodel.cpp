/***************************************************************************
                                diffmodel.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        copyright               : (C) 2001-2003 John Firebaugh <jfirebaugh@kde.org>
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
#include <qregexp.h>
#include <q3valuelist.h>

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
	m_modified( false ),
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
	m_modified( false ),
	m_diffIndex( 0 ),
	m_selectedDifference( 0 ),
	m_blended( false )
{
}

/**  */
DiffModel::~DiffModel()
{
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
		m_destinationFile = m_source;

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
		m_modified = model.m_modified;

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

DifferenceList* DiffModel::allDifferences()
{
	if ( m_hunks.count() != 0 )
	{
		DiffHunkListConstIterator hunkIt = m_hunks.begin();
		DiffHunkListConstIterator hEnd   = m_hunks.end();

		for ( ; hunkIt != hEnd; ++hunkIt )
		{
			DiffHunk* hunk = *hunkIt;

			DifferenceListConstIterator diffIt = hunk->differences().begin();
			DifferenceListConstIterator dEnd   = hunk->differences().end();

			for ( ; diffIt != dEnd; ++diffIt )
			{
				m_allDifferences.append( *diffIt );
			}
		}
		return &m_allDifferences;
	}
	else
	{
		DifferenceList *diffList = new DifferenceList;
		return diffList;
	}
}

Difference* DiffModel::firstDifference()
{
	kDebug( 8101 ) << "DiffModel::firstDifference()" << endl;
	m_diffIndex = 0;
	kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences[ m_diffIndex ];

	return m_selectedDifference;
}

Difference* DiffModel::lastDifference()
{
	kDebug( 8101 ) << "DiffModel::lastDifference()" << endl;
	m_diffIndex = m_differences.count() - 1;
	kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences[ m_diffIndex ];

	return m_selectedDifference;
}

Difference* DiffModel::prevDifference()
{
	kDebug( 8101 ) << "DiffModel::prevDifference()" << endl;
	if ( --m_diffIndex < m_differences.count() )
	{
		kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences[ m_diffIndex ];
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0;
		kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
	}

	return m_selectedDifference;
}

Difference* DiffModel::nextDifference()
{
	kDebug( 8101 ) << "DiffModel::nextDifference()" << endl;
	if (  ++m_diffIndex < m_differences.count() )
	{
		kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences[ m_diffIndex ];
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0; // just for safety...
		kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
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

void DiffModel::setSourceFile( const QString& path )
{
	m_source = path;
	splitSourceInPathAndFileName();
}

void DiffModel::setDestinationFile( const QString& path )
{
	m_destination = path;
	splitDestinationInPathAndFileName();
}

void DiffModel::setSourceTimestamp( const QString& timestamp )
{
	m_sourceTimestamp = timestamp;
}

void DiffModel::setDestinationTimestamp( const QString& timestamp )
{
	m_destinationTimestamp = timestamp;
}

void DiffModel::setSourceRevision( const QString& revision )
{
	m_destinationRevision = revision;
}

void DiffModel::setDestinationRevision( const QString& revision )
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

void DiffModel::applyDifference( bool apply )
{
	if ( apply && !m_selectedDifference->applied() )
		m_appliedCount++;
	else if ( !apply && m_selectedDifference->applied() )
		m_appliedCount--;

	bool modified;

	// Not setting the m_modified yet so i can still query the current
	// modified status from the slot that is connected to the signal
	if ( m_appliedCount == 0 )
		modified = false;
	else
		modified = true;

	emit setModified( modified );

	m_modified = modified;

	m_selectedDifference->apply( apply );
}

void DiffModel::applyAllDifferences( bool apply )
{
	bool modified;

	// Not setting the m_modified yet so i can still query the current
	// modified status from the slot that is connected to the signal
	if ( apply )
	{
		m_appliedCount = m_differences.count();
		modified = true;
	}
	else
	{
		m_appliedCount = 0;
		modified = false;
	}

	emit setModified( modified );

	m_modified = modified;

	DifferenceListIterator diffIt = m_differences.begin();
	DifferenceListIterator dEnd   = m_differences.end();

	for ( ; diffIt != dEnd; ++diffIt )
	{
		(*diffIt)->apply( apply );
	}
}

void DiffModel::slotSetModified( bool modified )
{
	// Not setting the m_modified yet so i can still query the current
	// modified status from the slot that is connected to the signal
	emit setModified( modified );

	m_modified = modified;
}

bool DiffModel::setSelectedDifference( Difference* diff )
{
	kDebug(8101) << "diff = " << diff << endl;
	kDebug(8101) << "m_selectedDifference = " << m_selectedDifference << endl;

	if ( diff != m_selectedDifference )
	{
		if ( ( findItem( diff, m_differences ) ) == -1 )
			return false;
		// Dont set m_diffIndex if it cant be found
		m_diffIndex = findItem( diff, m_differences );
		kDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = diff;
	}

	return true;
}

#include "diffmodel.moc"

/* vim: set ts=4 sw=4 noet: */
