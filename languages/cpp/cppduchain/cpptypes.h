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

#include <language/duchain/identifier.h>
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

struct KDEVCPPDUCHAIN_EXPORT CppClassTypeData : public KDevelop::StructureTypeData {
  CppClassTypeData() {
  }

  CppClassTypeData(const CppClassTypeData& rhs) :KDevelop::StructureTypeData(rhs)  {
  }

  ~CppClassTypeData() {
  }

  private:
    CppClassTypeData& operator=(const CppClassTypeData&) {
      return *this;
    }
};

// Currently, this type is only required to provide the correct toString() for specialized declarations.
class KDEVCPPDUCHAIN_EXPORT CppClassType : public KDevelop::StructureType
{
public:
  CppClassType(const CppClassType& rhs) : KDevelop::StructureType(copyData<CppClassType>(*rhs.d_func())) {
  }

  CppClassType(CppClassTypeData& data) : KDevelop::StructureType(data) {
  }

  typedef TypePtr<CppClassType> Ptr;

  CppClassType();

  virtual QString toString() const;

  virtual KDevelop::AbstractType* clone() const;

  virtual uint hash() const;

  // equals function does not need to be reimplemented

  // accept0 function does not need to be reimplemented

  // exchangeTypes function does not need to be reimplemented

  enum {
    Identity = 18
  };

  typedef CppClassTypeData Data;

protected:
  TYPE_DECLARE_DATA(CppClassType);
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
  CppTemplateParameterType(const CppTemplateParameterType& rhs) : CppTemplateParameterTypeBase(copyData<CppTemplateParameterType>(*rhs.d_func())) {
  }

  CppTemplateParameterType(CppTemplateParameterTypeData& data) : CppTemplateParameterTypeBase(data) {
  }

  CppTemplateParameterType() : CppTemplateParameterTypeBase(createData<CppTemplateParameterType>()) {
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

