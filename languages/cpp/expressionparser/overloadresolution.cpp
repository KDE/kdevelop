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
#include "duchain/classfunctiondeclaration.h"
#include "duchainbuilder/cpptypes.h"
#include "typeutils.h"
#include "viablefunctions.h"

using namespace Cpp;
using namespace KDevelop;

OverloadResolver::OverloadResolver( DUContext* context ) : m_context(context), m_worstConversionRank(NoMatch) {
  
}

Declaration* OverloadResolver::resolveConstructor( const ParameterList& params, bool implicit, bool noUserDefinedConversion ) {

    QList<Declaration*> goodDeclarations;
    QList<Declaration*> declarations = m_context->localDeclarations();
    
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      if( (*it)->abstractType() )
      {
        CppFunctionType* function = dynamic_cast<CppFunctionType*>( (*it)->abstractType().data() );
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>(*it);
        Q_ASSERT(functionDeclaration);
        
        if( functionDeclaration->isConstructor() )
        {
          if( function->arguments().size() >= params.parameters.size() )
          {
            if( !implicit || !functionDeclaration->isExplicit() )
              goodDeclarations << *it;
          }
        }
      }
    }

    return resolveInternal( params, goodDeclarations, noUserDefinedConversion );
}

Declaration* OverloadResolver::resolve( const ParameterList& params, const QualifiedIdentifier& functionName, bool noUserDefinedConversion )
{
  QList<Declaration*> declarations = m_context->findDeclarations(functionName);
  
  return resolveInternal(params, declarations, noUserDefinedConversion );
}

uint OverloadResolver::worstConversionRank() {
  return m_worstConversionRank;
}

Declaration* OverloadResolver::resolveInternal( const ParameterList& params, const QList<Declaration*>& declarations, bool noUserDefinedConversion ) {
  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;
  
  ViableFunction bestViableFunction;

  ///First step: Find classes in the declaration-list. If there are classes, insert all their constructors.
  QList<Declaration*> newDeclarations = declarations;
  
  for( QList<Declaration*>::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    Declaration* decl = *it;

    if( CppClassType* klass = dynamic_cast<CppClassType*>( decl->abstractType().data() ) ) {
      TypeUtils::getConstructors( klass, newDeclarations );
    }
  }

  ///Second step: Find best viable function
  for( QList<Declaration*>::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    ViableFunction viable( *it , noUserDefinedConversion );
    viable.matchParameters( params );
    
    if( viable.isBetter(bestViableFunction) ) {
      bestViableFunction = viable;
      m_worstConversionRank = bestViableFunction.worstConversion();
    }
  }

  if( bestViableFunction.isViable() )
    return bestViableFunction.declaration();
  else
    return 0;
}

