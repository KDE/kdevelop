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
#include "declaration_p.h"
#include "ducontext.h"

namespace KDevelop
{

class FunctionDeclarationPrivate : public DeclarationPrivate
{
  public:
    FunctionDeclarationPrivate()
    {
    }
    FunctionDeclarationPrivate( const FunctionDeclarationPrivate& rhs )
      :DeclarationPrivate( rhs )
    {
    }
};

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs) : Declaration(*new FunctionDeclarationPrivate( *rhs.d_func() )), AbstractFunctionDeclaration(rhs) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

FunctionDeclaration::FunctionDeclaration(const HashedString& url, const SimpleRange& range, Scope scope, DUContext* context)
  : Declaration(*new FunctionDeclarationPrivate, url, range, scope)
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

void FunctionDeclaration::setAbstractType(AbstractType::Ptr type) {
  Q_ASSERT( !type || dynamic_cast<FunctionType*>(type.data()) );
  Declaration::setAbstractType(type);
}

QString FunctionDeclaration::toString() const {
  if( !abstractType() )
    return Declaration::toString();

  KSharedPtr<FunctionType> function = type<FunctionType>();
  Q_ASSERT(function);
  
  return QString("function %1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn )).arg(identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
}

}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
