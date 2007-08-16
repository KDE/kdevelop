/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "stringhelpers.h"

#include <QString>
#include <QStringList>

namespace KDevelop
{

class ParamIteratorPrivate
{
public:
  QString m_prefix;
  QString m_source;
  QString m_parens;
  int m_cur;
  int m_curEnd;
  int m_end;
  
  int next() const
  {
    return findCommaOrEnd( m_source, m_cur, m_parens[ 1 ] );
  }
};


bool parenFits( QChar c1, QChar c2 )
{
  if( c1 == '<' && c2 == '>' ) return true;
  else if( c1 == '(' && c2 == ')' ) return true;
  else if( c1 == '[' && c2 == ']' ) return true;
  else if( c1 == '{' && c2 == '}' ) return true;
  else
    return false;
}

int findClose( const QString& str , int pos )
{
  int depth = 0;
  QList<QChar> st;
  QChar last = ' ';

  for( int a = pos; a < (int)str.length(); a++)
  {
    switch(str[a].unicode()) {
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
        if( !st.isEmpty() && parenFits(st.front(), str[a]) )
        {
          depth--;
          st.pop_front();
        }
      break;
    case '"':
      last = str[a];
      a++;
      while( a < (int)str.length() && (str[a] != '"' || last == '\\'))
      {
        last = str[a];
        a++;
      }
      continue;
      break;
    }

    last = str[a];

    if( depth == 0 )
    {
      return a;
    }
  }

  return -1;
}

int findCommaOrEnd( const QString& str , int pos, QChar validEnd)
{

  for( int a = pos; a < (int)str.length(); a++)
  {
    switch(str[a].unicode())
    {
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

ParamIterator::ParamIterator( QString parens, QString source, int offset ) : d(new ParamIteratorPrivate)
{
  d->m_source = source;
  d->m_parens = parens;
  
  d->m_cur = offset;
  d->m_curEnd = offset;
  d->m_end = d->m_source.length();

  ///The whole search should be stopped when: A) The end-sign is found on the top-level B) A closing-brace of parameters was found
  int parenBegin = d->m_source.indexOf( parens[ 0 ], offset );
  
  //Search for an interrupting end-sign that comes before the found paren-begin
  int foundEnd = -1;
  if( parens.length() > 2 ) {
    foundEnd = d->m_source.indexOf( parens[2], offset );
    if( foundEnd > parenBegin && parenBegin != -1 )
      foundEnd = -1;
  }
  
  if( foundEnd != -1 ) {
    //We have to stop the search, because we found an interrupting end-sign before the opening-paren
    d->m_prefix = d->m_source.mid( offset, foundEnd - offset );
    
    d->m_curEnd = d->m_end = d->m_cur = foundEnd;
  } else {
    if( parenBegin != -1 ) {
      //We have a valid prefix before an opening-paren. Take the prefix, and start iterating parameters.
      d->m_prefix = d->m_source.mid( offset, parenBegin - offset );
      d->m_cur = parenBegin + 1;
      d->m_curEnd = d->next();
      if( d->m_curEnd == d->m_source.length() ) {
        //The paren was not closed. It might be an identifier like "operator<", so count everything as prefix.
        d->m_prefix = d->m_source.mid(offset);
        d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
      }
    } else {
      //We have neither found an ending-character, nor an opening-paren, so take the whole input and end
      d->m_prefix = d->m_source.mid(offset);
      d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
    }
  }
}

ParamIterator& ParamIterator::operator ++()
{
  if( d->m_source[d->m_curEnd] == d->m_parens[1] ) {
    //We have reached the end-paren. Stop iterating.
    d->m_cur = d->m_end = d->m_curEnd + 1;
  } else {
    //Iterate on through parameters
    d->m_cur = d->m_curEnd + 1;
    if ( d->m_cur < ( int ) d->m_source.length() )
    {
      d->m_curEnd = d->next();
    }
  }
  return *this;
}

QString ParamIterator::operator *()
{
  return d->m_source.mid( d->m_cur, d->m_curEnd - d->m_cur ).trimmed();
}

ParamIterator::operator bool() const
{
  return d->m_cur < ( int ) d->m_end;
}

QString ParamIterator::prefix() const
{
  return d->m_prefix;
}

uint ParamIterator::position() const {
  return (uint)d->m_cur;
}

}
