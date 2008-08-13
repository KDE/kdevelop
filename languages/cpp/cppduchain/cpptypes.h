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

   You should have received a copy of the GNU Library Genera {
    return d_func()->m_value;
  }l Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPTYPES_H
#define CPPTYPES_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPair>
#include <util/kdevvarlengtharray.h>

#include <language/duchain/identifier.h>
#include <language/duchain/types/typesystemdata.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/identifiedtype.h>
#include "cppduchainexport.h"
#include <language/duchain/types/alltypes.h>
#include <language/duchain/types/typesystem.h>

namespace KDevelop
{
class DUContext;
class Declaration;
}

template<typename T>
T constant_value(const qint64* realval)
{
  T value;
  memcpy(&value, realval, sizeof(T));
  return value;
}

//A simple hack that tries taking the modifiers and the type itself into account
bool KDEVCPPDUCHAIN_EXPORT moreExpressiveThan(KDevelop::IntegralType* lhs, KDevelop::IntegralType* rhs);

class TemplateParameterDeclaration;

typedef KDevelop::MergeIdentifiedType<KDevelop::AbstractType> CppTypeAliasTypeBase;

struct CppTypeAliasTypeData : public CppTypeAliasTypeBase::Data {
  KDevelop::IndexedType m_type;
};

///@todo remove
class KDEVCPPDUCHAIN_EXPORT CppTypeAliasType : public CppTypeAliasTypeBase
{
public:
  typedef TypePtr<CppTypeAliasType> Ptr;

  CppTypeAliasType(const CppTypeAliasType& rhs) : CppTypeAliasTypeBase(copyData<CppTypeAliasTypeData>(*rhs.d_func())) {
  }

  CppTypeAliasType(CppTypeAliasTypeData& data) : CppTypeAliasTypeBase(data) {
  }

  CppTypeAliasType() : CppTypeAliasTypeBase(createData<CppTypeAliasTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppTypeAliasType>();
  }

  KDevelop::AbstractType::Ptr type() const;
  void setType(KDevelop::AbstractType::Ptr type);

  virtual uint hash() const;

  virtual QString toString() const;

//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  enum {
    Identity = 19
  };

  typedef CppTypeAliasTypeData Data;

protected:
  TYPE_DECLARE_DATA(CppTypeAliasType);

  virtual void accept0 (KDevelop::TypeVisitor *v) const
  {
    if (v->visit (this))
      acceptType (d_func()->m_type.type(), v);

    //v->endVisit (this);
  }
};

/**
 * This class represents a template-parameter on the type-level(it is strictly attached to a template-declaration)
 * This is only attached to unset template-parameters. Once the template-parameters are set, the TemplateDeclarations
 * will return the real set types as abstractType().
 * This means that when you encounter this type, it means that the template-parameter is not set.
 * */

typedef KDevelop::MergeIdentifiedType<KDevelop::AbstractType> CppTemplateParameterTypeBase;

typedef CppTemplateParameterTypeBase::Data CppTemplateParameterTypeData;

class KDEVCPPDUCHAIN_EXPORT CppTemplateParameterType : public CppTemplateParameterTypeBase
{
public:
  CppTemplateParameterType(const CppTemplateParameterType& rhs) : CppTemplateParameterTypeBase(copyData<CppTemplateParameterTypeData>(*rhs.d_func())) {
  }

  CppTemplateParameterType(CppTemplateParameterTypeData& data) : CppTemplateParameterTypeBase(data) {
  }

  CppTemplateParameterType() : CppTemplateParameterTypeBase(createData<CppTemplateParameterTypeData>()) {
    d_func_dynamic()->setTypeClassId<CppTemplateParameterType>();
  }

  typedef TypePtr<CppTemplateParameterType> Ptr;

  TemplateParameterDeclaration* declaration(const KDevelop::TopDUContext* top) const;

  virtual QString toString() const;
//   virtual QString mangled() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual uint hash() const;

  enum {
    Identity = 23
  };

  typedef CppTemplateParameterTypeData Data;

  protected:
  virtual void accept0 (KDevelop::TypeVisitor *v) const;
  TYPE_DECLARE_DATA(CppTemplateParameterType);
};

namespace KDevelop {

template<>
inline CppTemplateParameterType* fastCast<CppTemplateParameterType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeAbstract)
    return 0;
  else
    return dynamic_cast<CppTemplateParameterType*>(static_cast<CppTemplateParameterType*>(from));
}

}


#endif // CPPTYPES_H

