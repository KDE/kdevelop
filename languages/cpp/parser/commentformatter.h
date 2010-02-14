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
#include <cppparserexport.h>

template<class Tp>
class ListNode;

/**
 * This class cares about extracting the interesting information out of a comment.
 * For example it removes all the stars at the beginning, and re-indents the text.
 * */

class ParseSession;

class KDEVCPPPARSER_EXPORT CommentFormatter {
    public:
    ///Processes the comment represented by the given token-number within the parse-session's token-stream
    static QByteArray formatComment( uint token, const ParseSession* session );

    ///Processes the list of comments represented by the given token-number within the parse-session's token-stream
    static QByteArray formatComment( const ListNode<uint>* node, const ParseSession* session );
  private:
};

#endif
