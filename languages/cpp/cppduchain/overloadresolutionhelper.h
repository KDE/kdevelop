/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef OVERLOADRESOLUTIONHELPER_H
#define OVERLOADRESOLUTIONHELPER_H

#include "cppduchainexport.h"
#include <QPair>
#include <QList>
#include "overloadresolution.h"
#include "viablefunctions.h"
#include <duchain/identifier.h>

namespace KDevelop {
  class QualifiedIdentifier;
  class Declaration;
}

namespace Cpp
{
  class ViableFunction;
  
  struct KDEVCPPDUCHAIN_EXPORT OverloadResolutionFunction {
    OverloadResolutionFunction();
    OverloadResolutionFunction( int _matchedArguments, const ViableFunction& _viable );

    /**The count of arguments that are already matched in this OverloadResolutionFunction. The argument of the OverloadResolutionFunction at this offset is the next one to be matched.
     *This is variable, and needed so global binary overloaded operators and member binary
     *operators can be treated same(global operators take 2 arguments, local
     *operators only 1)
     * */
    int matchedArguments;

    ///Result of matching the OverloadResolutionFunction to available arguments. Also contains the declaration.
    ViableFunction function;
  };
  
  /**
   * Helps searching and revolving functions or operator-functions in a unified way, partially or completely.
   * The du-chain must be locked for the whole lifetime of this object.
   * */
  class KDEVCPPDUCHAIN_EXPORT OverloadResolutionHelper {
    public:
    typedef QPair<OverloadResolver::ParameterList, KDevelop::Declaration*> DeclarationWithArgument;

    OverloadResolutionHelper(const KDevelop::DUContextPointer& context, const KDevelop::TopDUContextPointer& topContext);

    /**
     * @param base Sets the base-type, which is used while searching overloaded operators as:
     * Container-class of member operators, or first argument of global operators.
     * @param operatorName is the string-name of the operator, without "operator". Example: ">" ">=", etc.
     * */
    void setOperator( const OverloadResolver::Parameter& base, const QString& operatorName );

    /**
     * Call this instead of setOperator to use the specified list of functions for overload-resolution
     * */
    void setFunctions( const QList<Declaration*>& functions );

    /**
     * Call this to set the already known parameter-types for the function-call,
     * or for the operator-call.(For operators, this should not include the base-type)
     * */
    void setKnownParameters( const OverloadResolver::ParameterList& parameters );
    
    /**
     * @param partial If partial is given, it is not required that all parameters of the functions have a value.
     *
     * The du-chain must be read-locked.
     * */
    QList<OverloadResolutionFunction> resolve( bool partial );

    private:
      void log(const QString& str) const;
      KDevelop::DUContextPointer m_context;
      KDevelop::TopDUContextPointer m_topContext;
      OverloadResolver::Parameter m_baseType;
      bool m_isOperator;
      QList< DeclarationWithArgument > m_declarations; //Declarations are paired with the optional first argument for the declared functions
      OverloadResolver::ParameterList m_knownParameters;
      Identifier m_operatorIdentifier;
  };

}

#endif
