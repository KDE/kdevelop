
/***************************************************************************
k   copyright            : (C) 2006 by David Nolden
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

#include "stringhelpers.h"
#include "safetycounter.h"

namespace StringHelpers
{

void clearStr( QString& str, int start, int end ) {
  for( int a = start; a < end; a++) str[a] = ' ';
}

bool isValidIdentifierSign( const QChar& c ) {
	if( c.isLetter() || c.isDigit() || c == '_' ) return true;
	else return false;
}


QString clearComments( QString str ) {
  if( str.isEmpty() ) return "";
  
  SafetyCounter s( 1000 );
  int lastPos = 0;
  int pos;
  int len = str.length();
  while( (pos = str.find( "/*", lastPos )) != -1 ) {
    if( !s ) return str;
    int i = str.find( "*/", pos );
    if( i != -1 && i <= len - 2 ) {
      clearStr( str, pos, i+2 );
      lastPos = i+2;
      if( lastPos == len ) break;
    } else {
      break;
    }
  }
  
  lastPos = 0;
  while( (pos = str.find( "//", lastPos )) != -1 ) {
    if( !s ) return str;
    int i = str.find( "\n", pos );
    if( i != -1 && i <= len - 1 ) {
      clearStr( str, pos, i+1 );
      lastPos = i+1;
    } else {
      clearStr( str, pos, len );
      break;
    }
  }
  
  return str;
}

QString cutTemplateParams( QString str ) {
  int p;
  if( (p = str.find('<') ) != -1) {
    return str.left( p );
  }
  
  return str.stripWhiteSpace().replace('*',"");
}

QPair<QString, QString> splitTemplateParams( QString str ) {
  QPair<QString, QString> ret;
  int p;
  if( (p = str.find('<') ) != -1) {
    ret.first = str.left( p ).stripWhiteSpace();
    ret.second = str.mid( p ).stripWhiteSpace();
  } else {
    ret.first = str.stripWhiteSpace();
  }
  
  return ret;
}

bool parenFits( QChar c1, QChar c2 ) {
  if( c1 == '<' && c2 == '>' ) return true;
  else if( c1 == '(' && c2 == ')' ) return true;
  else if( c1 == '[' && c2 == ']' ) return true;
  else if( c1 == '{' && c2 == '}' ) return true;
  else
    return false;
}

bool isParen( QChar c1 ) {
  if( c1 == '<' || c1 == '>' ) return true;
  else if( c1 == '(' || c1 == ')' ) return true;
  else if( c1 == '[' || c1 == ']' ) return true;
  else if( c1 == '{' || c1 == '}' ) return true;
  else
    return false;
}

bool isTypeParen( QChar c1 ) {
  if( c1 == '<' || c1 == '>' ) return true;
  else
    return false;
}

bool isTypeOpenParen( QChar c1 ) {
  if( c1 == '<' ) return true;
  else
    return false;
}

bool isTypeCloseParen( QChar c1 ) {
  if( c1 == '>' ) return true;
  else
    return false;
}

bool isLeftParen( QChar c1 ) {
  if( c1 == '<' ) return true;
  else if( c1 == '(' ) return true;
  else if( c1 == '[' ) return true;
  else if( c1 == '{' ) return true;
  else
    return false;
}

int findClose( const QString& str , int pos ) {
  int depth = 0;
  QValueList<QChar> st;
  QChar last = ' ';
  
  for( int a = pos; a < (int)str.length(); a++) {
    switch(str[a]) {
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
        if( !st.isEmpty() && parenFits(st.front(), str[a]) ) {
          depth--;
          st.pop_front();
        }
      break;
    case '"':
      last = str[a];
      a++;
      while( a < (int)str.length() && (str[a] != '"' || last == '\\')) {
        last = str[a];
        a++;
      }
      continue;
      break;
    }
    
    last = str[a];
    
    if( depth == 0 ) {
      return a;
    }
  }
  
  return -1;
}

QString tagType( const Tag& tag )
{
  if ( tag.hasAttribute( "t" ) )
  {
    QString type = tag.attribute( "t" ).toString();
    return type;
  }
  else if ( tag.kind() == Tag::Kind_Class || tag.kind() == Tag::Kind_Namespace )
  {
    QStringList l = tag.scope();
    l << tag.name();
    return l.join("::");
  }
  return QString();
}

int findCommaOrEnd( const QString& str , int pos, QChar validEnd) {
  
  for( int a = pos; a < (int)str.length(); a++) {
    switch(str[a]) {
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

int countExtract( QChar c, const QString& str ) {
  int ret  = 0;
  for( int a = 0; a < (int)str.length(); a++) {
    if( str[a] == c ) ++ret;
    switch( str[a] ) {
    case '(':
      case '[':
        case '{':
        case '<':
        a = findClose( str, a );
    }
  }
  return ret;
}

QString templateParamFromString( int num, QString str ) {
  if( str.endsWith("::") ) str.truncate( str.length() - 2 );
  int begin = str.find('<');
  int end = str.findRev('>');
  
  if(begin == -1 || end == -1) return "";
  
  begin++;
  
  for(int a = 0; a < num; a++) {
    begin = findCommaOrEnd( str, begin );
    if( begin == (int)str.length() ) return "";
    begin++;
  }
  end = findCommaOrEnd( str, begin );
  
  if( end == (int)str.length() ) return "";
  
  return str.mid( begin, end - begin ).stripWhiteSpace();
}

QStringList splitType( QString str ) {
  QStringList ret;
  int currentStart = 0;
  bool was = false;
  for( int a = 0; a < (int)str.length(); ++a ) {
    if( isLeftParen( str[a] ) ) {
      a = findClose( str, a );
      if( a == -1 ) {
      CompletionDebug::dbg() << "misformatted type: " << str << endl;
        return ret;
      }
      was = false;
    } else {
      if( str[a] == ':' ) {
        if( was ) {
          if( currentStart < a - 1 ) 
            ret << str.mid( currentStart, (a - 1) - currentStart ).stripWhiteSpace();
          currentStart = a + 1;
        }
        was = true;
      } else {
        was = false;
      }
    }
  }
  if( currentStart < (int)str.length() ) 
    ret << str.mid( currentStart, str.length() - currentStart ).stripWhiteSpace();
  return ret;
}

QString stringMult( int count, QString str ){
	QString ret;
	for( int a = 0; a < count; a++ ) ret += str;
	return ret;
}

}


// kate: indent-mode csands; tab-width 4;
