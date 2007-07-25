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

#ifndef KDEVTEAMWORK_UTILS
#define KDEVTEAMWORK_UTILS

#include <QString>
#include <string>
#include <ostream>

QString toQ( const std::string& rhs );
std::string fromQ( const QString& str );

///This operator converts between Q- and std-classes
std::string operator ~ ( const QString& rhs );

QString operator ~ ( const std::string& rhs );

std::ostream& operator << ( std::ostream& stream, const QString& str );

void indexToLineCol( int index, const QString& text, int& line, int& col );

///Returns -1 if the index does not exist
int lineColToIndex( const QString& text, int line, int col );

void indexToLineCol( int index, const std::string& text, int& line, int& col );

///Returns -1 if the index does not exist
int lineColToIndex( const std::string& text, int line, int col );

struct Block {
  bool& b;
  bool old;
  Block( bool& bl ) : b( bl ), old(bl) {
    b = true;
  }
  ~Block() {
    b = old;
  }
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
