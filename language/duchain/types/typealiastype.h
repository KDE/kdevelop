/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef KDEVPLATFORM_TYPEALIASTYPE_H
#define KDEVPLATFORM_TYPEALIASTYPE_H

#include "abstracttype.h"
#include "identifiedtype.h"
#include "typeregister.h"
#include <language/languageexport.h>

namespace KDevelop {

class TypeAliasTypeData;

typedef MergeIdentifiedType<AbstractType> TypeAliasTypeBase;

class KDEVPLATFORMLANGUAGE_EXPORT TypeAliasType : public TypeAliasTypeBase
{
public:
  typedef TypePtr<TypeAliasType> Ptr;

  TypeAliasType(const TypeAliasType& rhs) : TypeAliasTypeBase(copyData<TypeAliasType>(*rhs.d_func())) {
  }

  TypeAliasType(TypeAliasTypeData& data) : TypeAliasTypeBase(data) {
  }

  TypeAliasType() : TypeAliasTypeBase(createData<TypeAliasType>()) {
  }

  KDevelop::AbstractType::Ptr type() const;
  void setType(KDevelop::AbstractType::Ptr type);

  virtual uint hash() const override;

  virtual QString toString() const override;

//   virtual QString mangled() const;

  virtual void exchangeTypes(KDevelop::TypeExchanger* exchanger) override;

  virtual KDevelop::AbstractType* clone() const override;

  virtual bool equals(const KDevelop::AbstractType* rhs) const override;

  virtual KDevelop::AbstractType::WhichType whichType() const override;
  
  enum {
    Identity = 9
  };

  typedef TypeAliasTypeData Data;

protected:
  TYPE_DECLARE_DATA(TypeAliasType);

  virtual void accept0 (KDevelop::TypeVisitor *v) const override;
};

template<>
inline TypeAliasType* fastCast<TypeAliasType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeIntegral)
    return 0;
  else
    return static_cast<TypeAliasType*>(from);
}

}

#endif // KDEVPLATFORM_TYPEALIASTYPE_H
