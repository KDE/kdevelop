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
#include "levenshteintable.h"
#include "stringlistpair.h"

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
	qDeleteAll( m_differences );
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
	connect(diff, SIGNAL(differenceApplied(Difference*)), SLOT(slotDifferenceApplied(Difference*)));
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
	bool appliedState = m_selectedDifference->applied();
	if ( appliedState == apply )
	{
		return;
	}
	if ( apply && !m_selectedDifference->applied() )
		m_appliedCount++;
	else if ( !apply && m_selectedDifference->applied() )
		m_appliedCount--;

	m_selectedDifference->apply( apply );
}

int GetDifferenceDelta(Difference* diff)
{
	int delta = diff->destinationLineCount() - diff->sourceLineCount();
	if ( !diff->applied() )
	{
		delta = -delta;
	}
	return delta;
}

void DiffModel::slotDifferenceApplied(Difference* diff)
{
	int delta = GetDifferenceDelta(diff);
	foreach( Difference* current, m_differences )
	{
		if ( current->destinationLineNumber() > diff->destinationLineNumber() )
		{
			current->setTrackingDestinationLineNumber(current->trackingDestinationLineNumber() + delta);
		}
	}
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

	int totalDelta = 0;
	for ( ; diffIt != dEnd; ++diffIt )
	{
		(*diffIt)->setTrackingDestinationLineNumber((*diffIt)->trackingDestinationLineNumber() + totalDelta);
		bool appliedState = (*diffIt)->applied();
		if ( appliedState == apply )
		{
			continue;
		}
		(*diffIt)->applyQuietly( apply );
		int currentDelta = GetDifferenceDelta(*diffIt);
		totalDelta += currentDelta;
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

QPair<QList<Difference*>, QList<Difference*> > DiffModel::linesChanged(const QStringList& oldLines, const QStringList& newLines, int editLineNumber)
{
	// These two will be returned as the function result
	QList<Difference*> inserted;
	QList<Difference*> removed;
	if (oldLines.size() == 0 && newLines.size() == 0) {
		return qMakePair(QList<Difference*>(), QList<Difference*>());
	}
	int editLineEnd = editLineNumber + oldLines.size();
	// Find the range of differences [iterBegin, iterEnd) that should be updated
	// TODO: assume that differences are ordered by starting line. Check that this is always the case
	DifferenceList applied;
	DifferenceListIterator iterBegin; // first diff ending a line before editLineNo or later
	for (iterBegin = m_differences.begin(); iterBegin != m_differences.end(); ++iterBegin) {
		// If the difference ends a line before the edit starts, they should be merged if this difference is applied.
		// Also it should be merged if it starts on editLineNumber, otherwise there will be two markers for the same line
		int lineAfterLast = (*iterBegin)->trackingDestinationLineEnd();
		if (lineAfterLast > editLineNumber || (lineAfterLast == editLineNumber &&
			((*iterBegin)->applied() || (*iterBegin)->trackingDestinationLineNumber() == editLineNumber))) {
			break;
		}
	}
	DifferenceListIterator iterEnd;
	for (iterEnd = iterBegin; iterEnd != m_differences.end(); ++iterEnd) {
		// If the difference starts a line after the edit ends, it should still be merged if it is applied
		int firstLine = (*iterEnd)->trackingDestinationLineNumber();
		if (firstLine > editLineEnd || (!(*iterEnd)->applied() && firstLine == editLineEnd)) {
			break;
		}
		if ((*iterEnd)->applied()) {
			applied.append(*iterEnd);
		}
	}

	// Compute line numbers in source and destination to which the for diff line sequences (will be created later)
	int sourceLineNumber;
	int destinationLineNumber;
	if (iterBegin == m_differences.end()) {    // All existing diffs are after the change
		destinationLineNumber = editLineNumber;
		if (!m_differences.isEmpty()) {
			sourceLineNumber = m_differences.last()->sourceLineEnd() - (m_differences.last()->trackingDestinationLineEnd() - editLineNumber);
		} else {
			sourceLineNumber = destinationLineNumber;
		}
	} else if (!(*iterBegin)->applied() || (*iterBegin)->trackingDestinationLineNumber() >= editLineNumber) {
		destinationLineNumber = editLineNumber;
		sourceLineNumber = (*iterBegin)->sourceLineNumber() - ((*iterBegin)->trackingDestinationLineNumber() - editLineNumber);
	} else {
		sourceLineNumber = (*iterBegin)->sourceLineNumber();
		destinationLineNumber = (*iterBegin)->trackingDestinationLineNumber();
	}

	// Only the applied differences are of interest, unapplied can be safely removed
	DifferenceListConstIterator appliedBegin = applied.constBegin();
	DifferenceListConstIterator appliedEnd = applied.constEnd();

	// Now create a sequence of lines for the destination file and the corresponding lines in source
	QStringList sourceLines;
	QStringList destinationLines;
	DifferenceListIterator insertPosition;  // where to insert the created diffs
	if (appliedBegin == appliedEnd) {
		destinationLines = newLines;
		sourceLines = oldLines;
	} else {
		// Create the destination line sequence
		int firstDestinationLineNumber = (*appliedBegin)->trackingDestinationLineNumber();
		for (int lineNumber = firstDestinationLineNumber; lineNumber < editLineNumber; ++lineNumber) {
			destinationLines.append((*appliedBegin)->destinationLineAt(lineNumber - firstDestinationLineNumber)->string());
		}
		foreach(const QString& line, newLines) {
			destinationLines.append(line);
		}
		DifferenceListConstIterator appliedLast = appliedEnd;
		--appliedLast;
		int lastDestinationLineNumber = (*appliedLast)->trackingDestinationLineNumber();
		for (int lineNumber = editLineEnd; lineNumber < (*appliedLast)->trackingDestinationLineEnd(); ++lineNumber) {
			destinationLines.append((*appliedLast)->destinationLineAt(lineNumber - lastDestinationLineNumber)->string());
		}

		// Create the source line sequence
		if ((*appliedBegin)->trackingDestinationLineNumber() >= editLineNumber) {
			for (int i = editLineNumber; i < (*appliedBegin)->trackingDestinationLineNumber(); ++i) {
				sourceLines.append(oldLines.at(i - editLineNumber));
			}
		}

		QStringList::const_iterator oldLinesIter = oldLines.begin();
		for (DifferenceListConstIterator iter = appliedBegin; iter != appliedEnd;) {
			int startPos = (*iter)->trackingDestinationLineNumber();
			if ((*iter)->applied()) {
				for(int i = 0; i < (*iter)->sourceLineCount(); ++i) {
					sourceLines.append((*iter)->sourceLineAt(i)->string());
				}
				startPos = (*iter)->trackingDestinationLineEnd();
			} else if (startPos < editLineNumber) {
				startPos = editLineNumber;
			}
			++iter;
			int endPos = (iter == appliedEnd) ? editLineEnd : (*iter)->trackingDestinationLineNumber();
			for (int i = startPos; i < endPos; ++i) {
				sourceLines.append(oldLines.at(i - editLineNumber));
			}
		}
	}

	for (DifferenceListIterator iter = iterBegin; iter != iterEnd; ++iter) {
		removed << *iter;
	}
	insertPosition = m_differences.erase(iterBegin, iterEnd);

	// Compute the Levenshtein table for two line sequences and construct the shortest possible edit script
	StringListPair* pair = new StringListPair(sourceLines, destinationLines);
	LevenshteinTable<StringListPair> table;
	table.createTable(pair);
	table.createListsOfMarkers();
	MarkerList sourceMarkers = pair->markerListFirst();
	MarkerList destinationMarkers = pair->markerListSecond();

	int currentSourceListLine = 0;
	int currentDestinationListLine = 0;
	MarkerListConstIterator sourceMarkerIter = sourceMarkers.constBegin();
	MarkerListConstIterator destinationMarkerIter = destinationMarkers.constBegin();
	const int terminatorLineNumber = sourceLines.size() + destinationLines.size() + 1;    // A virtual offset for simpler computation - stands for infinity

	// Process marker lists, converting pairs of Start-End markers into differences.
	// Marker in source list only stands for deletion, in source and destination lists - for change, in destination list only - for insertion.
	while(sourceMarkerIter != sourceMarkers.constEnd() || destinationMarkerIter != destinationMarkers.constEnd()) {
		int nextSourceListLine = sourceMarkerIter != sourceMarkers.constEnd() ? (*sourceMarkerIter)->offset() : terminatorLineNumber;
		int nextDestinationListLine = destinationMarkerIter != destinationMarkers.constEnd() ? (*destinationMarkerIter)->offset() : terminatorLineNumber;

		// Advance to the nearest marker
		int linesToSkip = qMin(nextDestinationListLine - currentDestinationListLine, nextSourceListLine - currentSourceListLine);
		currentSourceListLine += linesToSkip;
		currentDestinationListLine += linesToSkip;
		Difference* diff = new Difference(sourceLineNumber + currentSourceListLine, destinationLineNumber + currentDestinationListLine);
		if (nextSourceListLine == currentSourceListLine) {
			processStartMarker(diff, sourceLines, sourceMarkerIter, currentSourceListLine, true);
		}
		if (nextDestinationListLine == currentDestinationListLine) {
			processStartMarker(diff, destinationLines, destinationMarkerIter, currentDestinationListLine, false);
		}
		computeDiffStats(diff);
		Q_ASSERT(diff->type() != Difference::Unchanged);
		diff->applyQuietly(true);
		diff->setTrackingDestinationLineNumber(diff->destinationLineNumber());
		insertPosition = m_differences.insert(insertPosition, diff);
		++insertPosition;
		inserted << diff;
	}
	// Update line numbers for differences that are after the edit
	for (; insertPosition != m_differences.end(); ++insertPosition) {
		(*insertPosition)->setTrackingDestinationLineNumber((*insertPosition)->trackingDestinationLineNumber() + (newLines.size() - oldLines.size()));
	}
	return qMakePair(inserted, removed);
}

// Some common computing after diff contents have been filled.
void DiffModel::computeDiffStats(Difference* diff)
{
	if (diff->sourceLineCount() > 0 && diff->destinationLineCount() > 0) {
		diff->setType(Difference::Change);
	} else if (diff->sourceLineCount() > 0) {
		diff->setType(Difference::Delete);
	} else if (diff->destinationLineCount() > 0) {
		diff->setType(Difference::Insert);
	}
	diff->determineInlineDifferences();
}

// Helper method to extract duplicate code from DiffModel::linesChanged
void DiffModel::processStartMarker(Difference* diff, const QStringList& lines, MarkerListConstIterator& currentMarker, int& currentListLine, bool isSource)
{
	Q_ASSERT((*currentMarker)->type() == Marker::Start);
	++currentMarker;
	Q_ASSERT((*currentMarker)->type() == Marker::End);
	int nextDestinationListLine = (*currentMarker)->offset();
	for (; currentListLine < nextDestinationListLine; ++currentListLine) {
		if (isSource) {
			diff->addSourceLine(lines.at(currentListLine));
		} else {
			diff->addDestinationLine(lines.at(currentListLine));
		}
	}
	++currentMarker;
	currentListLine = nextDestinationListLine;
}

/* vim: set ts=4 sw=4 noet: */
