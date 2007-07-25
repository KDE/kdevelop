/* 
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QString>
#include <QChar>
#include <QStringList>
#include "stringhelpers.h"

namespace KDevelop {

bool parenFits( QChar c1, QChar c2 ) {
  if( c1 == '<' && c2 == '>' ) return true;
  else if( c1 == '(' && c2 == ')' ) return true;
  else if( c1 == '[' && c2 == ']' ) return true;
  else if( c1 == '{' && c2 == '}' ) return true;
  else
    return false;
}

int findClose( const QString& str , int pos ) {
  int depth = 0;
  QList<QChar> st;
  QChar last = ' ';
  
  for( int a = pos; a < (int)str.length(); a++) {
    switch(str[a].toAscii()) {
    case '<':
    case '(':
      case '[':
        case '{':
        st.push_front( str[a] );
      depth++;
      break;
    case '>':
      if( last == '-' ) break;
    case ')':
      case ']':
        case '}':
        if( !st.isEmpty() && parenFits(st.front(), str[a]) ) {
          depth--;
          st.pop_front();
        }
      break;
    case '"':
      last = str[a];
      a++;
      while( a < (int)str.length() && (str[a] != '"' || last == '\\')) {
        last = str[a];
        a++;
      }
      continue;
      break;
    }
    
    last = str[a];
    
    if( depth == 0 ) {
      return a;
    }
  }
  
  return -1;
}

int findCommaOrEnd( const QString& str , int pos, QChar validEnd) {
  
  for( int a = pos; a < (int)str.length(); a++) {
    switch(str[a].toAscii()) {
    case '"':
    case '(':
      case '[':
        case '{':
        case '<':
        a = findClose( str, a );
      if( a == -1 ) return str.length();
      break;
    case ')':
      case ']':
        case '}':
        case '>':
        if( validEnd != ' ' && validEnd != str[a] )
          continue;
    case ',':
      return a;
    }
  }
  
  return str.length();
}

ParamIterator::ParamIterator( QString parens, QString source ) : m_source( source ), m_parens( parens ), m_cur( 0 ), m_curEnd ( 0 ) {
  int begin = m_source.indexOf( m_parens[ 0 ] );
  int end = m_source.lastIndexOf( m_parens[ 1 ] );
  
  if( begin != -1 )
    m_prefix = m_source.left( begin );
  else
    m_prefix = m_source;

  if ( begin == -1 || end == -1 && end - begin > 1 )
    m_cur = m_source.length();
  else {
    m_source = source.mid( begin + 1, end - begin );
    m_curEnd = next();
  }
}

ParamIterator& ParamIterator::operator ++() {
  m_cur = m_curEnd + 1;
  if ( m_cur < ( int ) m_source.length() ) {
    m_curEnd = next();
  }
  return *this;
}

QString ParamIterator::operator *() {
  return m_source.mid( m_cur, m_curEnd - m_cur ).trimmed();
}

ParamIterator::operator bool() const {
  return m_cur < ( int ) m_source.length();
}

QString ParamIterator::prefix() const {
        return m_prefix;
}

int ParamIterator::next() const {
  return findCommaOrEnd( m_source, m_cur, m_parens[ 1 ] );
}

}
