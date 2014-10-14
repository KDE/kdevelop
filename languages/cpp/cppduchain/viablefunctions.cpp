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

#include "viablefunctions.h"
#include "cppduchain/typeutils.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include "templatedeclaration.h"
#include "typeconversion.h"
#include "debug.h"

#define ifDebug(x)
// #define ifDebug(x) x

using namespace Cpp;

///@todo prefer more specialized template-functions above less specialized ones

inline bool ViableFunction::ParameterConversion::operator<(const ParameterConversion& rhs) const {
  if( rank < rhs.rank )
    return true;
  else if( rank > rhs.rank )
    return false;
  else
    return baseConversionLevels > rhs.baseConversionLevels; //Conversion-rank is same, so use the base-conversion levels for ranking
}

ViableFunction::ViableFunction( TopDUContext* topContext, Declaration* decl,
                                OverloadResolver::Constness constness, bool noUserDefinedConversion )
: m_declaration(decl)
, m_topContext(topContext)
, m_type(0)
, m_parameterCountMismatch(true)
, m_noUserDefinedConversion(noUserDefinedConversion)
, m_constness(constness)
{
  if( decl )
    m_type = decl->abstractType().cast<KDevelop::FunctionType>();
  m_funDecl = dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data());
}

KDevelop::DeclarationPointer ViableFunction::declaration() const {
  return m_declaration;
}

bool ViableFunction::isValid() const {
  return m_type && m_declaration && m_funDecl;
}

void ViableFunction::matchParameters( const OverloadResolver::ParameterList& params, bool partial ) {
  if( !isValid() || !m_topContext )
    return;
  Q_ASSERT(m_funDecl);

  uint functionArgumentCount = m_type->indexedArgumentsSize();
  bool hasVarArgs = false;
  if (functionArgumentCount) {
    hasVarArgs = TypeUtils::isVarArgs(m_type->indexedArguments()[functionArgumentCount-1].abstractType());
  }

  ifDebug(qCDebug(CPPDUCHAIN) << "matchParameters" << params << " to " << m_type->toString() << "partial:" << partial << "varargs" << hasVarArgs;)

  if (!hasVarArgs) {
    if( params.parameters.size() + m_funDecl->defaultParametersSize() < functionArgumentCount && !partial ) {
      return; //Not enough parameters + default-parameters
    }
    if( static_cast<uint>(params.parameters.size()) > functionArgumentCount ) {
      return; //Too many parameters
    }
  }

  m_parameterCountMismatch = false;
  //Match all parameters against the argument-type
  const IndexedType* arguments = m_type->indexedArguments();
  const IndexedType* argumentIt = arguments;

  TypeConversion conv(m_topContext.data());

  for( QList<OverloadResolver::Parameter>::const_iterator it = params.parameters.begin(); it != params.parameters.end(); ++it )  {
    ParameterConversion c;
    c.rank = conv.implicitConversion( (*it).type->indexed(), *argumentIt, (*it).lValue, m_noUserDefinedConversion );
    c.baseConversionLevels = conv.baseConversionLevels();
    m_parameterConversions << c;

    if (!hasVarArgs || argumentIt < arguments + functionArgumentCount - 1) {
      ++argumentIt;
    } // else keep argumentIt at last argument, i.e. vararg
  }
}

bool ViableFunction::operator< ( const ViableFunction& other ) const {
  return isBetter(other);
}

bool ViableFunction::isBetter( const ViableFunction& other ) const {
  if( !isViable() )
    return false;
  if( !other.isViable() )
    return true;

  ///iso c++ 13.3.3 - best viable function

  //Is one of our conversions worse than one of the other function's?

  int minParams = m_parameterConversions.size();
  if(other.m_parameterConversions.size() < minParams)
    minParams = other.m_parameterConversions.size();

  bool hadBetterConversion = false;
  for(int a = 0; a < minParams; ++a) {

    if( m_parameterConversions[a] < other.m_parameterConversions[a] )
      return false; //All this function's conversions must not be worse than the other function one's

    if( other.m_parameterConversions[a] < m_parameterConversions[a] )
      hadBetterConversion = true;
  }

  ///@todo any special measures when parameter-counts differ?

  if( hadBetterConversion )
    return true;

  /**Until now both functions have the same match-quality. Iso c++ says this is better when:
   * - this is a non-template function while other is one
   * - this is a template-function that is more specialized than other
   * - we are looking for a const function and we are const or vice-versa
   */
  if((m_constness == Cpp::OverloadResolver::Const && TypeUtils::isConstant(m_declaration->abstractType()))
    || (m_constness == Cpp::OverloadResolver::NonConst && !TypeUtils::isConstant(m_declaration->abstractType())))
  {
    return true;
  }
  if(!dynamic_cast<TemplateDeclaration*>(m_declaration.data()) && dynamic_cast<TemplateDeclaration*>(other.m_declaration.data()))
    return true;
//   if( m_type->isMoreSpecialized( other.m_type.data() ) )
//     return true;

  return false;
}

bool ViableFunction::isViable() const {
  if( !isValid() || m_parameterCountMismatch ) return false;

  for( int a = 0; a < m_parameterConversions.size(); ++a )
    if( !m_parameterConversions[a].rank )
      return false;

  return true;
}

uint ViableFunction::worstConversion() const {
  uint ret = (uint)-1;
  for( int a = 0; a < m_parameterConversions.size(); ++a )
    if( (uint) m_parameterConversions[a].rank < ret )
      ret *= m_parameterConversions[a].rank;

  if( ret == (uint)-1 )
    return 0;
  else
    return ret;
}

const KDevVarLengthArray<ViableFunction::ParameterConversion>& ViableFunction::parameterConversions() const {
  return m_parameterConversions;
}
