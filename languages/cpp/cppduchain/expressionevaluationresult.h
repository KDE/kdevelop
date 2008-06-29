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
#ifndef EXPRESSIONEVALUATIONRESULT_H
#define EXPRESSIONEVALUATIONRESULT_H

#include "cppduchainexport.h"
#include <duchain/appendedlist.h>
#include <typesystem.h>
#include <declarationid.h>

namespace KDevelop {
  class IndexedType;
  class TypeIdentifier;
}

class QString;

namespace Cpp {
using namespace KDevelop;;
KDEVCPPDUCHAIN_EXPORT DECLARE_LIST_MEMBER_HASH(ExpressionEvaluationResult, allDeclarations, DeclarationId);

class ExpressionEvaluationResult;

class KDEVCPPDUCHAIN_EXPORT IndexedExpressionEvaluationResult {
  public:
    IndexedExpressionEvaluationResult();
    IndexedExpressionEvaluationResult(uint index);
    
    const ExpressionEvaluationResult& result() const;
    
    uint hash() const {
      return m_index;
    }
    
    bool operator==(const IndexedExpressionEvaluationResult& rhs) const {
      return m_index == rhs.m_index;
    }
    
  private:
    uint m_index;
};

class KDEVCPPDUCHAIN_EXPORT ExpressionEvaluationResult {
  public:
    ExpressionEvaluationResult();
    ~ExpressionEvaluationResult();
    ExpressionEvaluationResult(const ExpressionEvaluationResult& rhs);
    ExpressionEvaluationResult& operator=(const ExpressionEvaluationResult& rhs);

    IndexedType type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate
    
    bool isInstance; ///Whether the result of this expression is an instance(as it normally should be)
    DeclarationId instanceDeclaration; ///If this expression is an instance of some type, this either contains the declaration of the instance, or the type

    ///Returns the indexed version of this evalation result(eventually it is put into a repository)
    IndexedExpressionEvaluationResult indexed() const;
    
    START_APPENDED_LISTS(ExpressionEvaluationResult);
    ///This list contains the declarations found for the item evaluated.
    ///@todo Eventually get rid of this list somehow
    APPENDED_LIST_FIRST(ExpressionEvaluationResult, DeclarationId, allDeclarations);
    
    END_APPENDED_LISTS(ExpressionEvaluationResult, allDeclarations);

    ///@return whether the result is an lvalue
    bool isLValue() const {
      return isInstance && (instanceDeclaration.isValid() || dynamic_cast<const ReferenceType*>( type.type().data() ));
    }

    ///@return whether this result is valid(has a type)
    bool isValid() const {
      return type.isValid();
    }
    
    bool operator==(const ExpressionEvaluationResult& rhs) const;

    unsigned int hash() const;
    
    ///Duchain must be read-locked
    TypeIdentifier identifier() const;
    
    ///@return A short version, that only contains the name or value, without instance-information etc. Should be language-processable.
    QString toShortString() const;
    
    ///it does not matter whether du-chain is locked or not
    QString toString() const;
};

}

#endif
