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

namespace KDevelop {
class StructureTypeData;

using StructureTypeBase = MergeIdentifiedType<AbstractType>;

/**
 * \short A type representing structure types.
 *
 * StructureType represents all structures, including classes,
 * interfaces, etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT StructureType
    : public StructureTypeBase
{
public:
    using Ptr = TypePtr<StructureType>;

    /// Default constructor
    StructureType();
    /// Copy constructor. \param rhs type to copy
    StructureType(const StructureType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit StructureType(StructureTypeData& data);
    /// Destructor
    ~StructureType() override;

    StructureType& operator=(const StructureType& rhs) = delete;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    QString toString() const override;

    uint hash() const override;

    WhichType whichType() const override;

    //virtual void exchangeTypes(KDevelop::TypeExchanger*);

    enum {
        Identity = 6
    };

    using Data = StructureTypeData;

protected:
    void accept0 (TypeVisitor* v) const override;

    TYPE_DECLARE_DATA(StructureType)
};

template <>
inline StructureType* fastCast<StructureType*>(AbstractType* from)
{
    if (!from || from->whichType() != AbstractType::TypeStructure)
        return nullptr;
    else
        return static_cast<StructureType*>(from);
}
}

#endif
