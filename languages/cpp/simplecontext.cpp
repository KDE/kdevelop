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

#include "simplecontext.h"
#include "safetycounter.h"

SimpleType getGlobal( SimpleType t ) {
  SimpleType global = t;
  SafetyCounter s( 50 );
  while( !global.scope().isEmpty() && s ) {
    if( !s ) { kdDebug( 9007 ) << "error" << endl; break; }
    global = global->parent();
  }
  
  return global;
}

void SimpleContext::offset( int lineOffset, int colOffset ) {
  for( QValueList<SimpleVariable>::iterator it = m_vars.begin(); it != m_vars.end(); ++it ) {
    if( (*it).endLine != (*it).startLine || (*it).endCol != (*it).startCol) {
      if( (*it).startLine == 0 ) {
        (*it).startCol += colOffset;
      }
      if( (*it).endLine == 0 ) {
        (*it).endCol += colOffset;
      }
      (*it).startLine += lineOffset;
      (*it).endLine += lineOffset;
    }
  }
}

SimpleVariable SimpleContext::findVariable( const QString& varname )
{
  SimpleContext * ctx = this;
  while ( ctx )
  {
    const QValueList<SimpleVariable>& vars = ctx->vars();
    for ( int i = vars.count() - 1; i >= 0; --i )
    {
      SimpleVariable v = vars[ i ];
      if ( v.name == varname )
        return v;
    }
    ctx = ctx->prev();
  }
  return SimpleVariable();
}
 
// kate: indent-mode csands; tab-width 4;
