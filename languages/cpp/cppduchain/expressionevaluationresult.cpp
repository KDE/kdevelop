/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "expressionevaluationresult.h"
#include <identifier.h>
#include <QString>
#include <duchainlock.h>
#include <repositories/itemrepository.h>
#include <duchain/duchain.h>
#include <duchain/typesystem.h>
#include <duchain/identifiedtype.h>

namespace Cpp {

  ///@todo remove this again
KDevelop::ItemRepository<ExpressionEvaluationResult, AppendedListItemRequest<ExpressionEvaluationResult> > expressionEvaluationResultRepository("Expression Evaluation Result Repository");
const uint standardExpressionEvaluationResultIndex( expressionEvaluationResultRepository.index( ExpressionEvaluationResult() ) );

DEFINE_LIST_MEMBER_HASH(ExpressionEvaluationResult, allDeclarations, DeclarationId)

TypeIdentifier ExpressionEvaluationResult::identifier() const {
  static TypeIdentifier noIdentifier("(no type)");
  
  AbstractType::Ptr t(type.type());
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.data());
  if( idType )
    return idType->identifier();
  
  DelayedType* delayedType = dynamic_cast<DelayedType*>(t.data());
  if( delayedType )
    return delayedType->identifier();
  
  if( t )
    return TypeIdentifier( t->toString() );
  else
    return noIdentifier;
}

IndexedExpressionEvaluationResult::IndexedExpressionEvaluationResult(uint index) : m_index(index) {
}

IndexedExpressionEvaluationResult::IndexedExpressionEvaluationResult() : m_index(standardExpressionEvaluationResultIndex) {
}

const ExpressionEvaluationResult& IndexedExpressionEvaluationResult::result() const {
  return *expressionEvaluationResultRepository.itemFromIndex(m_index);
}

IndexedExpressionEvaluationResult ExpressionEvaluationResult::indexed() const {
  return IndexedExpressionEvaluationResult( expressionEvaluationResultRepository.index( *this ) );
}

QString ExpressionEvaluationResult::toString() const {
  if( DUChain::lock()->currentThreadHasReadLock() )
    return QString(isLValue() ? "lvalue " : "") + QString(isInstance ? "instance " : "") + (type.type() ? type.type()->toString() : QString("(no type)"));

  DUChainReadLocker lock(DUChain::lock());
  return QString(isLValue() ? "lvalue " : "") + QString(isInstance ? "instance " : "") + (type ? type.type()->toString() : QString("(no type)"));
}

unsigned int ExpressionEvaluationResult::hash() const {
  uint ret = ((type.hash() + (isInstance ? 1 : 0) * 101) + instanceDeclaration.hash()) * 73;
  FOREACH_FUNCTION(const DeclarationId& id, allDeclarations)
      ret *= (id.hash() * 37);
  
  return ret;
}

QString ExpressionEvaluationResult::toShortString() const
{
  //Inline for now, so it can be used from the duchainbuilder module
  if( DUChain::lock()->currentThreadHasReadLock() )
    return type ? type.type()->toString() : QString("(no type)");

  DUChainReadLocker lock(DUChain::lock());
  return type ? type.type()->toString() : QString("(no type)");
}

ExpressionEvaluationResult::~ExpressionEvaluationResult() {
  freeAppendedLists();
}

ExpressionEvaluationResult::ExpressionEvaluationResult() : isInstance(false) {
  initializeAppendedLists();
}

ExpressionEvaluationResult::ExpressionEvaluationResult(const ExpressionEvaluationResult& rhs) {
  initializeAppendedLists();
  copyListsFrom(rhs);
    
  type = rhs.type;
  isInstance = rhs.isInstance;
  instanceDeclaration = rhs.instanceDeclaration;
}

bool ExpressionEvaluationResult::operator==(const ExpressionEvaluationResult& rhs) const {
    if(!listsEqual(rhs))
      return false;
    
    return type == rhs.type && isInstance == rhs.isInstance && instanceDeclaration == rhs.instanceDeclaration;
}


ExpressionEvaluationResult& ExpressionEvaluationResult::operator=(const ExpressionEvaluationResult& rhs) {
  copyListsFrom(rhs);
    
  type = rhs.type;
    
  isInstance = rhs.isInstance;
  instanceDeclaration = rhs.instanceDeclaration;
  return *this;
}

}
