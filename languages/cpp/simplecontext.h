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

#ifndef SIMPLECONTEXT_H
#define SIMPLECONTEXT_H

#include <qvaluelist.h>
#include <qstringlist.h>

#include "declarationinfo.h"
#include "typedesc.h"
#include "simpletype.h"

extern SimpleType getGlobal(SimpleType t);

class SimpleVariable
{
public:
  SimpleVariable()
  {
    startLine = endLine = startCol = endCol = 0;
  }
  
  SimpleVariable( const SimpleVariable& source )
    : name( source.name ),
    comment(source.comment),
    startLine(source.startLine),
    startCol(source.startCol),
    endLine(source.endLine),
    endCol(source.endCol),
    type( source.type ),
    ptrList( source.ptrList )
  {}
  ~SimpleVariable()
  {}
  
  SimpleVariable& operator = ( SimpleVariable& source )
  {
    name = source.name;
    type = source.type;
    ptrList = source.ptrList;
    comment = source.comment;
    startLine = source.startLine;
    startCol = source.startCol;
    endLine = source.endLine;
    endCol = source.endCol;
    return *this;
  }
  
  QString name;
  QString comment;
  int startLine, startCol;
  int endLine, endCol;
  TypeDesc type;
  QStringList ptrList;
  
  DeclarationInfo toDeclarationInfo( QString activeFileName ) {
    DeclarationInfo decl;
    decl.name = name;
    decl.file = activeFileName;
    decl.comment = comment;
    decl.startLine = startLine;decl.startCol = startCol;
    decl.endLine = endLine; decl.endCol = endCol;
    return decl;
  }
};



class SimpleContext
{
  public:
    SimpleContext( SimpleType container = SimpleType(), SimpleContext* prev = 0 )
  : m_prev( prev ), m_container( container )
    {
      (*m_container); ///Make the type physically create itself
    }

    virtual ~SimpleContext()
    {
      if ( m_prev )
      {
        delete( m_prev );
        m_prev = 0;
      }
    }

    SimpleContext* prev() const
    {
      return m_prev;
    }

    void attach( SimpleContext* ctx )
    {
      m_prev = ctx;
    }

    void detach()
    {
      m_prev = 0;
    }

    const QValueList<SimpleVariable>& vars() const
    {
      return m_vars;
    }

    void add( const SimpleVariable& v )
    {
      m_vars.append( v );
    }
  
    void add( const QValueList<SimpleVariable>& vars )
    {
      m_vars += vars;
    }

    //First the new name, aka "" for real imports, second the name to be imported
    void addImport( const QPair<QString, QString>& import ) {
      m_imports << import;
    }

    //Key the new name, aka "" for real imports, second the name to be imported
    QValueList<QPair<QString, QString> > imports() {
      return m_imports;
    }
  
    void offset( int lineOffset, int colOffset );

  SimpleVariable findVariable( const QString& varname );
  
    SimpleType global() {
      return getGlobal( container() );
    }
  
    SimpleType& container() {
      return m_container;
    }
  
    void setContainer( SimpleType cnt ) {
      m_container = cnt;
      (*m_container); ///make the type physically create itself
    }

  private:
    QValueList<SimpleVariable> m_vars;
    QValueList<QPair<QString, QString> > m_imports;
    SimpleContext* m_prev;
    SimpleType m_container;
};

#endif
// kate: indent-mode csands; tab-width 4;
