/***************************************************************************
                          kdevregexp.cpp  -  description
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 by Roberto Raggi
    email                : raggi@cli.di.unipi.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevregexp.h"
#include <kregexp.h>
#include <kdebug.h>

struct KDevRegExpData{
  KRegExp* rx;
  QString pattern;
  bool caseSensitive;

  KDevRegExpData()
    : rx( 0 ),
      pattern( QString::null ),
      caseSensitive( false )
      {}

  ~KDevRegExpData()
    { delete( rx ); rx=0;}
};

KDevRegExp::KDevRegExp()
{
  d = new KDevRegExpData();
}


KDevRegExp::KDevRegExp( const QString& pattern, bool caseSensitive )
{
  d = new KDevRegExpData();
  d->pattern = pattern;
  d->caseSensitive = caseSensitive;
  init();
}

KDevRegExp::KDevRegExp( const KDevRegExp& source )
{
  d = new KDevRegExpData();
  d->pattern = source.pattern();
  d->caseSensitive = source.caseSensitive();
  init();
}

KDevRegExp& KDevRegExp::operator = ( const KDevRegExp& source )
{
  d->pattern = source.pattern();
  d->caseSensitive = source.caseSensitive();
  init();
  return *this;
}


KDevRegExp::~KDevRegExp()
{
  delete( d );
  d=0;
}

void KDevRegExp::init()
{
  delete( d->rx );
  d->rx = new KRegExp( pattern(), caseSensitive() ? "" : "i" );
}

bool KDevRegExp::caseSensitive() const
{
  return d->caseSensitive;
}

void KDevRegExp::setCaseSensitive( bool sensitive )
{
  if( d->caseSensitive != sensitive ){
    d->caseSensitive = sensitive;
    init();
  }
}

QString KDevRegExp::pattern() const
{
  return d->pattern;
}

void KDevRegExp::setPattern( const QString& s )
{
  if( d->pattern != s ){
    d->pattern = s;
    init();
  }
}

bool KDevRegExp::exactMatch( const QString& str ) const
{
  if( d->rx->match(str) ){
    if( d->rx->groupStart(0) == 0 && d->rx->groupEnd(0) == (int)str.length() )
      return true;
  }
  return false;
}

int KDevRegExp::search( const QString& str, int index ) const
{
  QString s = str.mid( index );
  if( d->rx->match(s) ){
    return index + d->rx->groupStart( 0 );
  }
  return -1;
}

QString KDevRegExp::cap( int grp ) const
{
  const char* s = d->rx->group( grp );
  if( s ){
    return QString::fromUtf8( s );
  }
  return QString::null;
}

int KDevRegExp::pos( int grp ) const
{
  return d->rx->groupStart( grp );
}

QValueList<KDevRegExpCap> KDevRegExp::findAll( const QString& str, int index ) const
{
  QValueList<KDevRegExpCap> l;

  while( index < (int)str.length() ){
    int idx = search( str, index );
    if( idx == -1 ){
      break;
    }

    l << KDevRegExpCap( cap(), idx, cap().length() );
    index = idx + cap().length();
  }
  return l;
}
