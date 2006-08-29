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

#ifndef __TYPEDECORATION_H__
#define __TYPEDECORATION_H__


class TypeDecoration {

  public:
    TypeDecoration( const QString& str = "" ) {
      init( str );
    }

    ~TypeDecoration() {}

    TypeDecoration& operator = ( QString str ) {
      init( str );
      return *this;
    }

    QString apply( const QString& str ) const {
      QString ret = str;
      if ( !ret.startsWith( m_decoration_front ) )
        ret = m_decoration_front + ret;

      if ( !ret.endsWith( m_decoration_back ) )
        ret = ret + m_decoration_back;
      return ret;
    }

    void operator += ( const TypeDecoration& rhs ) {
      if ( !m_decoration_front.contains( rhs.m_decoration_front ) )
        m_decoration_front += rhs.m_decoration_front;
      if ( !m_decoration_back.contains( rhs.m_decoration_back ) )
        m_decoration_back += rhs.m_decoration_back;
    }

    void clear() {
      m_decoration_front = QString();
      m_decoration_back = QString();
    }

  private:
    void init( QString str ) {
      str = str.stripWhiteSpace();

      static QString cnst = "const ";
      static QString ref = "&";
      if ( str.startsWith( cnst ) )
        m_decoration_front = cnst;
      else
        m_decoration_front = QString();

      if ( str.endsWith( ref ) )
        m_decoration_back = ref;
      else
        m_decoration_back = QString();

    }

    QString m_decoration_front, m_decoration_back;
};

#endif 
// kate: indent-mode csands; tab-width 4;

