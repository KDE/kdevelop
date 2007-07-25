/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "utils.h"
#include <QDomDocument>
#include <QDomNode>
#include <QTextStream>
#include <sstream>
#include <iostream>
//#include <ostringstream>
#include <QList>
#include <QByteArray>
#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/traits.hpp>
#include <boost/serialization/level_enum.hpp>


QString toQ( const std::string& str ) {
	return QString::fromUtf8( str.c_str() );
	//return QString::fromStdString( str );
}

std::string fromQ( const QString& str ) {
	return str.toUtf8().data();
	//return QString::toStdString( str );
}

std::string operator ~ ( const QString& rhs ) {
	return fromQ( rhs );
}

QString operator ~ ( const std::string& rhs ) {
	return toQ(rhs );
}

std::ostream& operator << ( std::ostream& stream, const QString& str ) {
	stream << str.toUtf8().data();
  return stream;
}

void indexToLineCol( int position, const QString& text, int& line, int& col ) {
	line = 0;
	col = 0;
	int tlen = (int)text.length();
	for( int a = 0; a < tlen; a++ )
	{
		if( a == position ) return;
		if( text[a] == '\n' ) {
			line++;
			col = 0;
		} else {
			col++;
		}
	}
	return;
}

int lineColToIndex( const QString& text, int sline, int scol ) {
	int line = 0;
	int col = 0;
	int tlen = (int)text.length();
	for( int a = 0; a < tlen; a++ )
	{
		if( line == sline && col == scol ) return a;
		if( text[a] == '\n' ) {
			line++;
			col = 0;
		} else {
			col++;
		}
	}
  if( line == sline && col == scol ) return tlen; ///Include the last bounding index
  return -1;
}

void indexToLineCol( int position, const std::string& text, int& line, int& col ) {
  line = 0;
  col = 0;
  int tlen = (int)text.length();
  for( int a = 0; a < tlen; a++ )
  {
    if( a == position ) return;
    if( text[a] == '\n' ) {
      line++;
      col = 0;
    } else {
      col++;
    }
  }
  return;
}

int lineColToIndex( const std::string& text, int sline, int scol ) {
  int line = 0;
  int col = 0;
  int tlen = (int)text.length();
  for( int a = 0; a < tlen; a++ )
  {
    if( line == sline && col == scol ) return a;
    if( text[a] == '\n' ) {
      line++;
      col = 0;
    } else {
      col++;
    }
  }
  if( line == sline && col == scol ) return tlen; ///Include the last bounding index
  return -1;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
