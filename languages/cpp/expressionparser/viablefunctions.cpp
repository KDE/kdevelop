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

#include "viablefunctions.h"
#include "duchainbuilder/typeutils.h"
#include "duchain/ducontext.h"
#include "duchain/declaration.h"
#include "duchain/classfunctiondeclaration.h"
#include "duchainbuilder/cpptypes.h"

using namespace Cpp;

ViableFunction::ViableFunction( Declaration* decl, bool noUserDefinedConversion ) : m_declaration(decl), m_type(0), m_parameterCountMismatch(true), m_noUserDefinedConversion(noUserDefinedConversion) {
  if( decl )
    m_type = dynamic_cast<CppFunctionType*>( decl->abstractType().data());
}

KDevelop::DeclarationPointer ViableFunction::declaration() const {
  return m_declaration;
}

void ViableFunction::matchParameters( const OverloadResolver::ParameterList& params, bool partial ) {
  if( !isValid() )
    return;
  AbstractFunctionDeclaration* functionDecl = dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data());
  if( params.parameters.size() + functionDecl->defaultParameters().size() < m_type->arguments().size() && !partial )
    return; //Not enough parameters + default-parameters
  if( params.parameters.size() > m_type->arguments().size() )
    return; //Too many parameters

  m_parameterCountMismatch = false;
  //Match all parameters against the argument-type
  QList<AbstractType::Ptr>::const_iterator argumentIt = m_type->arguments().begin();

  for( QList<OverloadResolver::Parameter>::const_iterator it = params.parameters.begin(); it != params.parameters.end(); ++it )  {
    TypeConversion conv;
    m_parameterConversions << conv.implicitConversion(AbstractType::Ptr((*it).type), (*argumentIt), (*it).lValue, m_noUserDefinedConversion );
    ++argumentIt;
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

  Q_ASSERT( m_parameterConversions.size() == other.m_parameterConversions.size() );
  ///iso c++ 13.3.3 - best viable function

  //Is one of our conversions worse than one of the other function's?

  QList<int>::const_iterator myConversions = m_parameterConversions.begin();
  QList<int>::const_iterator otherConversions = other.m_parameterConversions.begin();

  bool hadBetterConversion = false;
  while( myConversions != m_parameterConversions.end() )
  {
    if( *myConversions < *otherConversions )
      return false; //All this function's conversions must not be worse then the other function one's

    if( *myConversions > *otherConversions )
      hadBetterConversion = true;

    ++myConversions;
    ++otherConversions;
  }

  if( hadBetterConversion )
    return true;

  /**Until now both functions have the same match-quality. Iso c++ says this is better when:
   * - this is a non-template function while other is one
   * - this is a template-function that is more specialized than other
   */
  if( !m_type->isTemplate() && other.m_type->isTemplate() )
    return true;
  if( m_type->isMoreSpecialized( other.m_type ) )
    return true;

  return false;
}

bool ViableFunction::isViable() const {
  if( !isValid() || m_parameterCountMismatch ) return false;

  for( QList<int>::const_iterator it = m_parameterConversions.begin(); it != m_parameterConversions.end(); ++it )
    if( !*it )
      return false;

  return true;
}

uint ViableFunction::worstConversion() const {
  uint ret = (uint)-1;
  for( QList<int>::const_iterator it = m_parameterConversions.begin(); it != m_parameterConversions.end(); ++it )
    if( (uint) *it < ret )
      ret *= *it;

  if( ret == (uint)-1 )
    return 0;
  else
    return ret;
}
