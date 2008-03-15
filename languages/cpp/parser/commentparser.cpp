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
// #include <kdebug.h>

Comment::Comment( size_t token, int line ) : m_line(line), m_token( token ) {
}

Comment::operator bool() const {
    return m_line != -1 && m_token != 0;
}

bool Comment::operator==( const Comment& rhs ) const {
  return isSame(rhs);
}


bool Comment::operator < ( const Comment& rhs ) const {
    return m_token < rhs.m_token;
}

bool Comment::isSame ( const Comment& rhs ) const {
    return m_token == rhs.m_token;
}

Comment CommentStore::takeComment( int line ) {
    CommentSet::iterator it = m_comments.find( Comment( 0, line ) );
    if( it != m_comments.end() ) {
        Comment ret = *it;
        m_comments.erase( it );
/*        kDebug() << "Took comment in line" << line << "new count:" << m_comments.size();*/
        return ret;
    } else {
        return Comment();
    }
}


Comment CommentStore::takeCommentInRange( int end, int start ) {
    CommentSet::iterator it = m_comments.lower_bound( Comment( 0, end ) );


    while( it != m_comments.begin() && (*it).line() > end ) {
        --it;
    }

    if( it != m_comments.end() && (*it).line() >= start && (*it).line() <= end ) {
        Comment ret = *it;
        m_comments.erase( it );
/*        kDebug() << "Took comment in line" << (*it).line() << "new count:" << m_comments.size();*/
        return ret;
    } else {
        return Comment();
    }
}

void CommentStore::addComment( Comment comment ) {
    CommentSet::iterator it = m_comments.find( comment );
    if( it != m_comments.end() ) {
        if( comment.isSame( *it ) ) return;
        ///@todo comment-merging?
/*        Comment c = *it;
        c += comment;
        comment = c;*/
        //m_comments.erase( it );
    }

/*    kDebug() << "size" << m_comments.size();*/
    m_comments.insert( comment );
/*    kDebug() << "added comment in line" << comment.line() << "token" << comment.token() << "new count" << m_comments.size();*/
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
/*    kDebug() << "taken first comment in line" << (*it).line() << "new count:" << m_comments.size();*/
    return ret;
}

void CommentStore::clear() {
    m_comments.clear();
}


