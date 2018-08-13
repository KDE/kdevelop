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

#include "debuglog.h"

#include <language/duchain/stringhelpers.h>
#include <language/duchain/safetycounter.h>

#include <QChar>
#include <QString>

namespace {

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

}

int Utils::expressionAt( const QString& text, int index ) {

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
        }
        --index;
        if ( count == 0 ) {
          //index;
          last = T_PAREN;
          break;
        }
      }
    } else if ( last != T_IDE && ch == '>' && ch2 != QLatin1String("->") ) {
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
    } else if ( ch2 == QLatin1String("::") ) {
      index -= 2;
      last = T_ACCESS;
    } else if ( ch2 == QLatin1String("->") ) {
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

QString Utils::quoteExpression(const QString& expr)
{
    return quote(expr, '"');
}

QString Utils::unquoteExpression(const QString& expr)
{
    return unquote(expr, false);
}

QString Utils::quote(const QString& str, char quoteCh)
{
    QString res = str;
    res.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(quoteCh, QStringLiteral("\\") + quoteCh);
    return quoteCh + res + quoteCh;
}

QString Utils::unquote(const QString &str, bool unescapeUnicode, char quoteCh)
{
    if (str.startsWith(quoteCh) && str.endsWith(quoteCh)) {
        QString res;
        res.reserve(str.length());
        bool esc = false;
        int type = 0;
        QString escSeq;
        escSeq.reserve(4);
        // skip beginning and ending quoteCh, no need for str = str.mid(1, str.length() - 2)
        for (int i = 1; i != str.length() - 1; i++) {
            auto ch = str[i];
            if (esc) {
                switch (ch.unicode()) {
                case '\\':
                    if (type != 0) {
                        escSeq += ch;
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += '\\';
                        res += escSeq;
                        escSeq.clear();
                        esc = false;
                        type = 0;
                    } else {
                        res.append('\\');
                        // escSeq.clear();    // escSeq must be empty.
                        esc = false;
                    }
                    break;
                case 'u':
                case 'x':
                    if (type != 0 || !unescapeUnicode) {
                        escSeq += ch;
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += '\\';
                        res += escSeq;
                        escSeq.clear();
                        esc = false;
                        type = 0;
                    } else {
                        type = ch == 'u' ? 1 : 2;
                    }
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    escSeq += ch;
                    if (type == 0) {
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += '\\';
                        res += escSeq;
                        escSeq.clear();
                        esc = false;
                        type = 0;
                    } else {
                        // \uNNNN
                        // \xNN
                        if (escSeq.length() == (type == 1 ? 4 : 2)) {
                            // no need to handle error, we know for sure escSeq is '[0-9a-fA-F]+'
                            auto code = escSeq.toInt(nullptr, 16);
                            res += QChar(code);
                            escSeq.clear();
                            esc = false;
                            type = 0;
                        }
                    }
                    break;
                default:
                    if (type == 0 && ch == quoteCh) {
                        res += ch;
                    } else {
                        escSeq += ch;
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += '\\';
                        res += escSeq;
                        escSeq.clear();
                    }
                    esc = false;
                    type = 0;
                    break;
                }
            } else {
                if (ch == '\\') {
                    esc = true;
                    continue;
                }
                res += ch;
            }
        }
        return res;
    } else {
        return str;
    }
}
