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
#include "rpp/pp-environment.h"

#include "lexer.h"
#include "memorypool.h"
#include <iproblem.h>
#include <indexedstring.h>

#include <cctype>

ParseSession::ParseSession()
  : mempool(new pool)
  , token_stream(0)
  , macros(0)
  , m_locationTable(0)
{
}

ParseSession::~ParseSession()
{
  delete mempool;
  delete token_stream;
  delete m_locationTable;
  delete macros;
}

rpp::Anchor ParseSession::positionAt(std::size_t offset, bool collapseIfMacroExpansion) const
{
  Q_ASSERT(m_locationTable);

  return m_locationTable->positionAt(offset, m_contents, collapseIfMacroExpansion).first;
}

QPair<rpp::Anchor, uint> ParseSession::positionAndSpaceAt(std::size_t offset, bool collapseIfMacroExpansion) const
{
  Q_ASSERT(m_locationTable);

  return m_locationTable->positionAt(offset, m_contents, collapseIfMacroExpansion);
}

std::size_t ParseSession::size() const
{
  return m_contents.size() + 1;
}

 uint* ParseSession::contents()
 {
   return m_contents.data();
 }

const uint* ParseSession::contents() const
 {
   return m_contents.data();
 }
 
const PreprocessedContents& ParseSession::contentsVector() const
{
  return m_contents;
}

void ParseSession::setContents(const PreprocessedContents& contents, rpp::LocationTable* locationTable)
{
  m_contents = contents;
  m_locationTable = locationTable;
}

void ParseSession::setContentsAndGenerateLocationTable(const PreprocessedContents& contents)
{
  m_contents = contents;
  ///@todo We need this in the lexer, the problem is that we copy the vector when doing this
  m_contents.append(0);
  m_contents.append(0);
  m_contents.append(0);
  m_contents.append(0);
  
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
