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

#ifndef COMMENTFORMATTER_H
#define COMMENTFORMATTER_H

#include <QString>
#include <QList>
#include <cppparserexport.h>
#include <language/duchain/problem.h>

class Control;
template<class Tp>
class ListNode;

class ParseSession;

/**
 * This class cares about extracting the interesting information out of a comment.
 * For example it removes all the stars at the beginning, and re-indents the text.
 * */
class KDEVCPPPARSER_EXPORT CommentFormatter {
  public:
    CommentFormatter();
    
    ///Processes the comment represented by the given token-number within the parse-session's token-stream
    QByteArray formatComment( uint token, const ParseSession* session );

    void extractToDos( uint token, const ParseSession* session, Control* control );

    ///Processes the list of comments represented by the given token-number within the parse-session's token-stream
    QByteArray formatComment( const ListNode<uint>* node, const ParseSession* session );
  private:
    bool containsToDo(const uint* start, const uint* end) const;
    bool containsToDo(const QByteArray& text) const;
    QVector<uint> m_commentMarkerIndices; // IndexedString indices
    QVector<QByteArray> m_commentMarkers;
};

#endif
