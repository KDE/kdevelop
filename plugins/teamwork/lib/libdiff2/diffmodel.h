/***************************************************************************
                                diffmodel.h  -  description
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

#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <qobject.h>
#include <qstringlist.h>

#include "diffhunk.h"
#include "kompare.h"

namespace Diff2
{

 template<class Container, class Item>
 int findItem( const Item& item, const Container* container ) {
   int i = 0;
   foreach( Item it, *container ) {
     if( it == item )
       return i;
     i++;
   }
   return -1;
 }

template<class Pointer>
int findItem( const Pointer* item, const DifferenceList& container ) {
  int i = 0;
  foreach( Difference* it, container ) {
    if( it == item )
      return i;
    i++;
  }
  return -1;
}

class DiffHunk;
class Difference;

class DiffModel : public QObject
{
Q_OBJECT
public:

	DiffModel( const QString& srcBaseURL, const QString& destBaseURL );
	DiffModel();
	DiffModel( const DiffModel& ) : QObject() {};
	~DiffModel();

	int parseDiff( enum Kompare::Format format, const QStringList& list );

	QString recreateDiff() const;

	int hunkCount() const       { return m_hunks.count(); }
	int differenceCount() const { return m_differences.count(); }
	int appliedCount() const    { return m_appliedCount; }

	DiffHunk* hunkAt( int i )               { return *( m_hunks.at( i ) ); }
	const Difference* differenceAt( int i ) { return *( m_differences.at( i ) ); }

	DiffHunkList*         hunks()             { return &m_hunks; }
	const DiffHunkList*   hunks() const       { return &m_hunks; }
	DifferenceList*       differences()       { return &m_differences; }
	const DifferenceList* differences() const { return &m_differences; }

	DifferenceList*       allDifferences();

	int findDifference( Difference* diff ) const {
          return findItem( diff, m_differences );
        }
          //return m_differences.( diff ); }

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

	void setSourceFile( const QString& path );
	void setDestinationFile( const QString& path );
	void setSourceTimestamp( const QString& timestamp );
	void setDestinationTimestamp( const QString& timestamp );
	void setSourceRevision( const QString& revision );
	void setDestinationRevision( const QString& revision );

	void addHunk( DiffHunk* hunk );
	void addDiff( Difference* diff );
	bool isModified() const { return m_modified; }

	const int diffIndex( void ) const       { return m_diffIndex; }
	void      setDiffIndex( int diffIndex ) { m_diffIndex = diffIndex; }

	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

	bool setSelectedDifference( Difference* diff );

	DiffModel& operator=( const DiffModel& model );
	bool operator<( const DiffModel& model );

	int localeAwareCompareSource( const DiffModel& model );

	bool isBlended() const { return m_blended; }
	void setBlended( bool blended ) { m_blended = blended; }

signals:
	void setModified( bool modified );

public slots:
	void slotSetModified( bool modified );

private:
	void splitSourceInPathAndFileName();
	void splitDestinationInPathAndFileName();

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
	DifferenceList m_allDifferences;

	int  m_appliedCount;
	bool m_modified;

	unsigned int m_diffIndex;
	Difference*  m_selectedDifference;

	bool m_blended;
};

} // End of namespace Diff2

#endif

