/*******************************************************************************
**
** Filename   : levenshteintable.h
** Created on : 08 november, 2003
** Copyright 2003 Otto Bruggeman <bruggie@home.nl>
** Copyright 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include <iostream>
#include <QtCore/QString>

#include <kdebug.h>

namespace Diff2 {

class Marker;


class Marker;

/**
 * Computes the Levenshtein distance between two sequences.
 * The actual sequence contents must be prepended with one virtual item each for easier index access.
 */
template<class SequencePair> class LevenshteinTable
{
public:
	LevenshteinTable();
	LevenshteinTable( unsigned int width, unsigned int height );
	~LevenshteinTable();

public:
	int  getContent( unsigned int posX, unsigned int posY ) const;
	int  setContent( unsigned int posX, unsigned int posY, int value );
	bool setSize   ( unsigned int width, unsigned int height );

	unsigned int width()  const { return m_width; };
	unsigned int height() const { return m_height; };

	/** Debug method  to check if the table is properly filled */
	void dumpLevenshteinTable( void );

	/**
	 * This calculates the levenshtein distance of 2 sequences.
	 * This object takes ownership of the argument
	 */
	unsigned int createTable( SequencePair* sequences );

	void createListsOfMarkers( void );
	int chooseRoute( int c1, int c2, int c3, int current );

protected:
	LevenshteinTable( const LevenshteinTable& table );
	const LevenshteinTable& operator = ( const LevenshteinTable& table );

private:
	unsigned int      m_width;
	unsigned int      m_height;
	unsigned int      m_size;
	unsigned int*     m_table;
	SequencePair*     m_sequences;
};

template<class SequencePair> LevenshteinTable<SequencePair>::LevenshteinTable()
: m_width( 256 ),
m_height( 256 ),
m_size( m_height * m_width ),
m_table( new unsigned int[ m_size ] ),
m_sequences(0)
{
}

template<class SequencePair> LevenshteinTable<SequencePair>::LevenshteinTable( unsigned int width, unsigned int height )
: m_width( width ),
m_height( height ),
m_size( m_width * m_height ),
m_table( new unsigned int[ m_size ] ),
m_sequences(0)
{
}

template<class SequencePair> LevenshteinTable<SequencePair>::~LevenshteinTable()
{
	delete[] m_table;
	delete m_sequences;
}

template<class SequencePair> int LevenshteinTable<SequencePair>::getContent( unsigned int posX, unsigned int posY ) const
{
//  kDebug(8101) << "Width = " << m_width << ", height = " << m_height << ", posX = " << posX << ", posY = " << posY;
	return m_table[ posY * m_width + posX ];
}

template<class SequencePair> int LevenshteinTable<SequencePair>::setContent( unsigned int posX, unsigned int posY, int value )
{
	m_table[ posY * m_width + posX ] = value;

	return 0;
}

template<class SequencePair> bool LevenshteinTable<SequencePair>::setSize( unsigned int width, unsigned int height )
{
// Set a limit of 16.7 million entries, will be about 64 MB of ram, that should be plenty
	if ( ( ( width ) * ( height ) ) > ( 256 * 256 * 256 ) )
		return false;

	if ( ( ( width ) * ( height ) ) > m_size )
	{
		delete[] m_table;

		m_size = width * height;
		m_table = new unsigned int[ m_size ];
	}

	m_width = width;
	m_height = height;

	return true;
}

template<class SequencePair> void LevenshteinTable<SequencePair>::dumpLevenshteinTable()
{
	for ( unsigned int i = 0; i < m_height; ++i )
	{
		for ( unsigned int j = 0; j < m_width; ++j )
		{
			std::cout.width( 3 );
			std::cout << getContent( j, i );
		}
		std::cout << std::endl;
	}
}

