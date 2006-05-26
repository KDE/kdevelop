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
#ifndef EXPRESSIONINFO_H
#define EXPRESSIONINFO_H

#include "stringhelpers.h"

using namespace StringHelpers;

struct ExpressionInfo {
  private:
    QString m_expr;
  
  public:
  
    enum Type {
      InvalidExpression = 0,
      NormalExpression = 1,
      TypeExpression = 2
    };
  
    Type t;
    int start, end;
  
    QString expr() {
      return m_expr;
    }
  
    void setExpr( const QString& str ) {
      m_expr = clearComments( str );
    }
  
    ExpressionInfo( QString str ) : t(  NormalExpression ), start( 0 ), end( str.length() ) {
      setExpr( str );
    
    }
  
    ExpressionInfo() : t( InvalidExpression ), start(0), end(0) {
    }
  
    operator bool() {
      return t != InvalidExpression && !m_expr.isEmpty();
    }
  
    bool isTypeExpression() {
      return t == TypeExpression && !m_expr.isEmpty();
    }
  
    bool canBeTypeExpression() {
      return t & TypeExpression && !m_expr.isEmpty();
    }
  
    bool isNormalExpression() {
      return t == NormalExpression && !m_expr.isEmpty();
    }
  
    bool canBeNormalExpression() {
      return t & NormalExpression && !m_expr.isEmpty();
    }
  
    QString typeAsString() {
      QString res ;
      if( t & NormalExpression )
        res += "NormalExpression, ";
      if( t & TypeExpression )
        res += "TypeExpression, ";
      if( t == InvalidExpression )
        res += "InvalidExpression, ";
      if( !res.isEmpty() ) {
        res = res.left( res.length() - 2 );
      } else {
        res = "Unknown";
      }
      return res;
    }
};



#endif
// kate: indent-mode csands; tab-width 4;
