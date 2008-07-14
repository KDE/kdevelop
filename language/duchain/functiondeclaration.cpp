/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>

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
#include "declarationdata.h"
#include "ducontext.h"
#include "duchainregister.h"
#include "language/duchain/types/functiontype.h"

namespace KDevelop
{

class FunctionDeclarationData : public DeclarationData
{
  public:
    FunctionDeclarationData()
    {
    }
    FunctionDeclarationData( const FunctionDeclarationData& rhs )
      :DeclarationData( rhs )
    {
    }
};

REGISTER_DUCHAIN_ITEM(FunctionDeclaration);

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data) : Declaration(data) {
}

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs) : Declaration(*new FunctionDeclarationData( *rhs.d_func() )), AbstractFunctionDeclaration(rhs) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

FunctionDeclaration::FunctionDeclaration(const SimpleRange& range, DUContext* context)
  : Declaration(*new FunctionDeclarationData, range)
{
  if( context )
    setContext( context );
}

FunctionDeclaration::~FunctionDeclaration()
{
}

Declaration* FunctionDeclaration::clone() const {
  return new FunctionDeclaration(*this);
}

bool FunctionDeclaration::isFunctionDeclaration() const
{
  return true;
}

void FunctionDeclaration::setAbstractType(AbstractType::Ptr type) {
  if( type && !type.cast<FunctionType>() )
    kDebug() << "wrong type attached to function declaration:" << type->toString();
  Declaration::setAbstractType(type);
}

QString FunctionDeclaration::toString() const {
  if( !abstractType() )
    return Declaration::toString();

  TypePtr<FunctionType> function = type<FunctionType>();
  Q_ASSERT(function);

  return QString("function %1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn )).arg(identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
}

uint FunctionDeclaration::additionalIdentity() const
{
  if(abstractType())
    return abstractType()->hash();
  else
    return 0;
}

}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
