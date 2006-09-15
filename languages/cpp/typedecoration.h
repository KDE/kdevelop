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
	TypeDecoration() {
	}

	///Removes the decoration from the given string
	TypeDecoration( QString& str ) {
      init( str );
    }

    ~TypeDecoration() {}

	///Removes the decoration from the assigned
	TypeDecoration& operator = ( QString& str ) {
	  clear();
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
    void init( QString& str ) {
      str = str.stripWhiteSpace();

      static const QString cnst = "const";
      static const QString ref = "&";
	    if ( str.startsWith( cnst ) ) {
		    str.remove( 0, cnst.length() );
		    if( str.isEmpty() || ( !str[0].isLetterOrNumber() && str[0] != '_' ) ) {
				m_decoration_front += cnst + " ";
			    str = str.stripWhiteSpace();
		    } else {
			    str = cnst + str; ///The const was not alone
		    }
	    }

	  if( str.endsWith( cnst ) ) {
		  str.remove( str.length() - cnst.length(), cnst.length() );
		  if( str.isEmpty() || ( !str[str.length()-1].isLetterOrNumber() && str[str.length()-1] != '_' ) ) {
			  m_decoration_back = (m_decoration_back + " " + cnst);
			  str = str.stripWhiteSpace();
		  } else {
			  str = str + cnst; ///The const was not alone
		  }
	  }
	    
	  if ( str.endsWith( ref ) ) {
	      m_decoration_back = ref + m_decoration_back;
		  str = str.remove( str.length() - ref.length(), ref.length() ).stripWhiteSpace();
		  
		  if( str.endsWith( cnst ) ) {
			  str.remove( str.length() - cnst.length(), cnst.length() );
			  if( str.isEmpty() || ( !str[str.length()-1].isLetterOrNumber() && str[str.length()-1] != '_' ) ) {
				  m_decoration_back = m_decoration_back + " " + cnst;
				  str = str.stripWhiteSpace();
			  } else {
				  str = str + cnst; ///The const was not alone
			  }
		  }
	  }
    }

    QString m_decoration_front, m_decoration_back;
};

#endif 
// kate: indent-mode csands; tab-width 4;

