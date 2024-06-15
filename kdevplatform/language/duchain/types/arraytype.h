/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ARRAYTYPE_H
#define KDEVPLATFORM_ARRAYTYPE_H

#include "abstracttype.h"

namespace KDevelop {
class ArrayTypeData;

class KDEVPLATFORMLANGUAGE_EXPORT ArrayType
    : public AbstractType
{
public:
    using Ptr = TypePtr<ArrayType>;

    /// Default constructor
    ArrayType();
    /// Copy constructor. \param rhs type to copy
    ArrayType(const ArrayType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit ArrayType(ArrayTypeData& data);
    /// Destructor
    ~ArrayType() override;

    ArrayType& operator=(const ArrayType& rhs) = delete;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    /**
     * Retrieve the dimension of this array type. Multiple-dimensioned
     * arrays will have another array type as their elementType().
     *
     * \returns the dimension of the array, or zero if the array is dimensionless (eg. int[])
     */
    int dimension () const;

    /**
     * Set this array type's dimension.
     * If @p dimension is zero, the array is considered dimensionless (eg. int[]).
     *
     * \param dimension new dimension, set to zero for a dimensionless type (eg. int[])
     */
    void setDimension(int dimension);

    /**
     * Retrieve the element type of the array, e.g. "int" for int[3].
     *
     * \returns the element type.
     */
    AbstractType::Ptr elementType () const;

    /**
     * Set the element type of the array, e.g. "int" for int[3].
     */
    void setElementType(const AbstractType::Ptr& type);

    QString toString() const override;

    size_t hash() const override;

    WhichType whichType() const override;

    void exchangeTypes(TypeExchanger* exchanger) override;

    enum {
        Identity = 7
    };

    using Data = ArrayTypeData;

protected:
    void accept0 (TypeVisitor* v) const override;

    TYPE_DECLARE_DATA(ArrayType)
};
}

#endif // KDEVPLATFORM_TYPESYSTEM_H
