/***************************************************************************
                          kdevregexp.h  -  description
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 by Roberto Raggi
    email                : roberto@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVREGEXP_H
#define KDEVREGEXP_H

#include <qstring.h>
#include <qvaluelist.h>

struct KDevRegExpCap{
  KDevRegExpCap(): m_text(QString::null), m_start(0), m_end(0) {}
  KDevRegExpCap( const QString& t, int s, int e ): m_text(t), m_start(s), m_end(e) {}
  KDevRegExpCap( const KDevRegExpCap& source ){
    m_text = source.m_text;
    m_start = source.m_start;
    m_end = source.m_end;
  }

  KDevRegExpCap& operator = ( const KDevRegExpCap& source ){
    m_text = source.m_text;
    m_start = source.m_start;
    m_end = source.m_end;
    return *this;
  }

  bool operator == ( const KDevRegExpCap& p ) const{
    return m_text == p.m_text && m_start == p.m_start && m_end == p.m_end;
  }

  QString text() const { return m_text; }
  int start() const { return m_start; }
  int end() const { return m_end; }

private:
  QString m_text;
  int m_start;
  int m_end;
};


class KDevRegExpData;

/**
  *@author Roberto Raggi
  */
class KDevRegExp {
public:
  KDevRegExp();
  KDevRegExp( const QString& pattern, bool caseSensitive=true);
  KDevRegExp( const KDevRegExp& source );
  ~KDevRegExp();

  KDevRegExp& operator = ( const KDevRegExp& source );

  bool caseSensitive() const;
  void setCaseSensitive( bool );

  QString pattern() const;
  void setPattern( const QString& );

  bool exactMatch( const QString& str ) const;
  int search( const QString& str, int index=0 ) const;
  QValueList<KDevRegExpCap> findAll( const QString&, int index=0 ) const;

  QString cap( int grp=0 ) const;
  int pos( int grp=0 ) const;

private:
  void init();

private:
  KDevRegExpData* d;
};

#endif
