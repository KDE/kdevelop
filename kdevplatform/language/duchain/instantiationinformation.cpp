/*
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "instantiationinformation.h"
#include "identifier.h"
#include "serialization/itemrepository.h"
#include "serialization/itemrepositoryreferencecounting.h"
#include <serialization/repositorymanager.h>
#include "types/typeutils.h"
#include <typeinfo>
#include "types/typealiastype.h"
#include "types/typerepository.h"

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(InstantiationInformation, templateParameters, IndexedType)

QualifiedIdentifier InstantiationInformation::applyToIdentifier(const QualifiedIdentifier& id) const
{
    QualifiedIdentifier ret;
    if (id.count() > 1) {
        ret = id;
        ret.pop();
        if (previousInstantiationInformation.index())
            ret = previousInstantiationInformation.information().applyToIdentifier(ret);
    }

    Identifier lastId(id.last());

    KDevVarLengthArray<IndexedTypeIdentifier> oldTemplateIdentifiers;
    for (uint a = 0; a < lastId.templateIdentifiersCount(); ++a)
        oldTemplateIdentifiers.append(lastId.templateIdentifier(a));

    lastId.clearTemplateIdentifiers();

    for (uint a = 0; a < templateParametersSize(); ++a) {
        if (templateParameters()[a].abstractType()) {
            lastId.appendTemplateIdentifier(IndexedTypeIdentifier(templateParameters()[a].abstractType()->toString(),
                                                                  true));
        } else {
            lastId.appendTemplateIdentifier(
                ( uint ) oldTemplateIdentifiers.size() > a ? oldTemplateIdentifiers[a] : IndexedTypeIdentifier());
        }
    }

    for (int a = templateParametersSize(); a < oldTemplateIdentifiers.size(); ++a)
        lastId.appendTemplateIdentifier(oldTemplateIdentifiers[a]);

    ret.push(lastId);
    return ret;
}

void InstantiationInformation::addTemplateParameter(const KDevelop::AbstractType::Ptr& type)
{
    templateParametersList().append(IndexedType(type));
}

QString InstantiationInformation::toString(bool local) const
{
    QString ret;
    if (previousInstantiationInformation.index() && !local)
        ret = previousInstantiationInformation.information().toString() + QLatin1String("::");
    ret += QLatin1Char('<');
    QStringList types;
    types.reserve(templateParametersSize());
    for (uint a = 0; a < templateParametersSize(); ++a) {
        if (templateParameters()[a].abstractType())
            types.append(templateParameters()[a].abstractType()->toString());
        else
            // TODO: what should be here instead?
            types.append(QString());
    }

    ret += QLatin1Char('<') + types.join(QLatin1String(", ")) + QLatin1Char('>');
    return ret;
}

InstantiationInformation::InstantiationInformation() : m_refCount(0)
{
    initializeAppendedLists();
}

InstantiationInformation::InstantiationInformation(const InstantiationInformation& rhs,
                                                   bool dynamic) : previousInstantiationInformation(
        rhs.previousInstantiationInformation)
    , m_refCount(0)
{
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
}

InstantiationInformation::~InstantiationInformation()
{
    freeAppendedLists();
}

InstantiationInformation& InstantiationInformation::operator=(const InstantiationInformation& rhs)
{
    previousInstantiationInformation = rhs.previousInstantiationInformation;
    copyListsFrom(rhs);
    return *this;
}

bool InstantiationInformation::operator==(const InstantiationInformation& rhs) const
{
    if (!(previousInstantiationInformation == rhs.previousInstantiationInformation))
        return false;
    return listsEqual(rhs);
}

size_t InstantiationInformation::hash() const
{
    KDevHash kdevhash;
    FOREACH_FUNCTION(const IndexedType &param, templateParameters) {
        kdevhash << param.hash();
    }
    return kdevhash << previousInstantiationInformation.index();
}

using InstantiationInformationRepository
    = ItemRepository<InstantiationInformation, AppendedListItemRequest<InstantiationInformation>, true,
                     QRecursiveMutex>;
using InstantiationInformationRepositoryManager = RepositoryManager<InstantiationInformationRepository>;

template <>
class ItemRepositoryFor<IndexedInstantiationInformation>
{
    friend struct LockedItemRepository;
    static InstantiationInformationRepository& repo()
    {
        static InstantiationInformationRepositoryManager manager(QStringLiteral("Instantiation Information Repository"),
                                                                 typeRepositoryMutex());
        return *manager.repository();
    }
};

uint standardInstantiationInformationIndex()
{
    static uint idx = LockedItemRepository::write<IndexedInstantiationInformation>(
        [standardInstantiationInformation = InstantiationInformation()](InstantiationInformationRepository& repo) {
            return repo.index(standardInstantiationInformation);
        });
    return idx;
}

void initInstantiationInformationRepository()
{
    standardInstantiationInformationIndex();
}

IndexedInstantiationInformation::IndexedInstantiationInformation(uint index) : m_index(index)
{
    if (m_index == standardInstantiationInformationIndex())
        m_index = 0;

    if (m_index) {
        ItemRepositoryReferenceCounting::inc(this);
    }
}

// NOTE: the definitions of ItemRepositoryReferenceCounting's inc(), dec() and setIndex() are so
// complex that they can throw exceptions for many reasons. Yet some special member functions of
// IndexedInstantiationInformation, which call them, are implicitly (the destructor) or explicitly
// noexcept. The noexcept-ness of these functions is important for correctness and performance.
// This is safe at the moment, because the entire KDevPlatformLanguage library, that contains
// IndexedInstantiationInformation, is compiled with exceptions disabled (-fno-exceptions), which
// already prevents exception propagation to a caller of any non-inline function in this library.

IndexedInstantiationInformation::IndexedInstantiationInformation(const IndexedInstantiationInformation& rhs) noexcept
    : m_index(rhs.m_index)
{
    if (m_index) {
        ItemRepositoryReferenceCounting::inc(this);
    }
}

IndexedInstantiationInformation& IndexedInstantiationInformation::operator=(const IndexedInstantiationInformation& rhs) noexcept
{
    const auto checkIndex = [](unsigned int index) { return index != 0; };
    ItemRepositoryReferenceCounting::setIndex(this, m_index, rhs.m_index, checkIndex);
    return *this;
}

IndexedInstantiationInformation::~IndexedInstantiationInformation()
{
    if (m_index) {
        ItemRepositoryReferenceCounting::dec(this);
    }
}

bool IndexedInstantiationInformation::isValid() const
{
    return m_index;
}

const InstantiationInformation& IndexedInstantiationInformation::information() const
{
    auto index = m_index ? m_index : standardInstantiationInformationIndex();
    // TODO: it's probably unsafe to return the const& here, as the repo won't be locked during access anymore
    return *LockedItemRepository::read<IndexedInstantiationInformation>(
        [index](const InstantiationInformationRepository& repo) { return repo.itemFromIndex(index); });
}

IndexedInstantiationInformation InstantiationInformation::indexed() const
{
    auto index = LockedItemRepository::write<IndexedInstantiationInformation>(
        [this](InstantiationInformationRepository& repo) { return repo.index(*this); });
    return IndexedInstantiationInformation(index);
}
}
