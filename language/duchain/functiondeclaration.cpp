/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "functiondeclaration.h"
#include "ducontext.h"
#include "duchainregister.h"
#include "types/functiontype.h"

namespace KDevelop
{

REGISTER_DUCHAIN_ITEM(FunctionDeclaration);

DEFINE_LIST_MEMBER_HASH(FunctionDeclarationData, m_defaultParameters, IndexedString)

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data) : FunctionDeclarationBase(data) {
}

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data, const RangeInRevision& range) : FunctionDeclarationBase(data, range) {
}

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs) : FunctionDeclarationBase(*new FunctionDeclarationData( *rhs.d_func() )) {
}

FunctionDeclaration::FunctionDeclaration(const RangeInRevision& range, DUContext* context)
  : FunctionDeclarationBase(*new FunctionDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  if( context )
    setContext( context );
}

FunctionDeclaration::~FunctionDeclaration()
{
}

Declaration* FunctionDeclaration::clonePrivate() const {
  return new FunctionDeclaration(*this);
}

bool FunctionDeclaration::isFunctionDeclaration() const
{
  return true;
}

void FunctionDeclaration::setAbstractType(AbstractType::Ptr type) {
  if( type && !type.cast<FunctionType>() ) {
    kDebug() << "wrong type attached to function declaration:" << type->toString();
  }
  Declaration::setAbstractType(type);
}

QString FunctionDeclaration::toString() const {
  AbstractType::Ptr type = abstractType();
  if( !type )
    return Declaration::toString();

  TypePtr<FunctionType> function = type.cast<FunctionType>();
  if(function) {
    return QString("%1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn )).arg(identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
  }else{
    return Declaration::toString();
  }
}

uint FunctionDeclaration::additionalIdentity() const
{
  if(abstractType())
    return abstractType()->hash();
  else
    return 0;
}

const IndexedString* FunctionDeclaration::defaultParameters() const
{
  return d_func()->m_defaultParameters();
}

unsigned int FunctionDeclaration::defaultParametersSize() const
{
  return d_func()->m_defaultParametersSize();
}

void FunctionDeclaration::addDefaultParameter(const IndexedString& str)
{
  d_func_dynamic()->m_defaultParametersList().append(str);
}

void FunctionDeclaration::clearDefaultParameters()
{
  d_func_dynamic()->m_defaultParametersList().clear();
}

}
