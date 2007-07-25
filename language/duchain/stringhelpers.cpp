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

ParamIterator::ParamIterator( QString parens, QString source ) : d(new ParamIteratorPrivate)
{
  d->m_source = source;
  d->m_parens = parens;
  d->m_cur = 0;
  d->m_curEnd = 0;
  int begin = d->m_source.indexOf( d->m_parens[ 0 ] );
  int end = d->m_source.lastIndexOf( d->m_parens[ 1 ] );

  if( begin != -1 )
    d->m_prefix = d->m_source.left( begin );
  else
    d->m_prefix = d->m_source;

  if ( begin == -1 || end == -1 && end - begin > 1 )
    d->m_cur = d->m_source.length();
  else
  {
    d->m_source = source.mid( begin + 1, end - begin );
    d->m_curEnd = d->next();
  }
}

ParamIterator& ParamIterator::operator ++()
{
  d->m_cur = d->m_curEnd + 1;
  if ( d->m_cur < ( int ) d->m_source.length() )
  {
    d->m_curEnd = d->next();
  }
  return *this;
}

QString ParamIterator::operator *()
{
  return d->m_source.mid( d->m_cur, d->m_curEnd - d->m_cur ).trimmed();
}

ParamIterator::operator bool() const
{
  return d->m_cur < ( int ) d->m_source.length();
}

QString ParamIterator::prefix() const
{
  return d->m_prefix;
}

}
