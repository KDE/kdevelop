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

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <cstdlib>

#include <QtCore/QString>
#include <QtCore/QByteArray>

#include <editor/simplecursor.h>

#include <cppparserexport.h>
#include <ksharedptr.h>
#include <kurl.h>
#include "rpp/anchor.h"

#include <hashedstring.h>

namespace Cpp {
  class EnvironmentFile;
}

class pool;
class TokenStream;
class Token;

typedef QVector<unsigned int> PreprocessedContents;

namespace rpp { class MacroBlock; class LocationTable; }

/// Contains everything needed to keep an AST useful once the rest of the parser
/// has gone away.
class KDEVCPPPARSER_EXPORT ParseSession
{
public:
  ///@todo Make stringUnifier unnecessary by moving the string-repository into the parser
  ParseSession();
  ~ParseSession();

  /**
   * Return the position of the preprocessed source \a offset in the original source
   * If the "collapsed" member of the returned anchor is true, the position is within a collapsed range.
   @param collapseIfMacroExpansion @see LocationTable::positionForOffset
   * \note the return line starts from 0, not 1.
   */
  rpp::Anchor positionAt(std::size_t offset, bool collapseIfMacroExpansion = false) const;

  ///The contents must already be tokenized. Either by the preprocessor, or by tokenizeFromByteArray(..)
  void setContents(const PreprocessedContents& contents, rpp::LocationTable* locationTable);

  /// Unweildy name, but we want to be clear here, if there is already a location table, this would be the wrong setup function to call
  void setContentsAndGenerateLocationTable(const PreprocessedContents& contents);

  void setUrl(const KDevelop::HashedString& url); ///@todo change to IndexedString
  const KDevelop::HashedString& url() const;

  uint *contents();
  const uint *contents() const;
  const PreprocessedContents& contentsVector() const;
  std::size_t size() const;
  pool* mempool;
  TokenStream* token_stream;

  rpp::MacroBlock* macros;
  KDevelop::HashedString m_url; //Should contain the url from which the content was extracted, can also be empty.

private:
  PreprocessedContents m_contents;
  rpp::LocationTable* m_locationTable;
};

#endif
