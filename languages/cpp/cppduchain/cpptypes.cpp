/* This file is part of KDevelop
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

#include "cpptypes.h"

#include <language/duchain/abstractfunctiondeclaration.h>
#include "templateparameterdeclaration.h"

using namespace KDevelop;

//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(CppClassType);
REGISTER_TYPE(CppTemplateParameterType);

AbstractType* CppClassType::clone() const {
  return new CppClassType(*this);
}

CppClassType::CppClassType() : KDevelop::StructureType(createData<CppClassType>()) {
}

uint CppClassType::hash() const
{
  return 3 * StructureType::hash();
}

QString CppClassType::toString() const
{
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty()) {
    if(declarationId().specialization().index())
      return AbstractType::toString() + KDevelop::IndexedInstantiationInformation(declarationId().specialization()).information().applyToIdentifier(id).toString();
    else
    return AbstractType::toString() + id.toString();
  }

  //This path usually is not taken
  QString type = "class";

  return QString("<%1>%2").arg(type).arg(AbstractType::toString(true));
}


bool moreExpressiveThan(IntegralType* lhs, IntegralType* rhs) {
  bool ret = lhs->dataType() > rhs->dataType() && !((rhs->modifiers() & AbstractType::SignedModifier) && !(lhs->modifiers() & AbstractType::SignedModifier));
  if((rhs->modifiers() & AbstractType::LongLongModifier) && !(lhs->modifiers() & AbstractType::LongLongModifier))
    ret = false;
  if((rhs->modifiers() & AbstractType::LongModifier) && !(lhs->modifiers() & AbstractType::LongLongModifier) && !(lhs->modifiers() & AbstractType::LongModifier))
    ret = false;
  return ret;
}

AbstractType* CppTemplateParameterType::clone() const {
  return new CppTemplateParameterType(*this);
}

bool CppTemplateParameterType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppTemplateParameterType*>(_rhs))
    return false;
  const CppTemplateParameterType* rhs = static_cast<const CppTemplateParameterType*>(_rhs);

  if( this == rhs )
    return true;

  return IdentifiedType::equals(rhs) && AbstractType::equals(rhs);
}

TemplateParameterDeclaration* CppTemplateParameterType::declaration(const TopDUContext* top) const {
  return static_cast<TemplateParameterDeclaration*>(IdentifiedType::declaration(top));
}

QString CppTemplateParameterType::toString() const {
  return AbstractType::toString(false) + "<template> " + IdentifiedType::qualifiedIdentifier().toString();
}

void CppTemplateParameterType::accept0 (KDevelop::TypeVisitor *v) const {
    v->visit(this);
/*    v->endVisit(this);*/
}

uint CppTemplateParameterType::hash() const {
  return 41*IdentifiedType::hash() + AbstractType::hash();
}
