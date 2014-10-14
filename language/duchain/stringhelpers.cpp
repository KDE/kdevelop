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
#include "safetycounter.h"
#include "util/debug.h"

#include <QString>
#include <QStringList>

namespace {

template<typename T>
int strip_impl(const T& str, T& from)
{
  if( str.isEmpty() )
    return 0;

  int i = 0;
  int ip = 0;
  int s = from.length();

  for( int a = 0; a < s; a++ ) {
      if( QChar(from[a]).isSpace() ) {
          continue;
      } else {
          if( from[a] == str[i] ) {
              i++;
              ip = a+1;
              if( i == (int)str.length() ) break;
          } else {
              break;
          }
      }
  }

  if( ip ) {
    from = from.mid( ip );
  }
  return s - from.length();
}

template<typename T>
int rStrip_impl(const T& str, T& from)
{
  if( str.isEmpty() )
    return 0;

  int i = 0;
  int ip = from.length();
  int s = from.length();

  for( int a = s-1; a >= 0; a-- ) {
      if( QChar( from[a] ).isSpace() ) { ///@todo Check whether this can cause problems in utf-8, as only one real character is treated!
          continue;
      } else {
          if( from[a] == str[i] ) {
              i++;
              ip = a;
              if( i == (int)str.length() ) break;
          } else {
              break;
          }
      }
  }

  if( ip != (int)from.length() ) {
    from = from.left( ip );
  }
  return s - from.length();
}

template<typename T>
T formatComment_impl(const T& comment)
{
  T ret;

  QList<T> lines = comment.split( '\n' );

  if ( !lines.isEmpty() ) {

    auto it = lines.begin();
    auto eit = lines.end();

    // remove common leading chars from the beginning of lines
    for( ; it != eit; ++it ) {
        strip_impl<T>( "///", *it );
        strip_impl<T>( "//", *it );
        strip_impl<T>( "**", *it );
        rStrip_impl<T>( "/**", *it );
    }

    foreach(const T& line, lines) {
      if(!ret.isEmpty())
        ret += '\n';
      ret += line;
    }
  }

  return ret.trimmed();
}

}

namespace KDevelop
{

class ParamIteratorPrivate
{
public:
  QString m_prefix;
  QString m_source;
  QString m_parens;
  int m_cur;
  int m_curEnd;
  int m_end;

