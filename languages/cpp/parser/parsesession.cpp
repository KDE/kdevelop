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
#include <language/interfaces/iproblem.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/declaration.h>

#include "ast.h"
#include "dumptree.h"
#include "parentvisitor.h"

ParseSession::ParseSession()
  : mempool(new MemoryPool)
  , token_stream(0)
  , m_locationTable(0)
  , m_topAstNode(0)
{
}

ParseSession::~ParseSession()
{
  delete mempool;
  delete token_stream;
  delete m_locationTable;
}

TranslationUnitAST * ParseSession::topAstNode(void)
{
  return m_topAstNode;
}

void ParseSession::topAstNode(TranslationUnitAST * node)
{
  Q_ASSERT(!m_topAstNode);

  m_topAstNode = node;
}

void ParseSession::mapAstDuChain (AST * node , KDevelop::DeclarationPointer declaration)
{
  //Duplicates shouldn't exist
  Q_ASSERT(m_AstToDuchain.find(node) == m_AstToDuchain.end() ||
           m_AstToDuchain[node] != declaration);

  // NOTE: Don't call declaration->toString() here. It seems that you cannot
  //        assume at this point that the DUChain is at least locked for reading.
  //kDebug() << "Mapping AST node: " << names[node->kind] <<
  //            "With Declaration: " << declaration->toString();

  m_AstToDuchain[node] = declaration;
  m_DuchainToAst[declaration] = node;
}

void ParseSession::mapAstUse(AST *node, const SimpleUse& use)
{
  //Duplicates shouldn't exist(? Same for uses?)
  if(m_AstToUse.contains(node) && m_AstToUse[node] != use)
    kWarning() << "Found dupplicate use mapping for node" << node;

  m_AstToUse[node] = use;
  m_UseToAst[use] = node;
}

void ParseSession::setASTNodeParents()
{
  ParentVisitor visitor(this);
  visitor.visit(m_topAstNode);
}

void ParseSession::mapAstParent(AST* node, AST* parent)
{
  m_AstToParent.insert(node, parent);
}

AST * ParseSession::astNodeFromDeclaration(KDevelop::DeclarationPointer declaration)
{
  //declaration was not mapped
  if(m_DuchainToAst.find(declaration) == m_DuchainToAst.end())
    return 0;
  else
    return m_DuchainToAst[declaration];
}

void ParseSession::mapCallAstToType(AST* node, KDevelop::FunctionType::Ptr type)
{
  m_AstToType.insert(node, type);
}

AST * ParseSession::astNodeFromDeclaration(KDevelop::Declaration * declaration)
{
  return astNodeFromDeclaration(KDevelop::DeclarationPointer(declaration));
}

AST * ParseSession::astNodeFromUse(const SimpleUse &use) const
{
  return m_UseToAst.value(use);
}

AST* ParseSession::parentAstNode(AST* node)
{
  return m_AstToParent.value(node, 0);
}

KDevelop::FunctionType::Ptr ParseSession::typeFromCallAst(AST* ast) const
{
  return m_AstToType.value(ast);
}

KDevelop::DeclarationPointer ParseSession::declarationFromAstNode(AST * node)
{
  //declaration was not mapped
  if(m_AstToDuchain.find(node) == m_AstToDuchain.end())
    return KDevelop::DeclarationPointer();
  else
    return m_AstToDuchain[node];
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

void ParseSession::setUrl(const KDevelop::IndexedString& url)
{
  m_url = url;
}

const KDevelop::IndexedString& ParseSession::url() const
{
  return m_url;
}

QString ParseSession::stringForNode(AST* node, bool withoutSpaces) const
{
  QString ret;
  for( uint a = node->start_token; a < node->end_token; a++ ) {
    ret += token_stream->symbolString(a);
    if (!withoutSpaces) {
      // Decode operator-names without spaces for now, since we rely on it in other places.
      /// @todo change this, here and in all the places that rely on it.
      /// Operators should then by written like "operator [ ]"(space between each token)
      ret += QLatin1Char(' ');
    }
  }
  return ret;
}

void ParseSession::dumpNode(AST* node) const
{
  DumpTree dumper;
  dumper.dump(node, token_stream, true);
}