template<class SequencePair> unsigned int LevenshteinTable<SequencePair>::createTable( SequencePair* sequences )
{
	m_sequences = sequences;
	unsigned int m = m_sequences->lengthFirst();
	unsigned int n = m_sequences->lengthSecond();

	if ( !setSize( m, n ) )
		return 0;

	unsigned int i;
	unsigned int j;

// initialize first row
	for ( i = 0; i < m; ++i )
		setContent( i, 0, i );
// initialize first column
	for ( j = 0; j < n; ++j )
		setContent( 0, j, j );

	int cost = 0, north = 0, west = 0, northwest = 0;

	QChar si, dj;
// Optimization, calculate row wise instead of column wise, wont trash the cache so much with large strings
	for ( j = 1; j < n; ++j )
	{
		for ( i = 1; i < m; ++i )
		{
			if (m_sequences->equal(i, j))
				cost = 0;
			else
				cost = SequencePair::allowReplace? 1 : 2;

			north     = getContent( i, j-1 ) + 1;
			west      = getContent( i-1, j ) + 1;
			northwest = getContent( i-1, j-1 ) + cost;

			setContent( i, j, qMin( north, qMin( west, northwest ) ) );
		}
	}

	return getContent( m-1, n-1 );
}

template<class SequencePair> int LevenshteinTable<SequencePair>::chooseRoute( int c1, int c2, int c3, int current )
{
//  kDebug(8101) << "c1 = " << c1 << ", c2 = " << c2 << ", c3 = " << c3;
// preference order: c2, c3, c1, hopefully this will work out for me
	if ( c2 <= c1 && c2 <= c3 )
	{
		if ( SequencePair::allowReplace || (c2 == current) )
		{
			return 1;
		}
	}

	if ( c3 <= c2 && c3 <= c1 )
		return 2;

	return 0;
}

