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
#include <language/duchain/identifier.h>
#include <QString>
#include <language/duchain/duchainlock.h>
#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/delayedtype.h>

namespace Cpp {

IndexedTypeIdentifier ExpressionEvaluationResult::identifier() const {
  static IndexedTypeIdentifier noIdentifier("(no type)");

  AbstractType::Ptr t(type.abstractType());
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.data());
  if( idType )
    return IndexedTypeIdentifier(idType->qualifiedIdentifier());

  DelayedType* delayedType = dynamic_cast<DelayedType*>(t.data());
  if( delayedType )
    return delayedType->identifier();

  if( t ) {
    return IndexedTypeIdentifier( t->toString(), true );
  }else
    return noIdentifier;
}

QString ExpressionEvaluationResult::toString() const {
  if( DUChain::lock()->currentThreadHasReadLock() )
    return QString(isLValue() ? "lvalue " : "") + QString(isInstance ? "instance " : "") + (type.abstractType() ? type.abstractType()->toString() : QString("(no type)"));

  DUChainReadLocker lock(DUChain::lock());
  return QString(isLValue() ? "lvalue " : "") + QString(isInstance ? "instance " : "") + (type ? type.abstractType()->toString() : QString("(no type)"));
}

unsigned int ExpressionEvaluationResult::hash() const {
  uint ret = ((type.hash() + (isInstance ? 1 : 0) * 101) + instanceDeclaration.hash()) * 73;
  foreach(const DeclarationId& id, allDeclarations)
      ret *= (id.hash() * 37);

  return ret;
}

QString ExpressionEvaluationResult::toShortString() const
{
  //Inline for now, so it can be used from the duchainbuilder module
  if( DUChain::lock()->currentThreadHasReadLock() )
    return type ? type.abstractType()->toString() : QString("(no type)");

  DUChainReadLocker lock(DUChain::lock());
  return type ? type.abstractType()->toString() : QString("(no type)");
}

ExpressionEvaluationResult::~ExpressionEvaluationResult() {
}

ExpressionEvaluationResult::ExpressionEvaluationResult() : isInstance(false) {
}

ExpressionEvaluationResult::ExpressionEvaluationResult(const ExpressionEvaluationResult& rhs) {
  type = rhs.type;
  isInstance = rhs.isInstance;
  instanceDeclaration = rhs.instanceDeclaration;
  allDeclarations = rhs.allDeclarations;
}

bool ExpressionEvaluationResult::operator==(const ExpressionEvaluationResult& rhs) const {
    return type == rhs.type && isInstance == rhs.isInstance && instanceDeclaration == rhs.instanceDeclaration && allDeclarations == rhs.allDeclarations;
}


ExpressionEvaluationResult& ExpressionEvaluationResult::operator=(const ExpressionEvaluationResult& rhs) {
  allDeclarations = rhs.allDeclarations;
  type = rhs.type;
  isInstance = rhs.isInstance;
  instanceDeclaration = rhs.instanceDeclaration;
  return *this;
}

}
