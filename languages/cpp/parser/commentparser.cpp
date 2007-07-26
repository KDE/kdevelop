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
#include "commentparser.h"
#include <QStringList>

void CommentFormatter::rStrip( QString str, QString& from ) {
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

void CommentFormatter::strip( QString str, QString& from ) {
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

QString CommentFormatter::formatComment( QString comment ) {
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

void Comment::format() {
  if( m_formatted ) return;
  m_formatted = true;
  m_text = CommentFormatter::formatComment( m_text );
}

Comment::Comment( QString text, int line ) : m_text( text ), m_line( line ), m_formatted(false) {
}

Comment::Comment( int line ) : m_line( line ) {
}

void Comment::operator += ( Comment rhs ) {
    format();
    rhs.format();
    m_text += " " + rhs.m_text;
}

Comment::operator bool() const {
    return !m_text.isEmpty();
}

Comment::operator QString() {
    format();
    return m_text;
}

bool Comment::operator < ( Comment& rhs ) const {
    return m_line < rhs.m_line;
}

bool Comment::isSame ( const Comment& rhs ) {
    if( rhs.m_formatted ) format();
    return m_text == rhs.m_text;
}

Comment CommentStore::takeComment( int line ) {
    CommentSet::iterator it = m_comments.find( line );
    if( it != m_comments.end() ) {
        Comment ret = *it;
        m_comments.erase( it );
        return ret;
    } else {
        return Comment();
    }
}


Comment CommentStore::takeCommentInRange( int end, int start ) {
    CommentSet::iterator it = m_comments.lower_bound(  end );


    while( it != m_comments.begin() && (*it).line() > end ) {
        --it;
    }

    if( it != m_comments.end() && (*it).line() >= start && (*it).line() <= end ) {
        Comment ret = *it;
        m_comments.erase( it );
        return ret;
    } else {
        return Comment();
    }
}

void CommentStore::addComment( Comment comment ) {

    CommentSet::iterator it = m_comments.find( comment );
    if( it != m_comments.end() ) {
        if( comment.isSame( *it ) ) return;
        Comment c = *it;
        c += comment;
        comment = c;
        m_comments.erase( it );
    }

    m_comments.insert( comment );
}

///Does not delete the comment
Comment CommentStore::latestComment() const {
    CommentSet::const_iterator it = m_comments.end();
    if( it == m_comments.begin() ) return Comment();
    --it;
    return *it;
}

Comment CommentStore::takeFirstComment() {
    CommentSet::iterator it = m_comments.begin();
    if( it == m_comments.end() ) return Comment();
    Comment ret = *it;
    m_comments.erase(it);
    return ret;
}

void CommentStore::clear() {
    m_comments.clear();
}