template<class SequencePair> void LevenshteinTable<SequencePair>::createListsOfMarkers()
{
//  kDebug(8101) << source;
//  kDebug(8101) << destination;
//  dumpLevenshteinTable();

	unsigned int x = m_width-1;
	unsigned int y = m_height-1;

	unsigned int difference = getContent(x, y);

// If the number of differences is more than half the length of the largest string
// dont bother to mark the individual changes
// Patch based on work by Felix Berger as put as attachment to bug 75794
	if ( !m_sequences->needFineGrainedOutput(difference) )
	{
		m_sequences->prependFirst( new Marker( Marker::End, x ) );
		m_sequences->prependFirst( new Marker( Marker::Start, 0 ) );
		m_sequences->prependSecond( new Marker( Marker::End, y ) );
		m_sequences->prependSecond( new Marker( Marker::Start, 0 ) );
		return;
	}

	Marker* c = 0;

	int n, nw, w, direction, currentValue;
	while ( x > 0 && y > 0 )
	{
		currentValue = getContent( x, y );

		n  = getContent( x, y - 1 );
		w  = getContent( x - 1, y );
		nw = getContent( x - 1, y - 1 );
		direction = chooseRoute( n, nw, w, currentValue );
		
		switch ( direction )
		{
		case 0: // north
//          kDebug(8101) << "Picking north";
//          kDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] );

			if ( !m_sequences->markerListSecond().isEmpty() )
				c = m_sequences->markerListSecond().first();
			else
				c = 0;

			if ( c && c->type() == Marker::End )
			{
//              kDebug(8101) << "CurrentValue: " << currentValue;
				if ( n == currentValue )
					m_sequences->prependSecond( new Marker( Marker::Start, y ) );
// else: the change continues, do not do anything
			}
			else
			{
//              kDebug(8101) << "CurrentValue: " << currentValue;
				if ( n < currentValue )
					m_sequences->prependSecond( new Marker( Marker::End, y ) );
			}

			--y;
			if ( y == 0 ) {
				m_sequences->prependSecond( new Marker( Marker::Start, 0 ) );
			}
			break;
		case 1: // northwest
//          kDebug(8101) << "Picking northwest";
//          kDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] );

			if ( !m_sequences->markerListSecond().isEmpty() )
				c = m_sequences->markerListSecond().first();
			else
				c = 0;

			if ( c && c->type() == Marker::End )
			{
//              kDebug(8101) << "End found: CurrentValue: " << currentValue;
				if ( nw == currentValue )
					m_sequences->prependSecond( new Marker( Marker::Start, y ) );
// else: the change continues, do not do anything
			}
			else
			{
//              kDebug(8101) << "CurrentValue: " << currentValue;
				if ( nw < currentValue )
					m_sequences->prependSecond( new Marker( Marker::End, y ) );
			}

			if ( !m_sequences->markerListFirst().isEmpty() )
				c = m_sequences->markerListFirst().first();
			else
				c = 0;

			if ( c && c->type() == Marker::End )
			{
//              kDebug(8101) << "End found: CurrentValue: " << currentValue;
				if ( nw == currentValue )
					m_sequences->prependFirst( new Marker( Marker::Start, x ) );
// else: the change continues, do not do anything
			}
			else
			{
//              kDebug(8101) << "CurrentValue: " << currentValue;
				if ( nw < currentValue )
					m_sequences->prependFirst( new Marker( Marker::End, x ) );
			}

			--y;
			--x;
			break;
		case 2: // west
//          kDebug(8101) << "Picking west";
//          kDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] );

			if ( !m_sequences->markerListFirst().isEmpty() )
				c = m_sequences->markerListFirst().first();
			else
				c = 0;

			if ( c && c->type() == Marker::End )
			{
//              kDebug(8101) << "End found: CurrentValue: " << currentValue;
				if ( w == currentValue )
					m_sequences->prependFirst( new Marker( Marker::Start, x ) );
// else: the change continues, do not do anything
			}
			else
			{
//              kDebug(8101) << "CurrentValue: " << currentValue;
				if ( w < currentValue )
					m_sequences->prependFirst( new Marker( Marker::End, x ) );
			}

			--x;
			if (x == 0) {
				m_sequences->prependFirst( new Marker( Marker::Start, 0 ) );
			}
			break;
		}
	}

// When leaving the loop it does not mean both are 0! If not there is still a change at the beginning of the line we missed so adding now.
	if ( x != 0 )
	{
		m_sequences->prependFirst( new Marker( Marker::End, x ) );
		m_sequences->prependFirst( new Marker( Marker::Start, 0 ) );
	}

	if ( y != 0 )
	{
		m_sequences->prependSecond( new Marker( Marker::End, y ) );
		m_sequences->prependSecond( new Marker( Marker::Start, 0 ) );
	}

//  kDebug(8101) << "Source string: " << source;

//  QStringList list;
//  int prevValue = 0;
//  MarkerListConstIterator mit = m_sequences->markerListFirst().begin();
//  MarkerListConstIterator end = m_sequences->markerListFirst().end();
//  for ( ; mit != end; ++mit )
//  {
//      c = *mit;
//      kDebug(8101) << "Source Marker Entry : Type: " << c->type() << ", Offset: " << c->offset();
//      list.append( source.mid( prevValue, c->offset() - prevValue ) );
//      prevValue = c->offset();
//  }
//  if ( prevValue < source.length() - 1 )
//  {
//      list.append( source.mid( prevValue, source.length() - prevValue ) );
//  }
//  kDebug(8101) << "Source Resulting stringlist : " << list.join("\n");
//
//  list.clear();
//  prevValue = 0;
//
//  kDebug(8101) << "Destination string: " << destination;
//  mit = m_sequences->markerListSecond().begin();
//  end = m_sequences->markerListSecond().end();
//  for ( ; mit != end; ++mit )
//  {
//      c = *mit;
//      kDebug(8101) << "Destination Marker Entry : Type: " << c->type() << ", Offset: " << c->offset();
//      list.append( destination.mid( prevValue, c->offset() - prevValue ) );
//      prevValue = c->offset();
//  }
//  if ( prevValue < destination.length() - 1 )
//  {
//      list.append( destination.mid( prevValue, destination.length() - prevValue ) );
//  }
//  kDebug(8101) << "Destination Resulting string : " << list.join("\n");
}

} // namespace Diff2

#endif // LEVENSHTEIN_H
