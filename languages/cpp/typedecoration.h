/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
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

#ifndef __TYPEDECORATION_H__
#define __TYPEDECORATION_H__


class TypeDecoration {
 QString m_decoration_front, m_decoration_back;
 void init( QString str ) {
  str = str.stripWhiteSpace();
  m_decoration_front = "";
  m_decoration_back = "";
  
  if( str.startsWith( "const " ) ) m_decoration_front = "const ";
  if( str.endsWith( "&" ) ) m_decoration_back = "&";
 }
public:
 TypeDecoration( const QString& str = "" ) {
  init( str );
 }
 
 ~TypeDecoration() {
 }
 
 TypeDecoration& operator = ( QString str ) {
  init( str );
  return *this;
 }
 
 QString apply( const QString& str ) const {
  QString ret = str;
  if( !ret.startsWith( m_decoration_front ) )
   ret = m_decoration_front + ret;
  
  if( !ret.endsWith( m_decoration_back ) )
   ret = ret + m_decoration_back;
  return ret;
 }
 
 void operator += ( const TypeDecoration& rhs ) {
  if( !m_decoration_front.contains( rhs.m_decoration_front) ) m_decoration_front += rhs.m_decoration_front;
  if( !m_decoration_back.contains( rhs.m_decoration_back) ) m_decoration_back += rhs.m_decoration_back;
 }
};

#endif
// kate: indent-mode csands; tab-width 4;

