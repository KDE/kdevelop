/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef CPP_MISSINGDECLARATIONTYPE_H
#define CPP_MISSINGDECLARATIONTYPE_H

#include <language/duchain/types/delayedtype.h>
#include "expressionevaluationresult.h"
#include <language/duchain/ducontext.h>
#include "overloadresolution.h"
#include "cppduchainexport.h"


namespace Cpp {

///This is a placeholder type used in the expression-visitor to allow recording the context a missing item appears in
///The type itself contains the name of the missing item
class KDEVCPPDUCHAIN_EXPORT MissingDeclarationType : public KDevelop::DelayedType
{
  public:
    MissingDeclarationType();
    typedef TypePtr<MissingDeclarationType> Ptr;
    
    virtual QString toString() const override;

    //Context where the search for the item was started.
    KDevelop::IndexedDUContext searchStartContext;
    
    //Context within which this item should be/is contained. Optional.
    KDevelop::IndexedDUContext containerContext;
    
    //The missing item was converted to this type, either through an assignment, or as an argument in a function-call or operator
    ExpressionEvaluationResult convertedTo;
    
    //The given type was assigned to this item
    ExpressionEvaluationResult assigned;
    
    //The missing item was called like a function with these arguments
    QList<OverloadResolver::Parameter> arguments;
    bool isFunction;
    
    //Whether this declaration is supposed to be a type
    bool isType;
};

}

#endif // CPP_MISSINGDECLARATIONTYPE_H
