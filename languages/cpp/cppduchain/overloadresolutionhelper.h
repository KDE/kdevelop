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
#include <language/duchain/identifier.h>

namespace Cpp
{
  class ViableFunction;
  
  class KDEVCPPDUCHAIN_EXPORT OverloadResolutionFunction {
  public:

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
     * @param identifierForADL The identifier will be used for performing argument-dependent look-up.
     *                         This should only be non-empty if the function is not being called as a member.
     * */
    void setFunctionNameForADL( const KDevelop::QualifiedIdentifier& identifierForADL );
    
    /**
     * @param base Sets the base-type, which is used while searching overloaded operators as:
     * Container-class of member operators, or first argument of global operators.
     * @warning: If calling this, you _must_ also call setFunctionNameForADL with the full operator name,
     *           like "operator=".
     * */
    void setOperator( const OverloadResolver::Parameter& base );

    /**
     * Call this instead of setOperator to use the specified list of functions for overload-resolution
     * */
    void setFunctions( const QList<KDevelop::Declaration*>& functions );

    /**
     * Call this to set the already known parameter-types for the function-call,
     * or for the operator-call.(For operators, this should not include the base-type)
     * */
    void setKnownParameters( const OverloadResolver::ParameterList& parameters );

    /**
     * Call this to set the whether we are looking for a const or a non-const method.
     * By default, the constness is not taken into account.
     */
    void setConstness(OverloadResolver::Constness constness);

    /**
     * @param partial If partial is given, it is not required that all parameters of the functions have a value.
     *
     * This is relatively slow, and returns a list of all considered functions, sorted by viability.
     * 
     * The du-chain must be read-locked.
     * */
    QList<OverloadResolutionFunction> resolveToList( bool partial = false );

    /**
     * The du-chain must be read-locked.
     * 
     * Returns only the most viable matched function. If no function is viable, a nonviable
     * function will be returned.
     *
     * NOTE: you *must* call @c setFunctions before, if not looking for operators!
     *
     * @param forceIsInstance If this is true, all encountered class types will be considered _instances_
     *                        of the class.
     * */
    ViableFunction resolve( bool forceIsInstance = false );
    
    private:
      void initializeResolver(OverloadResolver& resolver);
      void log(const QString& str) const;
      KDevelop::DUContextPointer m_context;
      KDevelop::TopDUContextPointer m_topContext;
      OverloadResolver::Parameter m_baseType;
      bool m_isOperator;
      QList< DeclarationWithArgument > m_declarations; //Declarations are paired with the optional first argument for the declared functions
      OverloadResolver::ParameterList m_knownParameters;
      QMap<KDevelop::Declaration*, int> m_argumentCountMap; //Maps how many pre-defined arguments were given to which function
      QualifiedIdentifier m_identifierForADL;
      OverloadResolver::Constness m_constness;
  };

}

#endif
