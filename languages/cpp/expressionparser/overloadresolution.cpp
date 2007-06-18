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
#include "overloadresolution.h"
#include "typeutils.h"
#include "duchain/ducontext.h"
#include "duchain/declaration.h"
#include "duchainbuilder/cpptypes.h"
using namespace Cpp;

OverloadResolver::OverloadResolver( DUContext* context ) : m_context(context), m_worstConversionRank(NoMatch) {
  
}

CppFunctionType* OverloadResolver::resolveConstructor( const ParameterList& params, bool implicit, bool noUserDefinedConversion ) {
  m_worstConversionRank = ExactMatch;
  
    QList<Declaration*> declarations = m_context->localDeclarations();
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      if( (*it)->abstractType() ) {
        CppFunctionType* function = dynamic_cast<CppFunctionType*>( (*it)->abstractType().data() );
        if( function->kind() == CppFunctionType::Constructor ) {
          ///It is a constructor, check the parameters
          ///@todo finish

          
        }
      }
    }

    return 0;
}

ConversionRank OverloadResolver::worstConversionRank() {
  return m_worstConversionRank;
}
