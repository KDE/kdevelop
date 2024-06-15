/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CONSTANTINTEGRALTYPE_H
#define KDEVPLATFORM_CONSTANTINTEGRALTYPE_H

#include "integraltype.h"
#include "typesystemdata.h"

namespace KDevelop {
template <typename T>
T constant_value(const qint64* realval)
{
    T value;
    memcpy(&value, realval, sizeof(T));
    return value;
}

class KDEVPLATFORMLANGUAGE_EXPORT ConstantIntegralType
    : public IntegralType
{
public:
    ConstantIntegralType(const ConstantIntegralType& rhs);

    explicit ConstantIntegralType(ConstantIntegralTypeData& data);

    explicit ConstantIntegralType(uint type = TypeNone);

    ConstantIntegralType& operator=(const ConstantIntegralType& rhs) = delete;

    using Ptr = TypePtr<ConstantIntegralType>;

    /**The types and modifiers are not changed!
     * The values are casted internally to the local representation, so you can lose precision.
     * */
    template <class ValueType>
    void setValue(ValueType value)
    {
        if (AbstractType::modifiers() & UnsignedModifier)
            setValueInternal<quint64>(value);
        else if (IntegralType::dataType() == TypeHalf)
            setValueInternal<float>(value);
        else if (IntegralType::dataType() == TypeFloat)
            setValueInternal<float>(value);
        else if (IntegralType::dataType() == TypeDouble)
            setValueInternal<double>(value);
        else
            setValueInternal<qint64>(value);
    }

    /**
     * For booleans, the value is 1 for true, and 0 for false.
     * All signed values should be retrieved and set through value(),
     *
     * */
    template <class ValueType>
    ValueType value() const
    {
        if (modifiers() & UnsignedModifier) {
            return constant_value<quint64>(&d_func()->m_value);
        } else if (dataType() == TypeHalf) {
            return constant_value<float>(&d_func()->m_value);
        } else if (dataType() == TypeFloat) {
            return constant_value<float>(&d_func()->m_value);
        } else if (dataType() == TypeDouble) {
            return constant_value<double>(&d_func()->m_value);
        } else {
            return constant_value<qint64>(&d_func()->m_value);
        }
    }

    qint64 plainValue() const;

    QString toString() const override;

    QString valueAsString() const;

    bool equals(const KDevelop::AbstractType* rhs) const override;

    KDevelop::AbstractType* clone() const override;

    size_t hash() const override;

    enum {
        Identity = 14
    };

    using Data = ConstantIntegralTypeData;

protected:
    TYPE_DECLARE_DATA(ConstantIntegralType);

private:
    //Sets the value without casting
    template <class ValueType>
    void setValueInternal(ValueType value);
};
}

#endif // KDEVPLATFORM_CONSTANTINTEGRALTYPE_H
