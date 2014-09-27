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

#include <language/duchain/declaration.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/types/identifiedtype.h>

#include <cpptypes.h>

#include "typeutils.h"
#include "viablefunctions.h"
#include "cppduchain.h"
#include "adlhelper.h"

using namespace KDevelop;
using namespace Cpp;

///@todo Decide whether this should be enabled or disabled (by performance considerations)
static const bool useADLForOperators = true;

OverloadResolutionFunction::OverloadResolutionFunction() : matchedArguments(0) {
}

OverloadResolutionFunction::OverloadResolutionFunction( int _matchedArguments, const ViableFunction& _viable ) : matchedArguments(_matchedArguments), function(_viable) {
}

OverloadResolutionHelper::OverloadResolutionHelper(const KDevelop::DUContextPointer& context, const KDevelop::TopDUContextPointer& topContext)
: m_context(context)
, m_topContext(topContext)
, m_isOperator(false)
, m_constness(OverloadResolver::Unknown)
{
}

void OverloadResolutionHelper::setFunctionNameForADL(const KDevelop::QualifiedIdentifier& identifierForADL)
{
  m_identifierForADL = identifierForADL;
}

void OverloadResolutionHelper::setOperator( const OverloadResolver::Parameter& base )
{
  m_baseType = base;
  m_isOperator = true;
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

void OverloadResolutionHelper::setConstness( OverloadResolver::Constness constness )
{
  m_constness = constness;
}

void OverloadResolutionHelper::initializeResolver(OverloadResolver& resolv)
{
  if( m_isOperator ) {
    Q_ASSERT(!m_identifierForADL.isEmpty());
    ///Search for member operators
    AbstractType::Ptr real( TypeUtils::realType(m_baseType.type, m_context->topContext()) );
    if( dynamic_cast<CppClassType*>( real.data() ) )
    {
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>( real.data() );
      if( idType && idType->declaration(m_context->topContext()) ) {
        DUContext* ctx = idType->declaration(m_context->topContext())->logicalInternalContext(m_context->topContext());
        if( ctx ) {
          QList<Declaration*> decls = Cpp::findLocalDeclarations( ctx, m_identifierForADL.first(), m_context->topContext() );
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
    QList<Declaration*> decls = m_context->findDeclarations(m_identifierForADL);
    foreach( Declaration* decl, decls ) {
      FunctionType::Ptr fun = decl->abstractType().cast<FunctionType>();
      if( fun && (fun->arguments().size() == 1 || fun->arguments().size() == 2) )
        m_declarations << DeclarationWithArgument( m_baseType, decl );
    }
    ///If no global or class member operators found try ADL for namespaced operators
    if (m_declarations.isEmpty() && useADLForOperators) {
      
      OverloadResolver::ParameterList params;
      params.parameters << m_baseType;
      params.parameters += m_knownParameters.parameters;
      
      foreach( Declaration* decl, resolv.computeADLCandidates( params, m_identifierForADL ) ) {
        FunctionType::Ptr fun = decl->abstractType().cast<FunctionType>();
        if( fun && (fun->arguments().size() == 1 || fun->arguments().size() == 2) )
          m_declarations << DeclarationWithArgument( m_baseType, decl );
      }          
    }
  }else{
    //m_declarations should already be set by setFunctions(..)
  }

  foreach( const DeclarationWithArgument& decl, m_declarations )
    m_argumentCountMap[decl.second] = decl.first.parameters.size();

//   log( "functions given to overload-resolution:" );
//   foreach( const DeclarationWithArgument& declaration, m_declarations )
//     log( declaration.second->toString() );

//   log("parameters given to overload-resolution:");
//   lock.unlock();
//   foreach( const ExpressionEvaluationResult& result, m_knownArgumentTypes ) {
//     log( result.toString() );
//   }
//   lock.lock();
}

QList<OverloadResolutionFunction> OverloadResolutionHelper::resolveToList(bool partial)
{
  OverloadResolver resolv( m_context, m_topContext, m_constness );

  QList< ViableFunction > viableFunctions;

  viableFunctions = resolv.resolveListOffsetted( m_knownParameters, m_declarations, partial );

  // also retrieve names by ADL if partial argument list (only used by code completion)
  // and even in strict mode if normal lookup failed
  if (partial || viableFunctions.empty() || !viableFunctions[0].isViable()) {
    
    QList<Declaration*> adlDecls = resolv.computeADLCandidates( m_knownParameters, m_identifierForADL );
    if (!adlDecls.empty()) {
      QList< DeclarationWithArgument > adlDeclsWithArguments;
      foreach(Declaration * decl, adlDecls) {
        adlDeclsWithArguments << DeclarationWithArgument( OverloadResolver::ParameterList(), decl ); // see setFunctions
      }
      viableFunctions += resolv.resolveListOffsetted( m_knownParameters, adlDeclsWithArguments, partial );
    }
  }

  std::sort(viableFunctions.begin(), viableFunctions.end());

  QList<OverloadResolutionFunction> ret;
  
  foreach( const ViableFunction& function, viableFunctions )
  {
    if( function.declaration() && function.declaration()->abstractType() )
      ret << OverloadResolutionFunction( m_argumentCountMap[function.declaration().data()] + m_knownParameters.parameters.size(), function );
  }

  return ret;
}

ViableFunction OverloadResolutionHelper::resolve(bool forceInstance)
{
  OverloadResolver resolv( m_context, m_topContext, m_constness, forceInstance );

  initializeResolver(resolv);
  
  ViableFunction ret = resolv.resolveListViable( m_knownParameters, m_declarations );

  // also retrieve names by ADL if partial argument list (only used by code completion)
  // and even in strict mode if normal lookup failed
  if (!ret.isViable()) {
    QList<Declaration*> adlDecls = resolv.computeADLCandidates( m_knownParameters, m_identifierForADL );
    if (!adlDecls.empty()) {
      QList< DeclarationWithArgument > adlDeclsWithArguments;
      foreach(Declaration * decl, adlDecls)
        adlDeclsWithArguments << DeclarationWithArgument( OverloadResolver::ParameterList(), decl ); // see setFunctions
      ret = resolv.resolveListViable( m_knownParameters, adlDeclsWithArguments );
    }
  }

  return ret;
}

void OverloadResolutionHelper::log(const QString& str) const
{
  kDebug(9007) << "OverloadResolutionHelper: " << str;
}


