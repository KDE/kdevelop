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

#include <QList>
#include "overloadresolution.h"
#include "cppexpressionparserexport.h"
#include <duchainpointer.h>


namespace KDevelop  {
  class Declaration;
}

///The here defined class is about finding best viable functions as defined in iso c++ draft 13.3.3

namespace Cpp {
  using namespace KDevelop;
  
  class KDEVCPPEXPRESSIONPARSER_EXPORT ViableFunction {
    public:
      
    ViableFunction( Declaration* decl = 0, bool noUserDefinedConversion = false );

    /**
     * Is it a valid function?
     * */
    bool isValid() const {
      return m_type && m_declaration;
    }

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
    
    private:
    QList<int> m_parameterConversions;
    KDevelop::DeclarationPointer m_declaration;
    CppFunctionType* m_type;
    bool m_parameterCountMismatch, m_noUserDefinedConversion;
  };  
}

#endif
