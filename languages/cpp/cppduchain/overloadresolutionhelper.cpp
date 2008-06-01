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

#include "overloadresolutionhelper.h"

#include <kdebug.h>

#include <duchain/declaration.h>
#include <duchain/ducontext.h>
#include <duchain/identifiedtype.h>

#include <cpptypes.h>

#include "typeutils.h"
#include "viablefunctions.h"
#include "cppduchain.h"

using namespace KDevelop;
using namespace Cpp;

OverloadResolutionFunction::OverloadResolutionFunction() : matchedArguments(0) {
}

OverloadResolutionFunction::OverloadResolutionFunction( int _matchedArguments, const ViableFunction& _viable ) : matchedArguments(_matchedArguments), function(_viable) {
}

OverloadResolutionHelper::OverloadResolutionHelper(const DUContextPointer& context, const KDevelop::TopDUContextPointer& topContext) : m_context(context), m_topContext(topContext), m_isOperator(false)
{
}

void OverloadResolutionHelper::setOperator( const OverloadResolver::Parameter& base, const QString& operatorName )
{
  m_baseType = base;
  m_isOperator = true;
  m_operatorIdentifier = Identifier("operator"+operatorName);
}

void OverloadResolutionHelper::setFunctions( const QList<Declaration*>& functions )
{
    foreach( Declaration* decl, functions )
      m_declarations << DeclarationWithArgument( OverloadResolver::ParameterList(), decl ); //Insert with argument-offset zero
}

void OverloadResolutionHelper::setKnownParameters( const OverloadResolver::ParameterList& parameters )
{
  m_knownParameters = parameters;
}

QList<OverloadResolutionFunction> OverloadResolutionHelper::resolve(bool partial)
{
  QList<OverloadResolutionFunction> functions;

  if( m_isOperator ) {
    ///Search for member operators
    AbstractType::Ptr real( TypeUtils::realType(m_baseType.type, m_context->topContext()) );
    if( dynamic_cast<CppClassType*>( real.data() ) )
    {
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>( real.data() );
      if( idType && idType->declaration() ) {
        DUContext* ctx = idType->declaration()->logicalInternalContext(m_context->topContext());
        if( ctx ) {
          QList<Declaration*> decls = Cpp::findLocalDeclarations( ctx, m_operatorIdentifier, m_context->topContext() );
          foreach( Declaration* decl, decls )
            m_declarations << DeclarationWithArgument( OverloadResolver::ParameterList(), decl );
        } else {
          log( "no internal context found" );
        }
      } else {
          log( "type is not identified" );
      }
    }
    ///Search for static global operators
    QList<Declaration*> decls = m_context->findDeclarations(m_operatorIdentifier);
    foreach( Declaration* decl, decls ) {
      FunctionType* fun = dynamic_cast<FunctionType*>( decl->abstractType().data() );
      if( fun && fun->arguments().size() == 2 )
        m_declarations << DeclarationWithArgument( m_baseType, decl );
    }
  }else{
    //m_declarations should already be set by setFunctions(..)
  }
  
  QMap<Declaration*, int> m_argumentCountMap; //Maps how many pre-defined arguments were given to which function
  foreach( const DeclarationWithArgument& decl, m_declarations )
    m_argumentCountMap[decl.second] = decl.first.parameters.size();

  OverloadResolver resolv( m_context, m_topContext );

//   log( "functions given to overload-resolution:" );
//   foreach( const DeclarationWithArgument& declaration, m_declarations )
//     log( declaration.second->toString() );

//   log("parameters given to overload-resolution:");
//   lock.unlock();
//   foreach( ExpressionEvaluationResult result, m_knownArgumentTypes ) {
//     log( result.toString() );
//   }
//   lock.lock();


  QList< ViableFunction > viableFunctions;

  viableFunctions = resolv.resolveListOffsetted( m_knownParameters, m_declarations, partial );
  
  foreach( const ViableFunction& function, viableFunctions ) {
    if( function.declaration() && function.declaration()->abstractType() ) {
      functions << OverloadResolutionFunction( m_argumentCountMap[function.declaration().data()] + m_knownParameters.parameters.size(), function );
    }
  }
  
  return functions;
}

void OverloadResolutionHelper::log(const QString& str) const
{
  kDebug(9007) << "OverloadResolutionHelper: " << str;
}


