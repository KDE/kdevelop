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

#include <language/duchain/stringhelpers.h>
#include <language/duchain/safetycounter.h>
#include <QString>
#include <QChar>
#include <QStringList>

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

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN, T_TEMP };
///FIXME: move the bracket detectors into a function
int expressionAt( const QString& _text, int index ) {
  QString text = KDevelop::clearStrings(_text);
  
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
      //int parenStart = index;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '(' ) {
          ++count;
        } else if ( ch == ')' ) {
          --count;
        }
        --index;
        if ( count == 0 ) {
          last = T_PAREN;
          break;
        }
      }
      /*if (index == 0 && count != 0) {
        index = parenStart + 1;
        break; //No expression behind unmatched paren
      }*/
    } else if ( last != T_IDE && ch == '>' && ch2 != "->" ) {
      int count = 0;
      int tempStart = index;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '<' ) {
          ++count;
        } else if ( ch == '>' ) {
          --count;
        }
        --index;
        if ( count == 0 ) {
          last = T_TEMP;
          break;
        }
      }
      if (index == 0 && count != 0) {
        index = tempStart + 1;
        break; //No expression behind unmatched template bracket (could be '>' operator)
      }
    } else if ( ch == ']' ) {
      int count = 0;
      int bracketStart = index;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '[' ) {
          ++count;
        } else if ( ch == ']' ) {
          --count;
        }
        --index;
        if ( count == 0 ) {
          last = T_BRACKET;
          break;
        }
      }
      if (index == 0 && count != 0) {
        index = bracketStart + 1;
        break; //No expression behind unmatched bracket
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


}
