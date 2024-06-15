/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLASS_SPECIALIZATION_TYPE_H
#define CLASS_SPECIALIZATION_TYPE_H

#include <language/duchain/identifier.h>
#include <language/duchain/types/structuretype.h>

using KDevelop::IndexedType;

DECLARE_LIST_MEMBER_HASH(ClassSpecializationTypeData, parameters, IndexedType)

struct ClassSpecializationTypeData : public KDevelop::StructureTypeData
{
    ClassSpecializationTypeData();

    ClassSpecializationTypeData(const ClassSpecializationTypeData& rhs);

    ~ClassSpecializationTypeData();

    ClassSpecializationTypeData& operator=(const ClassSpecializationTypeData&) = delete;

    START_APPENDED_LISTS_BASE(ClassSpecializationTypeData, StructureTypeData);
    APPENDED_LIST_FIRST(ClassSpecializationTypeData, IndexedType, parameters);
    END_APPENDED_LISTS(ClassSpecializationTypeData, parameters);
};

// This type represents a template class specialization.
class ClassSpecializationType : public KDevelop::StructureType
{
public:
    ClassSpecializationType(const ClassSpecializationType& rhs);

    explicit ClassSpecializationType(ClassSpecializationTypeData& data);

    using Ptr = KDevelop::TypePtr<ClassSpecializationType>;

    ClassSpecializationType();

    QString toString() const override;

    bool equals(const KDevelop::AbstractType * rhs) const override;

    QVector<KDevelop::IndexedType> templateParameters() const;

    void addParameter(const KDevelop::IndexedType& param);
    void clearParameters();

    KDevelop::AbstractType* clone() const override;

    size_t hash() const override;

    enum {
        Identity = 18
    };

    using Data = ClassSpecializationTypeData;

protected:
    TYPE_DECLARE_DATA(ClassSpecializationType);
};

#endif // CLASS_SPECIALIZATION_TYPE_H