  int next() const
  {
    return findCommaOrEnd( m_source, m_cur, m_parens[ 1 ] );
  }
};


bool parenFits( QChar c1, QChar c2 )
{
  if( c1 == '<' && c2 == '>' ) return true;
  else if( c1 == '(' && c2 == ')' ) return true;
  else if( c1 == '[' && c2 == ']' ) return true;
  else if( c1 == '{' && c2 == '}' ) return true;
  else
    return false;
}

int findClose( const QString& str , int pos )
{
  int depth = 0;
  QList<QChar> st;
  QChar last = ' ';

  for( int a = pos; a < (int)str.length(); a++)
  {
    switch(str[a].unicode()) {
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
        if( !st.isEmpty() && parenFits(st.front(), str[a]) )
        {
          depth--;
          st.pop_front();
        }
      break;
    case '"':
      last = str[a];
      a++;
      while( a < (int)str.length() && (str[a] != '"' || last == '\\'))
      {
        last = str[a];
        a++;
      }
      continue;
      break;
    case '\'':
      last = str[a];
      a++;
      while( a < (int)str.length() && (str[a] != '\'' || last == '\\'))
      {
        last = str[a];
        a++;
      }
      continue;
      break;
    }

    last = str[a];

    if( depth == 0 )
    {
      return a;
    }
  }

  return -1;
}

int findCommaOrEnd( const QString& str , int pos, QChar validEnd)
{

  for( int a = pos; a < (int)str.length(); a++)
  {
    switch(str[a].unicode())
    {
    case '"':
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


QString reverse( const QString& str ) {
  QString ret;
  int len = str.length();
  for( int a = len-1; a >= 0; --a ) {
    switch(str[a].unicode()) {
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

///@todo this hackery sucks
QString escapeForBracketMatching(QString str) {
  str.replace("<<", "$&");
  str.replace(">>", "$$");
  str.replace("\\\"", "$!");
  str.replace("->", "$?");
  return str;
}

QString escapeFromBracketMatching(QString str) {
  str.replace("$&", "<<");
  str.replace("$$", ">>");
  str.replace("$!", "\\\"");
  str.replace("$?", "->");
  return str;
}

void skipFunctionArguments(QString str, QStringList& skippedArguments, int& argumentsStart ) {
  QString withStrings = escapeForBracketMatching(str);
  str = escapeForBracketMatching(clearStrings(str));

  //Blank out everything that can confuse the bracket-matching algorithm
  QString reversed = reverse( str.left(argumentsStart) );
  QString withStringsReversed = reverse( withStrings.left(argumentsStart) );
  //Now we should decrease argumentStart at the end by the count of steps we go right until we find the beginning of the function
  SafetyCounter s( 1000 );

  int pos = 0;
  int len = reversed.length();
  //we are searching for an opening-brace, but the reversion has also reversed the brace
  while( pos < len && s ) {
    int lastPos = pos;
    pos = KDevelop::findCommaOrEnd( reversed, pos )  ;
    if( pos > lastPos ) {
      QString arg = reverse( withStringsReversed.mid(lastPos, pos-lastPos) ).trimmed();
      if( !arg.isEmpty() )
        skippedArguments.push_front( escapeFromBracketMatching(arg) ); //We are processing the reversed reverseding, so push to front
    }
    if( reversed[pos] == ')' || reversed[pos] == '>' )
      break;
    else
      ++pos;
  }

  if( !s ) {
    qCDebug(LANGUAGE) << "skipFunctionArguments: Safety-counter triggered";
  }

  argumentsStart -= pos;
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

void fillString( QString& str, int start, int end, QChar replacement ) {
  for( int a = start; a < end; a++) str[a] = replacement;
}

QString stripFinalWhitespace(QString str) {

  for( int a = str.length() - 1; a >= 0; --a ) {
    if( !str[a].isSpace() )
      return str.left( a+1 );
    }

  return QString();
}

QString clearComments( QString str, QChar replacement ) {

  QString withoutStrings = clearStrings(str, '$');

  int pos = -1, newlinePos = -1, endCommentPos = -1, nextPos = -1, dest = -1;
  while ( (pos = str.indexOf('/', pos + 1)) != -1 )
  {
    newlinePos = withoutStrings.indexOf('\n', pos);

    if (withoutStrings[pos + 1] == '/')
    {
      //C style comment
      dest = newlinePos == -1 ? str.length() : newlinePos;
      fillString(str, pos, dest, replacement);
      pos = dest;
    }
    else if (withoutStrings[pos + 1] == '*')
    {
      //CPP style comment
      endCommentPos = withoutStrings.indexOf("*/", pos + 2);
      if (endCommentPos != -1)
        endCommentPos += 2;

      dest = endCommentPos == -1 ? str.length() : endCommentPos;
      while (pos < dest)
      {
        nextPos = (dest > newlinePos && newlinePos != -1) ? newlinePos : dest;
        fillString(str, pos, nextPos, replacement);
        pos = nextPos;
        if (pos == newlinePos)
        {
          ++pos; //Keep newlines intact, skip them
          newlinePos = withoutStrings.indexOf('\n', pos + 1);
        }
      }
    }
  }
  return str;
}

QString clearStrings( QString str, QChar replacement ) {
  bool inString = false;
  for(int pos = 0; pos < str.length(); ++pos) {
    //Skip cpp comments
    if(!inString && pos + 1 < str.length() && str[pos] == '/' && str[pos+1] == '*')
    {
      pos += 2;
      while(pos + 1 < str.length()) {
        if (str[pos] == '*' && str[pos + 1] == '/') {
          ++pos;
          break;
        }
        ++pos;
      }
    }
    //Skip cstyle comments
    if(!inString && pos + 1 < str.length() && str[pos] == '/' && str[pos+1] == '/')
    {
      pos += 2;
      while(pos < str.length() && str[pos] != '\n') {
        ++pos;
      }
    }
    //Skip a character a la 'b'
    if(!inString && str[pos] == '\'' && pos + 3 <= str.length())
    {
      //skip the opening '
      str[pos] = replacement;
      ++pos;

      if(str[pos] == '\\')
      {
        //Skip an escape character
        str[pos] = replacement;
        ++pos;
      }

      //Skip the actual character
      str[pos] = replacement;
      ++pos;

      //Skip the closing '
      if(pos < str.length() && str[pos] == '\'')
      {
        str[pos] = replacement;
      }

      continue;
    }

    bool intoString = false;
    if(str[pos] == '"' && !inString)
      intoString = true;

    if(inString || intoString) {
      if(inString) {
        if(str[pos] == '"')
          inString = false;
      }else{
        inString = true;
      }

      bool skip = false;
      if(str[pos] == '\\')
        skip = true;

      str[pos] = replacement;
      if(skip) {
        ++pos;
        if(pos < str.length())
          str[pos] = replacement;
      }
    }
  }
  return str;
}

int strip(const QByteArray& str, QByteArray& from)
{
  return strip_impl<QByteArray>(str, from);
}

int rStrip(const QByteArray& str, QByteArray& from)
{
  return rStrip_impl<QByteArray>(str, from);
}

QByteArray formatComment(const QByteArray& comment)
{
  return formatComment_impl<QByteArray>(comment);
}

QString formatComment(const QString& comment)
{
  return formatComment_impl<QString>(comment);
}


ParamIterator::~ParamIterator()
{
  delete d;
}

ParamIterator::ParamIterator( QString parens, QString source, int offset ) : d(new ParamIteratorPrivate)
{
  d->m_source = source;
  d->m_parens = parens;

  d->m_cur = offset;
  d->m_curEnd = offset;
  d->m_end = d->m_source.length();

  ///The whole search should be stopped when: A) The end-sign is found on the top-level B) A closing-brace of parameters was found
  int parenBegin = d->m_source.indexOf( parens[ 0 ], offset );

  //Search for an interrupting end-sign that comes before the found paren-begin
  int foundEnd = -1;
  if( parens.length() > 2 ) {
    foundEnd = d->m_source.indexOf( parens[2], offset );
    if( foundEnd > parenBegin && parenBegin != -1 )
      foundEnd = -1;
  }

  if( foundEnd != -1 ) {
    //We have to stop the search, because we found an interrupting end-sign before the opening-paren
    d->m_prefix = d->m_source.mid( offset, foundEnd - offset );

    d->m_curEnd = d->m_end = d->m_cur = foundEnd;
  } else {
    if( parenBegin != -1 ) {
      //We have a valid prefix before an opening-paren. Take the prefix, and start iterating parameters.
      d->m_prefix = d->m_source.mid( offset, parenBegin - offset );
      d->m_cur = parenBegin + 1;
      d->m_curEnd = d->next();
      if( d->m_curEnd == d->m_source.length() ) {
        //The paren was not closed. It might be an identifier like "operator<", so count everything as prefix.
        d->m_prefix = d->m_source.mid(offset);
        d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
      }
    } else {
      //We have neither found an ending-character, nor an opening-paren, so take the whole input and end
      d->m_prefix = d->m_source.mid(offset);
      d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
    }
  }
}

ParamIterator& ParamIterator::operator ++()
{
  if( d->m_source[d->m_curEnd] == d->m_parens[1] ) {
    //We have reached the end-paren. Stop iterating.
    d->m_cur = d->m_end = d->m_curEnd + 1;
  } else {
    //Iterate on through parameters
    d->m_cur = d->m_curEnd + 1;
    if ( d->m_cur < ( int ) d->m_source.length() )
    {
      d->m_curEnd = d->next();
    }
  }
  return *this;
}

QString ParamIterator::operator *()
{
  return d->m_source.mid( d->m_cur, d->m_curEnd - d->m_cur ).trimmed();
}

ParamIterator::operator bool() const
{
  return d->m_cur < ( int ) d->m_end;
}

QString ParamIterator::prefix() const
{
  return d->m_prefix;
}

uint ParamIterator::position() const {
  return (uint)d->m_cur;
}

}
