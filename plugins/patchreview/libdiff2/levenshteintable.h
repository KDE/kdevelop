/*******************************************************************************
**
** Filename   : levenshteintable.h
** Created on : 08 november, 2003
** Copyright 2003 Otto Bruggeman <bruggie@home.nl>
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

#include "difference.h"


namespace Diff2 {

class Marker;

class LevenshteinTable
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

	/** This will calculate the levenshtein distance of 2 strings */
	unsigned int createTable( DifferenceString* s, DifferenceString* d );

	void createListsOfMarkers( void );
	int chooseRoute( int c1, int c2, int c3 );

protected:
	LevenshteinTable( const LevenshteinTable& table );
	const LevenshteinTable& operator = ( const LevenshteinTable& table );

private:
	unsigned int      m_width;
	unsigned int      m_height;
	unsigned int      m_size;
	unsigned int*     m_table;
	DifferenceString* m_source;
	DifferenceString* m_destination;
};

} // namespace Diff2

#endif // LEVENSHTEIN_H
