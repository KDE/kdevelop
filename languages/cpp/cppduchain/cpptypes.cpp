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

#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/types/typeregister.h>
#include "templateparameterdeclaration.h"
#include <language/duchain/ducontext.h> //Only for FOREACH_ARRAY
#include "templatedeclaration.h"

using namespace KDevelop;

//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(CppTypeAliasType);
REGISTER_TYPE(CppTemplateParameterType);

AbstractType* CppTypeAliasType::clone() const {
  return new CppTypeAliasType(*this);
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

bool CppTypeAliasType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppTypeAliasType*>(_rhs))
    return false;
  const CppTypeAliasType* rhs = static_cast<const CppTypeAliasType*>(_rhs);

  if( this == rhs )
    return true;

  if( AbstractType::equals(rhs) && IdentifiedType::equals(rhs) )
  {
    if( (bool)d_func()->m_type != (bool)rhs->d_func()->m_type )
      return false;

    if( !d_func()->m_type )
      return true;

    return d_func()->m_type == rhs->d_func()->m_type;

  } else {
    return false;
  }
}

bool CppTemplateParameterType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const CppTemplateParameterType*>(_rhs))
    return false;
  const CppTemplateParameterType* rhs = static_cast<const CppTemplateParameterType*>(_rhs);

  if( this == rhs )
    return true;

  return IdentifiedType::equals(rhs);
}

AbstractType::Ptr CppTypeAliasType::type() const
{
  return d_func()->m_type.type();
}

void CppTypeAliasType::setType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_type = type->indexed();
}

uint CppTypeAliasType::hash() const
{
  return 31 * IdentifiedType::hash() + (type() ? type()->hash() + 83 : 0);
}

QString CppTypeAliasType::toString() const
{
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty())
    return id.toString();

  if (type())
    return type()->toString();

  return "typedef <notype>";
}

TemplateParameterDeclaration* CppTemplateParameterType::declaration(const TopDUContext* top) const {
  return static_cast<TemplateParameterDeclaration*>(IdentifiedType::declaration(top));
}

QString CppTemplateParameterType::toString() const {
  return "<template> " + IdentifiedType::qualifiedIdentifier().toString();
}

void CppTemplateParameterType::accept0 (KDevelop::TypeVisitor *v) const {
    v->visit(this);
/*    v->endVisit(this);*/
}

uint CppTemplateParameterType::hash() const {
  return 41*IdentifiedType::hash();
}
