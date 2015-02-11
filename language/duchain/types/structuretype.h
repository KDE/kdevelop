/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVPLATFORM_STRUCTURETYPE_H
#define KDEVPLATFORM_STRUCTURETYPE_H

#include "abstracttype.h"
#include "identifiedtype.h"
#include "typesystemdata.h"

namespace KDevelop
{
class StructureTypeData;

typedef MergeIdentifiedType<AbstractType> StructureTypeBase;

/**
 * \short A type representing structure types.
 *
 * StructureType represents all structures, including classes,
 * interfaces, etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT StructureType : public StructureTypeBase
{
public:
  typedef TypePtr<StructureType> Ptr;

  /// Default constructor
  StructureType();
  /// Copy constructor. \param rhs type to copy
  StructureType(const StructureType& rhs);
  /// Constructor using raw data. \param data internal data.
  StructureType(StructureTypeData& data);
  /// Destructor
  virtual ~StructureType();

  virtual AbstractType* clone() const override;

  virtual bool equals(const AbstractType* rhs) const override;

  virtual QString toString() const override;

  virtual uint hash() const override;

  virtual WhichType whichType() const override;

  //virtual void exchangeTypes(KDevelop::TypeExchanger*);

  enum {
    Identity = 6
  };

  typedef StructureTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const override;

  TYPE_DECLARE_DATA(StructureType)
};

template<>
inline StructureType* fastCast<StructureType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeStructure)
    return 0;
  else
    return static_cast<StructureType*>(from);
}

}

#endif
