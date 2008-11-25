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
#include "rpp/chartools.h"
#include <language/duchain/stringhelpers.h>


QString CommentFormatter::formatComment( size_t token, const ParseSession* session ) {
  if( !token )
    return QString();
  const Token& commentToken( (*session->token_stream)[token] );
  return KDevelop::formatComment( stringFromContents(session->contentsVector(), commentToken.position, commentToken.size ) );
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

