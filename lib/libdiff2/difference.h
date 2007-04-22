/***************************************************************************
                                difference.h  -  description
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

#ifndef DIFFERENCE_H
#define DIFFERENCE_H

#include <q3valuelist.h>
#include <q3valuevector.h>

#include <kdebug.h>

class QString;

namespace Diff2
{

class LevenshteinTable;

class Marker
{
public:
	enum Type { Start = 0, End = 1 };

public:
	Marker()
	{
		m_type = Marker::Start;
		m_offset = 0;
	}
	Marker( enum Marker::Type type, unsigned int offset )
	{
		m_type = type;
		m_offset = offset;
	}
	~Marker() {}

public:
	enum Marker::Type type()   const { return m_type;   }
	unsigned int      offset() const { return m_offset; }

	void setType  ( enum Marker::Type type ) { m_type   = type;   }
	void setOffset( unsigned int offset )    { m_offset = offset; }

private:
	enum Marker::Type m_type;
	unsigned int      m_offset;
};

typedef Q3ValueList<Marker*> MarkerList;
typedef Q3ValueList<Marker*>::iterator MarkerListIterator;
typedef Q3ValueList<Marker*>::const_iterator MarkerListConstIterator;

class DifferenceString
{
public:
	DifferenceString()
	{
//		kDebug(8101) << "DifferenceString::DifferenceString()" << endl;
	}
	DifferenceString( const QString& string, const MarkerList& markerList = MarkerList() ) :
		m_string( string ), 
		m_markerList( markerList )
	{
//		kDebug(8101) << "DifferenceString::DifferenceString( " << string << ", " << markerList << " )" << endl;
		calculateHash();
	}
	DifferenceString( const DifferenceString& ds ) :
		m_string( ds.m_string ),
		m_conflict( ds.m_conflict ),
		m_hash( ds.m_hash ),
		m_markerList( ds.m_markerList )
	{
//		kDebug(8101) << "DifferenceString::DifferenceString( const DifferenceString& " << ds << " )" << endl;
	}
	~DifferenceString() {}

public:
	const QString& string() const
	{
		return m_string;
	}
	const QString& conflictString() const
	{
		return m_conflict;
	}
	const MarkerList& markerList()
	{
		return m_markerList;
	}
	void setString( const QString& string )
	{
		m_string = string;
		calculateHash();
	}
	void setConflictString( const QString& conflict )
	{
		m_conflict = conflict;
	}
	void setMarkerList( const MarkerList& markerList )
	{
		m_markerList = markerList;
	}
	void prepend( Marker* marker )
	{
		m_markerList.prepend( marker );
	}
	bool operator==( const DifferenceString& ks )
	{
		if ( m_hash != ks.m_hash )
			return false;
		return m_string == ks.m_string;
	}

protected:
	void calculateHash()
	{
		unsigned short const* str = reinterpret_cast<unsigned short const*>( m_string.unicode() );
		const unsigned int len = m_string.length();

		m_hash = 1315423911;

		for ( unsigned int i = 0; i < len; i++ )
		{
			m_hash ^= ( m_hash << 5 ) + str[i] + ( m_hash >> 2 );
		}
	}

private:
	QString      m_string;
	QString      m_conflict;
	unsigned int m_hash;
	MarkerList   m_markerList;
};

typedef Q3ValueVector<DifferenceString*> DifferenceStringList;
typedef Q3ValueVector<DifferenceString*>::iterator DifferenceStringListIterator;
typedef Q3ValueVector<DifferenceString*>::const_iterator DifferenceStringListConstIterator;

class Difference
{
public:
	enum Type { Change, Insert, Delete, Unchanged };

public:
	Difference( int sourceLineNo, int destinationLineNo, int type = Difference::Unchanged );
	~Difference();

public:
	int type() const { return m_type; };

	int sourceLineNumber() const { return m_sourceLineNo; }
	int destinationLineNumber() const { return m_destinationLineNo; }

	int sourceLineCount() const;
	int destinationLineCount() const;

	DifferenceString* sourceLineAt( int i ) { return m_sourceLines[ i ]; }
	DifferenceString* destinationLineAt( int i ) { return m_destinationLines[ i ]; }

	const DifferenceStringList sourceLines() const { return m_sourceLines; }
	const DifferenceStringList destinationLines() const { return m_destinationLines; }

	bool hasConflict() const
	{
		return m_conflicts;
	}
	void setConflict( bool conflicts )
	{
		m_conflicts = conflicts;
	}

	void apply( bool apply );
	bool applied() const { return m_applied; }

	void setType( int type ) { m_type = type; }

	void addSourceLine( QString line );
	void addDestinationLine( QString line );

	/** This method will calculate the differences between the individual strings and store them as Markers */
	void determineInlineDifferences();

	QString recreateDifference() const;

private:
	int                   m_type;

	int                   m_sourceLineNo;
	int                   m_destinationLineNo;

	DifferenceStringList  m_sourceLines;
	DifferenceStringList  m_destinationLines;

	bool                  m_applied;
	bool                  m_conflicts;

	LevenshteinTable*     m_table;
};

typedef Q3ValueList<Difference*> DifferenceList;
typedef Q3ValueList<Difference*>::iterator DifferenceListIterator;
typedef Q3ValueList<Difference*>::const_iterator DifferenceListConstIterator;

} // End of namespace Diff2

#endif

