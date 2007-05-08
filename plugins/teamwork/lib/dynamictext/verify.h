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
#ifndef DYNAMICTEXT_VERIFY_H
#define DYNAMICTEXT_VERIFY_H

#include <string>
#include <sstream>

struct DynamicTextErrorDummy {
  std::string what() const {
		return "";
	}
};

struct DynamicTextError {
  std::string function;
  int line;
  std::string file;
  std::string expression;
  std::string realExpression;
  DynamicTextError( const std::string& fun, int l = 0, const std::string& f = "", const std::string& exp  = "", const std::string& realExp = "" );
  std::string what() const;
};

#define DYN_VERIFY( x ) if( x ){}else{ throw DynamicTextError( __FUNCTION__, __LINE__, __FILE__, #x ); }
#define DYN_VERIFY_SAME( a, b ) if( a == b ){}else{ throw DynamicTextError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" == " ) + #b, "\"" + toText( a ) + std::string( "\" == \"" ) + toText( b ) + "\"" ); }
#define DYN_VERIFY_SMALLER( a, b ) if( a < b ){}else{ throw DynamicTextError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" < " ) + #b, "\"" + toText( a ) + std::string( "\" <  \"" ) + toText( b ) + "\"" ); }
#define DYN_VERIFY_SMALLERSAME( a, b ) if( a <= b ){}else{ throw DynamicTextError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" <= " ) + #b,  "\"" + toText( a ) + std::string( "\" <= \"" ) + toText( b ) + "\"" ); }
#define DYN_VERIFY_NOTSAME( a, b ) if( !(a == b) ){}else{ throw DynamicTextError( __FUNCTION__, __LINE__, __FILE__, #a + std::string(" != " ) + #b,  "\"" + toText( a ) + std::string( "\"  != \"" ) + toText( b ) + "\"" ); }



#ifndef HAVE_TOTEXT
#define HAVE_TOTEXT
template <class Item>
std::string toText( const Item& it ) {
  ostringstream os;
  os << it;
  return os.str();
}
#endif

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
