/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "classfunctiondeclaration.h"

#include "ducontext.h"
#include "typesystem.h"
#include "classmemberdeclaration_p.h"

namespace KDevelop
{
Identifier conversionIdentifier("operator{...cast...}");

class ClassFunctionDeclarationPrivate : public ClassMemberDeclarationPrivate
{
public:
  ClassFunctionDeclarationPrivate() {
    m_functionType = ClassFunctionDeclaration::Normal;
  }
  ClassFunctionDeclarationPrivate( const ClassFunctionDeclarationPrivate& rhs )
      : ClassMemberDeclarationPrivate( rhs )
  {
    m_functionType = rhs.m_functionType;
  }
  ClassFunctionDeclaration::QtFunctionType m_functionType;
};

ClassFunctionDeclaration::ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs) 
    : ClassMemberDeclaration(*new ClassFunctionDeclarationPrivate( *rhs.d_func() ))
      , AbstractFunctionDeclaration(rhs) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

void ClassFunctionDeclaration::setAbstractType(AbstractType::Ptr type) {
  if(!( !type || dynamic_cast<FunctionType*>(type.data()) ))
    kWarning(9505) << "WARNING: Non-function type assigned to function declaration";
  ClassMemberDeclaration::setAbstractType(type);
}

ClassFunctionDeclaration::ClassFunctionDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context)
  : ClassMemberDeclaration(*new ClassFunctionDeclarationPrivate, url, range), AbstractFunctionDeclaration()
{
  if( context )
    setContext( context );
}

Declaration* ClassFunctionDeclaration::clone() const {
  return new ClassFunctionDeclaration(*this);
}

ClassFunctionDeclaration::~ClassFunctionDeclaration()
{
}

bool ClassFunctionDeclaration::isFunctionDeclaration() const
{
  return true;
}

QString ClassFunctionDeclaration::toString() const {
  if( !abstractType() )
    return ClassMemberDeclaration::toString();

  TypePtr<FunctionType> function = type<FunctionType>();
  if(function) {
    return QString("member-function %1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn )).arg(identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
  } else {
    QString type = abstractType() ? abstractType()->toString() : QString("<notype>");
    kDebug(9505) << "A function has a bad type attached:" << type;
    return QString("invalid member-function %1 type %2").arg(identifier().toString()).arg(type);
  }
}


/*bool ClassFunctionDeclaration::isSimilar(KDevelop::CodeItem *other, bool strict ) const
{
  if (!CppClassMemberType::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<ClassFunctionDeclaration*>(other);

  if (isConstant() != func->isConstant())
    return false;

  if (arguments().count() != func->arguments().count())
    return false;

  for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

  return true;
}*/

ClassFunctionDeclaration::QtFunctionType ClassFunctionDeclaration::functionType() const
{
  return d_func()->m_functionType;
}

void ClassFunctionDeclaration::setFunctionType(QtFunctionType functionType)
{
  d_func()->m_functionType = functionType;
}

bool ClassFunctionDeclaration::isConversionFunction() const {
  return identifier() == conversionIdentifier;
}

bool ClassFunctionDeclaration::isConstructor() const
{
  DUContext* ctx = context();
  if (ctx && ctx->type() == DUContext::Class && ctx->localScopeIdentifier().top().nameEquals(identifier()))
    return true;
  return false;
}

bool ClassFunctionDeclaration::isDestructor() const
{
  DUContext* ctx = context();
  QString id = identifier().toString();
  return ctx && ctx->type() == DUContext::Class && id.startsWith('~') && id.mid(1) == ctx->localScopeIdentifier().top().toString();
}

uint ClassFunctionDeclaration::additionalIdentity() const
{
  if(abstractType())
    return abstractType()->hash();
  else
    return 0;
}

}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
