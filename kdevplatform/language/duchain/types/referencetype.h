/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_REFERENCETYPE_H
#define KDEVPLATFORM_REFERENCETYPE_H

#include "abstracttype.h"

namespace KDevelop {
class ReferenceTypeData;

/**
 * \short A type representing reference types.
 *
 * ReferenceType is used to represent types which hold a reference to a
 * variable.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ReferenceType
    : public AbstractType
{
public:
    using Ptr = TypePtr<ReferenceType>;

    /// Default constructor
    ReferenceType ();
    /// Copy constructor. \param rhs type to copy
    ReferenceType (const ReferenceType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit ReferenceType(ReferenceTypeData& data);
    /// Destructor
    ~ReferenceType() override;

    ReferenceType& operator=(const ReferenceType& rhs) = delete;

    /**
     * Retrieve the referenced type, ie. what type of data this type references.
     *
     * \returns the base type.
     */
    AbstractType::Ptr baseType () const;

    /**
     * Sets the referenced type, ie. what type of data this type references.
     *
     * \param baseType the base type.
     */
    void setBaseType(const AbstractType::Ptr& baseType);

    /**
     * Checks whether this type is an rvalue- or lvalue-reference type.
     *
     * \returns true for rvalue-references, false for lvalue-references
     */
    bool isRValue() const;

    /**
     * Sets whether this type is an rvalue- or lvalue-reference type.
     *
     * \param isRValue true for rvalue-references, false for lvalue-references
     */
    void setIsRValue(bool isRValue);

    QString toString() const override;

    size_t hash() const override;

    WhichType whichType() const override;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    void exchangeTypes(TypeExchanger* exchanger) override;

    enum {
        Identity = 4
    };

    using Data = ReferenceTypeData;

protected:
    void accept0 (TypeVisitor* v) const override;

    TYPE_DECLARE_DATA(ReferenceType)
};
}

#endif // KDEVPLATFORM_TYPESYSTEM_H
