/***************************************************************************
                                diffmodel.h
                                -----------
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

#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "diffhunk.h"
#include "kompare.h"
#include "diff2export.h"

namespace Diff2
{

class DiffHunk;
class Difference;

class DIFF2_EXPORT DiffModel : public QObject
{
	Q_OBJECT
public:

	DiffModel( const QString& srcBaseURL, const QString& destBaseURL );
	DiffModel();
	~DiffModel();

private:
	DiffModel( const DiffModel& ) : QObject() {};

public:
	int parseDiff( enum Kompare::Format format, const QStringList& list );

	QString recreateDiff() const;

	int hunkCount() const       { return m_hunks.count(); }
	int differenceCount() const { return m_differences.count(); }
	int appliedCount() const    { return m_appliedCount; }

	DiffHunk* hunkAt( int i )               { return ( m_hunks.at( i ) ); }
	const Difference* differenceAt( int i ) const { return ( m_differences.at( i ) ); }
	Difference* differenceAt( int i ) { return ( m_differences.at( i ) ); }

	DiffHunkList*         hunks()             { return &m_hunks; }
	const DiffHunkList*   hunks() const       { return &m_hunks; }
	DifferenceList*       differences()       { return &m_differences; }
	const DifferenceList* differences() const { return &m_differences; }

	int findDifference( Difference* diff ) const { return m_differences.indexOf( diff ); }

	Difference* firstDifference();
	Difference* lastDifference();
	Difference* prevDifference();
	Difference* nextDifference();

	const QString source() const               { return m_source; }
	const QString destination() const          { return m_destination; }
	const QString sourceFile() const;
	const QString destinationFile() const;
	const QString sourcePath() const;
	const QString destinationPath() const;
	const QString sourceTimestamp() const      { return m_sourceTimestamp; }
	const QString destinationTimestamp() const { return m_destinationTimestamp; }
	const QString sourceRevision() const       { return m_sourceRevision; }
	const QString destinationRevision() const  { return m_destinationRevision; }

	void setSourceFile( QString path );
	void setDestinationFile( QString path );
	void setSourceTimestamp( QString timestamp );
	void setDestinationTimestamp( QString timestamp );
	void setSourceRevision( QString revision );
	void setDestinationRevision( QString revision );

	void addHunk( DiffHunk* hunk );
	void addDiff( Difference* diff );
	bool hasUnsavedChanges() const;

	int  diffIndex( void ) const       { return m_diffIndex; }
	void setDiffIndex( int diffIndex ) { m_diffIndex = diffIndex; }

	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

	bool setSelectedDifference( Difference* diff );

	DiffModel& operator=( const DiffModel& model );
	bool operator<( const DiffModel& model );

	int localeAwareCompareSource( const DiffModel& model );

	bool isBlended() const { return m_blended; }
	void setBlended( bool blended ) { m_blended = blended; }

	/**
	 * @p oldlines - lines that were removed.
	 * @p newLines - lines that were inserted.
	 * @p startPos - number of line at which the change occured
	 */
	QPair<QList<Difference*>, QList<Difference*> > linesChanged(const QStringList& oldLines, const QStringList& newLines, int editLineNumber);

private:
	void splitSourceInPathAndFileName();
	void splitDestinationInPathAndFileName();
	void computeDiffStats(Difference* diff);
	void processStartMarker(Difference* diff, const QStringList& lines, MarkerListConstIterator& currentMarker, int& currentListLine, bool isSource);

private:
	QString m_source;
	QString m_destination;

	QString m_sourcePath;
	QString m_destinationPath;

	QString m_sourceFile;
	QString m_destinationFile;

	QString m_sourceTimestamp;
	QString m_destinationTimestamp;

	QString m_sourceRevision;
	QString m_destinationRevision;

	DiffHunkList   m_hunks;
	DifferenceList m_differences;

	int  m_appliedCount;

	int          m_diffIndex;
	Difference*  m_selectedDifference;

	bool m_blended;

private slots:
	void slotDifferenceApplied( Difference* diff );
};

} // End of namespace Diff2

#endif

