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
#include "duchain/ducontext.h"
#include "duchain/topducontext.h"
#include "duchain/declaration.h"
#include "duchain/classfunctiondeclaration.h"
#include "cppduchain/cpptypes.h"
#include "templatedeclaration.h"

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

ViableFunction::ViableFunction( TopDUContext* topContext, Declaration* decl, bool noUserDefinedConversion ) : m_declaration(decl), m_topContext(topContext), m_type(0), m_parameterCountMismatch(true), m_noUserDefinedConversion(noUserDefinedConversion) {
  if( decl )
    m_type = KSharedPtr<CppFunctionType>(fastCast<CppFunctionType*>( decl->abstractType().data()));
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
  if( params.parameters.size() + m_funDecl->defaultParameters().size() < m_type->arguments().size() && !partial )
    return; //Not enough parameters + default-parameters
  if( params.parameters.size() > m_type->arguments().size() )
    return; //Too many parameters

  m_parameterCountMismatch = false;
  //Match all parameters against the argument-type
  QList<AbstractType::Ptr>::const_iterator argumentIt = m_type->arguments().begin();

  for( QList<OverloadResolver::Parameter>::const_iterator it = params.parameters.begin(); it != params.parameters.end(); ++it )  {
    TypeConversion conv(m_topContext.data());
    ParameterConversion c;
    c.rank = conv.implicitConversion(AbstractType::Ptr((*it).type), (*argumentIt), (*it).lValue, m_noUserDefinedConversion );
    c.baseConversionLevels = conv.baseConversionLevels();
    m_parameterConversions << c;
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

  ///iso c++ 13.3.3 - best viable function

  //Is one of our conversions worse than one of the other function's?

  QList<ParameterConversion>::const_iterator myConversions = m_parameterConversions.begin();
  QList<ParameterConversion>::const_iterator otherConversions = other.m_parameterConversions.begin();

  bool hadBetterConversion = false;
  while( myConversions != m_parameterConversions.end() && otherConversions != other.m_parameterConversions.end() )
  {
    if( *myConversions < *otherConversions )
      return false; //All this function's conversions must not be worse then the other function one's

    if( *otherConversions < *myConversions )
      hadBetterConversion = true;

    ++myConversions;
    ++otherConversions;
  }

  ///@todo any special measures when parameter-counts differ?

  if( hadBetterConversion )
    return true;

  /**Until now both functions have the same match-quality. Iso c++ says this is better when:
   * - this is a non-template function while other is one
   * - this is a template-function that is more specialized than other
   */
  if(!dynamic_cast<TemplateDeclaration*>(m_declaration.data()) && dynamic_cast<TemplateDeclaration*>(other.m_declaration.data()))
    return true;
//   if( m_type->isMoreSpecialized( other.m_type.data() ) )
//     return true;

  return false;
}

bool ViableFunction::isViable() const {
  if( !isValid() || m_parameterCountMismatch ) return false;

  for( QList<ParameterConversion>::const_iterator it = m_parameterConversions.begin(); it != m_parameterConversions.end(); ++it )
    if( !(*it).rank )
      return false;

  return true;
}

uint ViableFunction::worstConversion() const {
  uint ret = (uint)-1;
  for( QList<ParameterConversion>::const_iterator it = m_parameterConversions.begin(); it != m_parameterConversions.end(); ++it )
    if( (uint) (*it).rank < ret )
      ret *= (*it).rank;

  if( ret == (uint)-1 )
    return 0;
  else
    return ret;
}

const QList<ViableFunction::ParameterConversion>& ViableFunction::parameterConversions() const {
  return m_parameterConversions;
}
