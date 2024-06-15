/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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

    size_t hash() const override;

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
}

#endif
