/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

    uint hash() const override;

    enum {
        Identity = 18
    };

    using Data = ClassSpecializationTypeData;

protected:
    TYPE_DECLARE_DATA(ClassSpecializationType);
};

#endif // CLASS_SPECIALIZATION_TYPE_H
