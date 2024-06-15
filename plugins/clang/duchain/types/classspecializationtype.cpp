/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "classspecializationtype.h"

using namespace KDevelop;

#include <language/duchain/appendedlist.h>
#include <serialization/indexedstring.h>

// The type is registered in DUChainUtils::registerDUChainItems
// REGISTER_TYPE(ClassSpecializationType);

DEFINE_LIST_MEMBER_HASH(ClassSpecializationTypeData, parameters, IndexedType)

ClassSpecializationTypeData::ClassSpecializationTypeData()
{
    initializeAppendedLists(m_dynamic);
}

ClassSpecializationTypeData::ClassSpecializationTypeData(const ClassSpecializationTypeData& rhs)
    : KDevelop::StructureTypeData(rhs)
{
    initializeAppendedLists(m_dynamic);
    copyListsFrom(rhs);
}

ClassSpecializationTypeData::~ClassSpecializationTypeData()
{
    freeAppendedLists();
}

ClassSpecializationType::ClassSpecializationType(const ClassSpecializationType& rhs)
    : KDevelop::StructureType(copyData<ClassSpecializationType>(*rhs.d_func()))
{}

ClassSpecializationType::ClassSpecializationType(ClassSpecializationTypeData& data)
    : KDevelop::StructureType(data)
{}

AbstractType* ClassSpecializationType::clone() const
{
    return new ClassSpecializationType(*this);
}

ClassSpecializationType::ClassSpecializationType()
    : KDevelop::StructureType(createData<ClassSpecializationType>())
{}

size_t ClassSpecializationType::hash() const
{
    KDevHash kdevhash(StructureType::hash());

    FOREACH_FUNCTION(const auto& param, d_func()->parameters) {
        kdevhash << param.hash();
    }
    return kdevhash;
}

namespace {
// we need to skip the template parameters of the last identifier,
// so do the stringification manually here
QString strippedQid(const QualifiedIdentifier& qid)
{
    QString result;
    if (qid.explicitlyGlobal()) {
        result += QLatin1String("::");
    }
    const auto parts = qid.count();
    for (int i = 0; i < parts - 1; ++i) {
        result += qid.at(i).toString() + QLatin1String("::");
    }
    const auto last = qid.at(parts - 1);
    result += last.identifier().str();
    return result;
}
}

QString ClassSpecializationType::toString() const
{
    QualifiedIdentifier id = qualifiedIdentifier();
    if (!id.isEmpty()) {
        QString result = AbstractType::toString() + strippedQid(id) + QLatin1String("< ");
        bool first = true;
        const auto& templateParameters = this->templateParameters();
        for (const auto& param : templateParameters) {
            if (first) {
                first = false;
            } else {
                result += QLatin1String(", ");
            }
            result += param.abstractType()->toString();
        }
        result += QLatin1String(" >");
        return result;
    }

    return StructureType::toString();
}

bool ClassSpecializationType::equals(const KDevelop::AbstractType* rhs) const
{
    if (this == rhs) {
        return true;
    }

    auto tt = dynamic_cast<const ClassSpecializationType*>(rhs);
    if (!tt || templateParameters() != tt->templateParameters()) {
        return false;
    }

    return StructureType::equals(rhs);
}

QVector<KDevelop::IndexedType> ClassSpecializationType::templateParameters() const
{
    const auto size = d_func()->parametersSize();
    QVector<IndexedType> parameters(size);
    std::copy_n(d_func()->parameters(), size, parameters.begin());

    return parameters;
}

void ClassSpecializationType::addParameter(const KDevelop::IndexedType& param)
{
    d_func_dynamic()->parametersList().append(param);
}

void ClassSpecializationType::clearParameters()
{
    d_func_dynamic()->parametersList().clear();
}
