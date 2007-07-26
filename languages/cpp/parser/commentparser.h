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

class CommentFormatter {
    public:
    
    static QString formatComment( QString comment );
  private:
    static inline bool isWhite( QChar c ) {
        return c.isSpace();
    }
    
    static void rStrip( QString str, QString& from );
        
    static void strip( QString str, QString& from );
};

class Comment {
  public:
    Comment( QString text = "", int line = -1 );

    Comment( int line );

    void operator += ( Comment rhs );

    operator bool() const;

    operator QString();

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
  private:
    QString m_text;
    int m_line;
    bool m_formatted;
    
    
    void format();
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

