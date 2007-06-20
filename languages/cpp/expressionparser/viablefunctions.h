/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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
#include <QList>
#include "overloadresolution.h"

namespace KDevelop  {
  class ClassFunctionDeclaration;
}

///The here defined class is about finding best viable functions as defined in iso c++ draft 13.3.3

namespace Cpp {
  using namespace KDevelop;
  
  class ViableFunction {
    public:
      
    ViableFunction( Declaration* decl = 0, bool noUserDefinedConversion = false );

    /**
     * Is it a valid function?
     * */
    bool isValid() const {
      return m_type && m_declaration;
    }

    void matchParameters( const OverloadResolver::ParameterList& params );

    bool isBetter( const ViableFunction& other ) const;

    uint worstConversion() const;
    
    /**
     * Is the function viable for the parameters given by matchParameters(...), as defined in iso c++ 13.3.2?
     * */
    bool isViable() const;

    ClassFunctionDeclaration* declaration() const {
      return m_declaration;
    }
    
    private:
    QList<int> m_parameterConversions;
    ClassFunctionDeclaration* m_declaration;
    CppFunctionType* m_type;
    bool m_parameterCountMismatch, m_noUserDefinedConversion;
  };  
}
