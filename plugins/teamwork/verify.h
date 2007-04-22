/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VERIFY_H
#define VERIFY_H

#include <string>
#include <sstream>

struct ExpressionError {
  std::string function;
  int line;
  std::string file;
  std::string expression;
  std::string realExpression;
  ExpressionError( const std::string& fun, int l, const std::string& f, const std::string& exp, const std::string& realExp = "" );
  std::string what() const;
};

#ifndef HAVE_TOTEXT
#define HAVE_TOTEXT

template <class Item>
std::string toText( const Item& it ) {
  std::ostringstream os;
  os << it;
  return os.str();
}
#endif

#define VERIFY( x ) if( x ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #x ); }
#define VERIFY_SAME( a, b ) if( a == b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" == " ) + #b, toText( a ) + std::string( " == " ) + toText( b ) ); }
#define VERIFY_SMALLER( a, b ) if( a < b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" < " ) + #b, toText( a ) + std::string( " < " ) + toText( b ) ); }
#define VERIFY_SMALLERSAME( a, b ) if( a <= b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" <= " ) + #b, toText( a ) + std::string( " <= " ) + toText( b ) ); }
#define VERIFY_NOTSAME( a, b ) if( !(a == b) ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" != " ) + #b, toText( a ) + std::string( " != " ) + toText( b ) ); }


///These throw a std::string
#define S_VERIFY( x ) if( x ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #x ).what(); }
#define S_VERIFY_SAME( a, b ) if( a == b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" == " ) + #b, toText( a ) + std::string( " == " ) + toText( b ) ).what(); }
#define S_VERIFY_SMALLER( a, b ) if( a < b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" < " ) + #b, toText( a ) + std::string( " < " ) + toText( b ) ).what(); }
#define S_VERIFY_SMALLERSAME( a, b ) if( a <= b ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" <= " ) + #b, toText( a ) + std::string( " <= " ) + toText( b ) ).what(); }
#define S_VERIFY_NOTSAME( a, b ) if( !(a == b) ){}else{ throw ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" != " ) + #b, toText( a ) + std::string( " != " ) + toText( b ) ).what(); }

///These throw a QString
#define Q_VERIFY( x ) if( x ){}else{ throw QString( ExpressionError( __FUNCTION__, __LINE__, __FILE__, #x ).what().c_str() ); }
#define Q_VERIFY_SAME( a, b ) if( a == b ){}else{ throw QString( ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" == " ) + #b, toText( a ) + std::string( " == " ) + toText( b ) ).what().c_str() ); }
#define Q_VERIFY_SMALLER( a, b ) if( a < b ){}else{ throw QString( ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" < " ) + #b, toText( a ) + std::string( " < " ) + toText( b ) ).what().c_str() ); }
#define Q_VERIFY_SMALLERSAME( a, b ) if( a <= b ){}else{ throw QString( ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" <= " ) + #b, toText( a ) + std::string( " <= " ) + toText( b ) ).what().c_str() ); }
#define Q_VERIFY_NOTSAME( a, b ) if( !(a == b) ){}else{ throw QString( ExpressionError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" != " ) + #b, toText( a ) + std::string( " != " ) + toText( b ) ).what().c_str() ); }

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
