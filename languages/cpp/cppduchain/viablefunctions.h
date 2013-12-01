/* This file is part of KDevelop
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
#ifndef VIABLEFUNCTIONS_H
#define VIABLEFUNCTIONS_H

#include "overloadresolution.h"
#include "cppduchainexport.h"
#include <language/duchain/duchainpointer.h>


namespace KDevelop  {
  class Declaration;
  class AbstractFunctionDeclaration;
  class FunctionType;
}

///The here defined class is about finding best viable functions as defined in iso c++ draft 13.3.3

namespace Cpp {

  class TemplateDeclaration;

  using namespace KDevelop;

  class KDEVCPPDUCHAIN_EXPORT ViableFunction {
    public:

    explicit ViableFunction( TopDUContext* topContext = 0, Declaration* decl = 0,
                             OverloadResolver::Constness constness = OverloadResolver::Unknown,
                             bool noUserDefinedConversion = false );

    /**
     * Is it a valid function?
     * */
    bool isValid() const;

    /**
     * @param partial If this is true, the function is treated as if it had max. as many parameters as are given, so a match with only a part of the parameters is possible.
     * */
    void matchParameters( const OverloadResolver::ParameterList& params, bool partial = false );

    bool isBetter( const ViableFunction& other ) const;

    //Same as isBetter(..)
    bool operator< ( const ViableFunction& other ) const;

    uint worstConversion() const;

    /**
     * Is the function viable for the parameters given by matchParameters(...), as defined in iso c++ 13.3.2?
     * */
    bool isViable() const;

    KDevelop::DeclarationPointer declaration() const;

    struct ParameterConversion {
      ParameterConversion(int _rank=0, int _baseConversionLevels=0) : rank(_rank), baseConversionLevels(_baseConversionLevels) {
      }

      inline bool operator<(const ParameterConversion& rhs) const;

      ///The maximum value of rank is TypeConversion::MaximumConversionResult @see TypeConversion::ConversionRank
      int rank;
      int baseConversionLevels; ///@see TypeConversion::baseConversionLevels
    };

    ///Returns a list of structures that describe the conversion needed for each parameter of the function
    const KDevVarLengthArray<ParameterConversion>& parameterConversions() const;

    private:
    KDevVarLengthArray<ParameterConversion> m_parameterConversions;
    KDevelop::DeclarationPointer m_declaration;
    KDevelop::TopDUContextPointer m_topContext;
    TypePtr<KDevelop::FunctionType> m_type;
    KDevelop::AbstractFunctionDeclaration* m_funDecl;
    bool m_parameterCountMismatch, m_noUserDefinedConversion;
    OverloadResolver::Constness m_constness;
  };
}

#endif
