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
#include <QChar>
#include <QStringList>
#include "safetycounter.h"

namespace Utils {

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

void fillString( QString& str, int start, int end, QChar replacement ) {
  for( int a = start; a < end; a++) str[a] = replacement;
}

QString clearComments( QString str, QChar replacement ) {
  if( str.isEmpty() ) return "";

  SafetyCounter s( 1000 );
  int lastPos = 0;
  int pos;
  int len = str.length();
  while( (pos = str.indexOf( "/*", lastPos )) != -1 ) {
    if( !s ) return str;
    int i = str.indexOf( "*/", pos );
    if( i != -1 && i <= len - 2 ) {
      fillString( str, pos, i+2, replacement );
      lastPos = i+2;
      if( lastPos == len ) break;
    } else {
      break;
    }
  }

  lastPos = 0;
  while( (pos = str.indexOf( "//", lastPos )) != -1 ) {
    if( !s ) return str;
    int i = str.indexOf( "\n", pos );
    if( i != -1 && i <= len - 1 ) {
      fillString( str, pos, i+1, replacement );
      lastPos = i+1;
    } else {
      fillString( str, pos, len, replacement );
      break;
    }
  }

  return str;
}

QString reduceWhiteSpace(QString str) {
  str = str.trimmed();
  QString ret;

  QChar spaceChar = ' ';

  bool hadSpace = false;
  for( int a = 0; a < str.length(); a++ ) {
    if( str[a].isSpace() ) {
      hadSpace = true;
    } else {
      if( hadSpace ) {
        hadSpace = false;
        ret += spaceChar;
      }
      ret += str[a];
    }
  }

  return ret;
}

QString stripFinalWhitespace(QString str) {

  for( int a = str.length() - 1; a >= 0; --a ) {
    if( !str[a].isSpace() )
      return str.left( a+1 );
    }

  return QString();
}

QString clearStrings( QString str, QChar /*replacement*/ ) {
  ///@todo implement: Replace all strings with the given replacement-character
  return str;
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN, T_TEMP };

int expressionAt( const QString& text, int index ) {

  /* C++ style comments present issues with finding the expr so I'm
  	matching for them and replacing them with empty C style comments
  	of the same length for purposes of finding the expr. */

  if( index == 0 )
    return 0;

  int last = T_UNKNOWN;
  int start = index;
  --index;

  while ( index > 0 ) {
    while ( index > 0 && text[ index ].isSpace() ) {
      --index;
    }

    QChar ch = text[ index ];
    QString ch2 = text.mid( index - 1, 2 );
    if ( ( last != T_IDE ) && ( ch.isLetterOrNumber() || ch == '_' ) ) {
      while ( index > 0 && ( text[ index ].isLetterOrNumber() || text[ index ] == '_' ) ) {
        --index;
      }
      last = T_IDE;
    } else if ( last != T_IDE && ch == ')' ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '(' ) {
          ++count;
        } else if ( ch == ')' ) {
          --count;
        } else if ( count == 0 ) {
          //index;
          last = T_PAREN;
          break;
        }
        --index;
      }
    } else if ( last != T_IDE && ch == '>' && ch2 != "->" ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '<' ) {
          ++count;
        } else if ( ch == '>' ) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_TEMP;
          break;
        }
        --index;
      }
    } else if ( ch == ']' ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '[' ) {
          ++count;
        } else if ( ch == ']' ) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_BRACKET;
          break;
        }
        --index;
      }
    } else if ( ch == '.' ) {
      --index;
      last = T_ACCESS;
    } else if ( ch2 == "::" ) {
      index -= 2;
      last = T_ACCESS;
    } else if ( ch2 == "->" ) {
      index -= 2;
      last = T_ACCESS;
    } else {
      if ( start > index ) {
        ++index;
      }
      last = T_UNKNOWN;
      break;
    }
  }

  ///If we're at the first item, the above algorithm cannot be used safely,
  ///so just determine whether the sign is valid for the beginning of an expression, if it isn't reject it.
  if ( index == 0 && start > index && !( text[ index ].isLetterOrNumber() || text[ index ] == '_' || text[ index ] == ':' ) ) {
    ++index;
  }

  return index;
}

QString reverse( const QString& str ) {
  QString ret;
  int len = str.length();
  for( int a = len-1; a >= 0; --a ) {
    switch(str[a].toAscii()) {
    case '(':
      ret += ')';
      continue;
    case '[':
      ret += ']';
      continue;
    case '{':
      ret += '}';
      continue;
    case '<':
      ret += '>';
      continue;
    case ')':
      ret += '(';
      continue;
    case ']':
      ret += '[';
      continue;
    case '}':
      ret += '{';
      continue;
    case '>':
      ret += '<';
      continue;
    default:
      ret += str[a];
      continue;
    }
  }
  return ret;
}

void skipFunctionArguments(QString str, QStringList& skippedArguments, int& argumentsStart ) {
  QString reversed = reverse( str.left(argumentsStart) );
  //Now we should decrease argumentStart at the end by the count of steps we go right until we find the beginning of the function
  SafetyCounter s( 1000 );

  int pos = 0;
  int len = reversed.length();
  //we are searching for an opening-brace, but the reversion has also reversed the brace
  while( pos != len && s ) {
    int lastPos = pos;
    pos = KDevelop::findCommaOrEnd( reversed, pos )  ;
    if( pos > lastPos ) {
      QString arg = reverse( reversed.mid(lastPos, pos-lastPos) ).trimmed();
      if( !arg.isEmpty() )
        skippedArguments.push_front( arg ); //We are processing the reversed reverseding, so push to front
    }
    if( reversed[pos] == ')' )
      break;
    else
      ++pos;
  }

  if( !s ) {
    kDebug(9007) << "skipFunctionArguments: Safety-counter triggered";
  }

  argumentsStart -= pos;
}

}
