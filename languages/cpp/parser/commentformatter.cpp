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
#include <language/editor/simplerange.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include "control.h"

CommentFormatter::CommentFormatter()
{
  if(!KDevelop::ICore::self())
    return; // May happen in tests
  
  foreach(QString marker, KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords())
  {
    m_commentMarkers << marker.toUtf8();
    m_commentMarkerIndices << KDevelop::IndexedString(marker).index();
  }
}

bool CommentFormatter::containsToDo(const uint* start, const uint* end) const
{
  const uint* markersStart = m_commentMarkerIndices.data();
  const uint* markersEnd = m_commentMarkerIndices.data() + m_commentMarkerIndices.size();
  
  for(const uint* cursor = start; cursor < end; ++cursor)
    for(const uint* marker = markersStart; marker < markersEnd; ++marker)
      if(*cursor == *marker)
        return true;
  
  return false;
}

bool CommentFormatter::containsToDo(const QByteArray& text) const
{
  foreach(const QByteArray& marker, m_commentMarkers)
    if(text.contains(marker))
      return true;
  return false;
}

void CommentFormatter::extractToDos( uint token, const ParseSession* session, Control* control ) {

  if( !token )
    return;
  
  const Token& commentToken( (*session->token_stream)[token] );
  
  if( !containsToDo(session->contents() + commentToken.position, session->contents() + commentToken.position + commentToken.size) )
    return; // Most common code path: No todos
  
  QByteArray comment = stringFromContents(session->contentsVector(), commentToken.position, commentToken.size);
  QList<QByteArray> lines = comment.split( '\n' );
  if ( !lines.isEmpty() ) {
    QList<QByteArray>::iterator bit = lines.begin();
    QList<QByteArray>::iterator it = bit;
    QList<QByteArray>::iterator eit = lines.end();

    KDevelop::IndexedString document = session->url();
    KDevelop::SimpleCursor comment_start = session->positionAt(commentToken.position).castToSimpleCursor();

    for( ; it != eit; ++it ) {
      // remove common leading chars from the beginning of line
      int stripped_left = 0;
      int stripped_right = 0;
      stripped_left += KDevelop::strip( "///", *it );
      stripped_left += KDevelop::strip( "//", *it );
      stripped_left += KDevelop::strip( "**", *it );
      stripped_right += KDevelop::rStrip( "/**", *it );

      if( containsToDo(*it) ) {
        KDevelop::ProblemPointer p(new KDevelop::Problem());
        p->setSource(KDevelop::ProblemData::ToDo);
        p->setDescription(*it);
        p->setSeverity(KDevelop::ProblemData::Hint);
        int start_line = comment_start.line + (it - bit);
        int start_column = (it == bit) ? comment_start.column + stripped_left : stripped_left;
        p->setFinalLocation(KDevelop::DocumentRange(session->url(), KDevelop::SimpleRange(start_line, start_column, start_line, it->size() + start_column)));
        control->reportProblem(p);
      }
    }
  }
}

QByteArray CommentFormatter::formatComment( uint token, const ParseSession* session ) {
  if( !token )
    return QByteArray();
  const Token& commentToken( (*session->token_stream)[token] );
  return KDevelop::formatComment( stringFromContents(session->contentsVector(), commentToken.position, commentToken.size ) );
}

QByteArray CommentFormatter::formatComment( const ListNode<uint>* comments, const ParseSession* session ) {
  QByteArray ret;
  if( comments )
  {
    const ListNode<uint> *it = comments->toFront(), *end = it;
    do {
      QByteArray c = CommentFormatter::formatComment(it->element, session);

      if( ret.isEmpty() )
        ret = c;
      else
        ret += "\n(" + c + ")";

      it = it->next;
    }while( it != end );
  }

  return ret;
}

