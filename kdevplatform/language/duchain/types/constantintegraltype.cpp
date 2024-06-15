/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "constantintegraltype.h"

#include <debug.h>
#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {
REGISTER_TYPE(ConstantIntegralType);

ConstantIntegralType::ConstantIntegralType(const ConstantIntegralType& rhs)
    : IntegralType(copyData<ConstantIntegralType>(*rhs.d_func()))
{
}

ConstantIntegralType::ConstantIntegralType(ConstantIntegralTypeData& data)
    : IntegralType(data)
{
}

ConstantIntegralType::ConstantIntegralType(uint type)
    : IntegralType(createData<ConstantIntegralType>())
{
    setDataType(type);
    setModifiers(ConstModifier);
}

qint64 ConstantIntegralType::plainValue() const
{
    return d_func()->m_value;
}

AbstractType* ConstantIntegralType::clone() const
{
    return new ConstantIntegralType(*this);
}

bool ConstantIntegralType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!IntegralType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const ConstantIntegralType*>(_rhs));
    const auto* rhs = static_cast<const ConstantIntegralType*>(_rhs);

    return d_func()->m_value == rhs->d_func()->m_value;
}

QString ConstantIntegralType::toString() const
{
    QString ret;

    switch (dataType()) {
    case TypeNone:
        ret = QStringLiteral("none");
        break;
    case TypeChar:
        ret = QStringLiteral("char");
        break;
    case TypeWchar_t:
        ret = QStringLiteral("wchar_t");
        break;
    case TypeChar16_t:
        ret = QStringLiteral("char16_t");
        break;
    case TypeChar32_t:
        ret = QStringLiteral("char32_t");
        break;
    case TypeBoolean:
        ret = d_func()->m_value ? QStringLiteral("true") : QStringLiteral("false");
        break;
    case TypeInt:
        ret = (modifiers() & UnsignedModifier) ? QStringLiteral("unsigned") : QStringLiteral("int");
        break;
    case TypeHalf:
        ret = QStringLiteral("half");
        break;
    case TypeFloat:
        ret = QStringLiteral("float");
        break;
    case TypeDouble:
        ret = QStringLiteral("double");
        break;
    case TypeVoid:
        ret = QStringLiteral("void");
        break;
    default:
        ret = QStringLiteral("<unknown_value>");
        break;
    }

    return ret;
}

QString ConstantIntegralType::valueAsString() const
{
    switch (dataType()) {
    case TypeNone:
        return QStringLiteral("none");
    case TypeChar:
        return QString::number(( char )d_func()->m_value);
    case TypeWchar_t:
        return QString::number(( wchar_t )d_func()->m_value);
    case TypeChar16_t:
        return QString::number(( char16_t )d_func()->m_value);
    case TypeChar32_t:
        return QString::number(( char32_t )d_func()->m_value);
    case TypeBoolean:
        return d_func()->m_value ? QStringLiteral("true") : QStringLiteral("false");
    case TypeInt:
        return (modifiers() & UnsignedModifier) ?
               QString::number(( uint )d_func()->m_value) :
               QString::number(( int )d_func()->m_value);
    case TypeHalf:
    case TypeFloat:
        return QString::number(value<float>());
    case TypeDouble:
        return QString::number(value<double>());
    default:
        return QString();
    }
}

size_t ConstantIntegralType::hash() const
{
    return KDevHash(IntegralType::hash()) << d_func()->m_value;
}

template <>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<qint64>(qint64 value)
{
    if ((modifiers() & UnsignedModifier)) {
        qCWarning(LANGUAGE) << "setValue(signed) called on unsigned type";
    }
    d_func_dynamic()->m_value = value;
}

template <>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<quint64>(quint64 value)
{
    if (!(modifiers() & UnsignedModifier)) {
        qCWarning(LANGUAGE) << "setValue(unsigned) called on not unsigned type";
    }
    d_func_dynamic()->m_value = ( qint64 )value;
}

template <>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<float>(float value)
{
    if (dataType() != TypeFloat) {
        qCWarning(LANGUAGE) << "setValue(float) called on non-float type";
    }
    memcpy(&d_func_dynamic()->m_value, &value, sizeof(float));
}

template <>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<double>(double value)
{
    if (dataType() != TypeDouble) {
        qCWarning(LANGUAGE) << "setValue(double) called on non-double type";
    }
    memcpy(&d_func_dynamic()->m_value, &value, sizeof(double));
}
}
