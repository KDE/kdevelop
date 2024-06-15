/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_POINTERTYPE_H
#define KDEVPLATFORM_POINTERTYPE_H

#include "abstracttype.h"

namespace KDevelop {
class PointerTypeData;

/**
 * \short A type representing pointer types.
 *
 * PointerType is used to represent types which hold a pointer to a location
 * in memory.
 */
class KDEVPLATFORMLANGUAGE_EXPORT PointerType
    : public AbstractType
{
public:
    using Ptr = TypePtr<PointerType>;

    /// Default constructor
    PointerType ();
    /// Copy constructor. \param rhs type to copy
    PointerType(const PointerType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit PointerType(PointerTypeData& data);
    /// Destructor
    ~PointerType() override;

    PointerType& operator=(const PointerType& rhs) = delete;

    /**
     * Sets the base type of the pointer, ie. what type of data the pointer points to.
     *
     * \param type the base type.
     */
    void setBaseType(const AbstractType::Ptr& type);

    /**
     * Retrieve the base type of the pointer, ie. what type of data the pointer points to.
     *
     * \returns the base type.
     */
    AbstractType::Ptr baseType () const;

    QString toString() const override;

    size_t hash() const override;

    WhichType whichType() const override;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    void exchangeTypes(TypeExchanger* exchanger) override;

    enum {
        Identity = 3
    };

    using Data = PointerTypeData;

protected:
    void accept0 (TypeVisitor* v) const override;

    TYPE_DECLARE_DATA(PointerType)
};
}

#endif
