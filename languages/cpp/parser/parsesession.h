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

#include <cppparserexport.h>
#include "rpp/anchor.h"

#include <serialization/indexedstring.h>
#include <language/duchain/duchainpointer.h>
#include <language/interfaces/iastcontainer.h>
#include <language/editor/rangeinrevision.h>
#include <language/duchain/types/functiontype.h>
#include "commentformatter.h"

namespace Cpp {
  class EnvironmentFile;
}

class MemoryPool;
class TokenStream;
class Token;
class TranslationUnitAST;
class AST;

typedef QVector<unsigned int> PreprocessedContents;
typedef QPair<KDevelop::DUContextPointer, KDevelop::RangeInRevision> SimpleUse;

namespace rpp { class MacroBlock; class LocationTable; }

/// Contains everything needed to keep an AST useful once the rest of the parser
/// has gone away.
class KDEVCPPPARSER_EXPORT ParseSession : public KDevelop::IAstContainer
{
public:
  ParseSession();
  ~ParseSession();

  //Redefinitions from IAstContainer
  typedef QExplicitlySharedDataPointer<ParseSession> Ptr;
  typedef TranslationUnitAST TopAstNode;

  TopAstNode * topAstNode();
  void setTopAstNode(TopAstNode * node);

  ///Create a mapping between an AST node, and its DUChain Declaration
  void mapAstDuChain(AST *, KDevelop::DeclarationPointer);

  void mapAstUse(AST *node, const SimpleUse& use);

  ///Create a mapping between an AST node and its parent.
  void mapAstParent(AST *node, AST *parent);

  ///Create a mapping between an AST node and its parent.
  void mapCallAstToType(AST *node, KDevelop::FunctionType::Ptr type);

  /// Visits the tree and sets the appropriate nodes as parent on each node of the tree.
  void setASTNodeParents();

  /**
   * \brief Request an AST node from a DeclarationPointer
   * \return The Associated AST if available, NULL otherwise
   */
  AST * astNodeFromDeclaration(KDevelop::DeclarationPointer declaration);

  AST * astNodeFromDeclaration(KDevelop::Declaration * declaration);

  AST * astNodeFromUse(const SimpleUse &use) const;

  KDevelop::FunctionType::Ptr typeFromCallAst(AST* ast) const;

  /**
   * \brief Access the parent node for a given AST.
   * \return Parent AST node or NULL for the root node.
   */
  AST * parentAstNode(AST* node);

  /**
   * \brief Request a Declaration from an AST node
   * \return The Associated Declaration if available, empty DeclarationPointer otherwise
   */
  KDevelop::DeclarationPointer declarationFromAstNode(AST * node);

  /**
   * Return the position of the preprocessed source \a offset in the original source
   * If the "collapsed" member of the returned anchor is true, the position is within a collapsed range.
   @param collapseIfMacroExpansion @see LocationTable::positionForOffset
   * \note the return line starts from 0, not 1.
   */
  rpp::Anchor positionAt(std::size_t offset, bool collapseIfMacroExpansion = false) const;

  QPair<rpp::Anchor, uint> positionAndSpaceAt(std::size_t offset, bool collapseIfMacroExpansion = false) const;

  ///The contents must already be tokenized. Either by the preprocessor, or by tokenizeFromByteArray(..)
  void setContents(const PreprocessedContents& contents, rpp::LocationTable* locationTable);

  /// Unweildy name, but we want to be clear here, if there is already a location table, this would be the wrong setup function to call
  void setContentsAndGenerateLocationTable(const PreprocessedContents& contents);

  void setUrl(const KDevelop::IndexedString& url);
  const KDevelop::IndexedString& url() const;

  uint *contents();
  const uint *contents() const;
  const PreprocessedContents& contentsVector() const;
  std::size_t size() const;
  MemoryPool* mempool;
  TokenStream* token_stream;

  KDevelop::IndexedString m_url; //Should contain the url from which the content was extracted, can also be empty.

  CommentFormatter m_commentFormatter;

  /**
   * Get the string representation of all tokens in the AST's token range
   *
   * @param withSpaces if set to true, no additional space are added
   *                   between every token string
   *
   * @note This is very expensive
   */
  QString stringForNode(AST* node, bool withoutSpaces = false) const;

  /// useful for debugging, can be called from GDB
  void dumpNode(AST* node) const;

private:
  PreprocessedContents m_contents;
  rpp::LocationTable* m_locationTable;
  TranslationUnitAST * m_topAstNode;

  //AST-DUChain Mappings
  QHash<AST *, KDevelop::DeclarationPointer> m_AstToDuchain;
  QHash<KDevelop::DeclarationPointer, AST *> m_DuchainToAst;

  //TODO: look into removing these two
  QMap<AST *, SimpleUse> m_AstToUse;
  QMap<SimpleUse, AST *> m_UseToAst;

  //TODO: look into removing this astToParent
  QMap<AST *, AST *> m_AstToParent;
  QHash<AST*, KDevelop::FunctionType::Ptr> m_AstToType;
};

#endif
