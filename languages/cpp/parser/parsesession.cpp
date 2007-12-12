/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "parsesession.h"

#include "rpp/pp-location.h"

#include "lexer.h"
#include "memorypool.h"
#include <iproblem.h>

#include <cctype>

ParseSession::ParseSession(QString (*stringUnifier) (const QString&))
  : mempool(new pool)
  , token_stream(0)
  , m_locationTable(0)
  , m_unifier(stringUnifier)
{
}

ParseSession::~ParseSession()
{
  delete mempool;
  delete token_stream;
  delete m_locationTable;
}

KTextEditor::Cursor ParseSession::positionAt(std::size_t offset) const
{
  Q_ASSERT(m_locationTable);

  // FIXME shouldn't just add the column offset...??

  return m_locationTable->positionForOffset(offset) + m_contentOffset;
}

QString ParseSession::unify(const QString& str) const
{
  if(m_unifier)
    return m_unifier(str);
  else
    return str;
}

std::size_t ParseSession::size() const
{
  return m_contents.size() + 1;
}

const char * ParseSession::contents() const
{
  return m_contents.constData();
}

void ParseSession::setContents(const QByteArray & contents, rpp::LocationTable* locationTable, const KTextEditor::Cursor& offset)
{
  m_contents = contents;
  m_contentOffset = offset;
  m_locationTable = locationTable;
}

void ParseSession::setContentsAndGenerateLocationTable(const QByteArray & contents, const KTextEditor::Cursor& offset)
{
  m_contents = contents;
  m_contentOffset = offset;
  m_locationTable = new rpp::LocationTable(m_contents);
}

void ParseSession::setUrl(const KDevelop::HashedString& url)
{
  m_url = url;
}

const KDevelop::HashedString& ParseSession::url() const
{
  return m_url;
}
