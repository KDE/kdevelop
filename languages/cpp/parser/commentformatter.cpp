/*
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "commentformatter.h"
#include "ast.h"
#include <QStringList>
#include "parsesession.h"
#include "lexer.h"

void CommentFormatter::rStrip( const QString& str, QString& from ) {
  if( str.isEmpty() ) return;

  int i = 0;
  int ip = from.length();
  int s = from.length();

  for( int a = s-1; a >= 0; a-- ) {
      if( isWhite( from[a] ) ) {
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

  if( ip != (int)from.length() ) from = from.left( ip );
}

void CommentFormatter::strip( const QString& str, QString& from ) {
  if( str.isEmpty() ) return;

  int i = 0;
  int ip = 0;
  int s = from.length();

  for( int a = 0; a < s; a++ ) {
      if( isWhite( from[a] ) ) {
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

  if( ip ) from = from.mid( ip );
}

QString CommentFormatter::formatComment( size_t token, const ParseSession* session ) {
  const Token& commentToken( (*session->token_stream)[token] );
  return formatComment( QString::fromUtf8( QByteArray(commentToken.text+commentToken.position, commentToken.size) ) );
}

QString CommentFormatter::formatComment( const ListNode<size_t>* comments, const ParseSession* session ) {
  QString ret;
  if( comments )
  {
    const ListNode<size_t> *it = comments->toFront(), *end = it;
    do {
      QString c = CommentFormatter::formatComment(it->element, session);

      if( ret.isEmpty() )
        ret = c;
      else
        ret += QString("\n(%1)").arg(c);

      it = it->next;
    }while( it != end );
  }

  return ret;
}

QString CommentFormatter::formatComment( const QString& comment ) {
  QString ret;
  int i = 0;
  int s = comment.length();
  while( i < s && comment[i] == '/' ) {
      i++;
  }

  if( i > 1 ) {
      ret = comment.mid( i );
  } else {
      ///remove the star in each line
      QStringList lines = comment.split( "\n", QString::KeepEmptyParts );

      if( lines.isEmpty() ) return ret;

      strip( "/**", lines.front() );
      rStrip( "/**", lines.back() );

      QStringList::iterator it = lines.begin();
      ++it;
      QStringList::iterator eit = lines.end();

      if( it != lines.end() ) {
          --eit;

          for( ; it != eit; ++it ) {
              strip( "*", *it );
          }

          if( lines.front().trimmed().isEmpty() )
              lines.pop_front();

          if( lines.back().trimmed().isEmpty() )
              lines.pop_back();
      }

      ret = lines.join( "\n" );
  }

  return ret;
}
