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
#include <language/duchain/declarationid.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/indexedtype.h>

class QString;

namespace Cpp {
using namespace KDevelop;

class KDEVCPPDUCHAIN_EXPORT ExpressionEvaluationResult {
  public:
    ExpressionEvaluationResult();
    ~ExpressionEvaluationResult();
    ExpressionEvaluationResult(const ExpressionEvaluationResult& rhs);
    ExpressionEvaluationResult& operator=(const ExpressionEvaluationResult& rhs);

    IndexedType type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate

    bool isInstance; ///Whether the result of this expression is an instance(as it normally should be)
    DeclarationId instanceDeclaration; ///If this expression is an instance of some type, this either contains the declaration of the instance, or the type

    static size_t classSize() {
      return sizeof(ExpressionEvaluationResult);
    }

    ///This list contains the declarations found for the item evaluated.
    QList<DeclarationId> allDeclarations;

    uint allDeclarationsSize() const {
      return allDeclarations.size();
    }
    
    QList<DeclarationId>& allDeclarationsList() {
      return allDeclarations;
    }
    
    ///@return whether the result is an lvalue
    bool isLValue() const {
      return isInstance && (instanceDeclaration.isValid() || type.type<ReferenceType>());
    }

    ///@return whether this result is valid(has a type)
    bool isValid() const {
      return type.isValid();
    }

    bool operator==(const ExpressionEvaluationResult& rhs) const;

    unsigned int hash() const;

    ///Duchain must be read-locked
    IndexedTypeIdentifier identifier() const;

    ///@return A short version, that only contains the name or value, without instance-information etc. Should be language-processable.
    QString toShortString() const;

    ///it does not matter whether du-chain is locked or not
    QString toString() const;
};

}

#endif
