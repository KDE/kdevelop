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

#include <QString>
#include <set>

class ParseSession;

class Comment {
  public:
    Comment( size_t token = 0, int line = -1 );

    operator bool() const;

    inline int line() const {
        return m_line;
    }

    bool operator < ( Comment& rhs ) const;

    bool isSame ( const Comment& rhs );

    struct cmp {
        bool operator () ( const Comment& c1, const Comment& c2 ) const {
            return c1.line() < c2.line();
        }
    };

    size_t token() const {
      return m_token;
    }
  private:
    int m_line;
    size_t m_token;
};


class CommentStore {
    private:
        typedef std::set< Comment, Comment::cmp > CommentSet;
        CommentSet m_comments;

    public:

        /**
         * Returns & removes the first comment currently in the comment-store
         * */
        Comment takeFirstComment();
        
        /**
         * Returns the comment nearest to the line of "end"(inclusive), and returns & removes it
         * */
        Comment takeCommentInRange( int endLine, int startLine = 0 );
        
        ///Returns and removes the comment in the given line
        Comment takeComment( int line );
        
        void addComment( Comment comment );
        
        ///Does not delete the comment
        Comment latestComment() const;

        bool hasComment() const {
          return !m_comments.empty();
        }
        
        void clear();
};

