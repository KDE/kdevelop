/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "stringhelpers.h"

#include "debuglog.h"

#include <QChar>
#include <QString>

namespace {

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN, T_TEMP };

}

int Utils::expressionAt(QStringView text, int index)
{
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
    const auto ch2 = index > 0 ? text.sliced(index - 1, 2) : QStringView{};
    if ((last != T_IDE) && (ch.isLetterOrNumber() || ch == QLatin1Char('_'))) {
      while (index > 0 && (text[index].isLetterOrNumber() || text[index] == QLatin1Char('_'))) {
        --index;
      }
      last = T_IDE;
    } else if (last != T_IDE && ch == QLatin1Char(')')) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if (ch == QLatin1Char('(')) {
          ++count;
        } else if (ch == QLatin1Char(')')) {
          --count;
        }
        --index;
        if ( count == 0 ) {
          //index;
          last = T_PAREN;
          break;
        }
      }
    } else if (last != T_IDE && ch == QLatin1Char('>') && ch2 != QLatin1String("->")) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if (ch == QLatin1Char('<')) {
          ++count;
        } else if (ch == QLatin1Char('>')) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_TEMP;
          break;
        }
        --index;
      }
    } else if (ch == QLatin1Char(']')) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if (ch == QLatin1Char('[')) {
          ++count;
        } else if (ch == QLatin1Char(']')) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_BRACKET;
          break;
        }
        --index;
      }
    } else if (ch == QLatin1Char('.')) {
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
  if (index == 0 && start > index &&
      !(text[index].isLetterOrNumber() || text[index] == QLatin1Char('_') || text[index] == QLatin1Char(':'))) {
    ++index;
  }

  return index;
}

QString Utils::quoteExpression(const QString& expr)
{
    return quote(expr, QLatin1Char('"'));
}

QString Utils::unquoteExpression(const QString& expr)
{
    return unquote(expr, false);
}

QString Utils::quote(const QString& str, QChar quoteCh)
{
    QString res = str;
    res.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(quoteCh, QLatin1Char('\\') + quoteCh);
    return quoteCh + res + quoteCh;
}

QString Utils::unquote(const QString& str, bool unescapeUnicode, QChar quoteCh)
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
                        res += QLatin1Char('\\');
                        res += escSeq;
                        escSeq.clear();
                        esc = false;
                        type = 0;
                    } else {
                        res.append(QLatin1Char('\\'));
                        // escSeq.clear();    // escSeq must be empty.
                        esc = false;
                    }
                    break;
                case 'u':
                case 'x':
                    if (type != 0 || !unescapeUnicode) {
                        escSeq += ch;
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += QLatin1Char('\\');
                        res += escSeq;
                        escSeq.clear();
                        esc = false;
                        type = 0;
                    } else {
                        type = ch == QLatin1Char('u') ? 1 : 2;
                    }
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    escSeq += ch;
                    if (type == 0) {
                        qCDebug(DEBUGGERCOMMON) << "Unrecognized escape sequence:" << escSeq;
                        res += QLatin1Char('\\');
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
                        res += QLatin1Char('\\');
                        res += escSeq;
                        escSeq.clear();
                    }
                    esc = false;
                    type = 0;
                    break;
                }
            } else {
                if (ch == QLatin1Char('\\')) {
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
