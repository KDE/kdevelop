
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

#ifndef __STRINGHELPERS_H__
#define __STRINGHELPERS_H__


#include "completiondebug.h"
#include "codeinformationrepository.h"


namespace StringHelpers {

void clearStr( QString& str, int start, int end );

///Fills all comments with whitespaces
QString clearComments( QString str );

QString cutTemplateParams( QString str );

QPair<QString, QString> splitTemplateParams( QString str );

bool parenFits( QChar c1, QChar c2 );

bool isParen( QChar c1 );

bool isTypeParen( QChar c1 );

bool isTypeOpenParen( QChar c1 );

bool isTypeCloseParen( QChar c1 );


bool isLeftParen( QChar c1 );



/*only from left to right
searches a fitting closing sign ( a ')' for a '(', ']' for '['
ignores quoted text
comments are currently not allowed */
int findClose( const QString& str , int pos );


QString tagType( const Tag& tag );


int findCommaOrEnd( const QString& str , int pos, QChar validEnd = ' ' );


int countExtract( QChar c, const QString& str );


QString templateParamFromString( int num, QString str );

QStringList splitType( QString str ) ;

class ParamIterator{
private:
 QString m_source;
 QString m_parens;
 int m_cur;
 int m_curEnd;
 
 int next() {
  return findCommaOrEnd( m_source, m_cur,  m_parens[1] );
 }
public:
ParamIterator( QString parens, QString source ) : m_source( source ), m_parens( parens ), m_cur( 0 ), m_curEnd ( 0 ) {
 int begin = m_source.find(m_parens[0]);
 int end = m_source.findRev(m_parens[1]);
 
 if(begin == -1 || end == -1 && end - begin > 1)
  m_cur = m_source.length();
 else {
  m_source = source.mid( begin+1, end - begin );
  m_curEnd = next();
 }
}
 
 ParamIterator& operator ++() {
  m_cur = m_curEnd + 1;
  if( m_cur < (int)m_source.length() ) {
   m_curEnd = next();
  }
  return *this;
 }
 
 QString operator *() {
  return m_source.mid( m_cur, m_curEnd - m_cur ).stripWhiteSpace();
 }
 
 operator bool() {
  return m_cur < (int)m_source.length();
 }
 
};


};
#endif 
// kate: indent-mode csands; tab-width 4;
