/*
    SPDX-FileCopyrightText: 2008 Ramón Zarazúa <killerfox512+kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainchangeset.h"
#include <debug.h>
#include <serialization/indexedstring.h>

namespace KDevelop {
//DUChainRef::DUChainRef(DUChainChangeSet* set, DUChainBase* object, bool newObject) :
//    m_changeSet(set), m_object(object), m_objectRef(0), m_newObject(newObject)
//{
//}

DUChainChangeSet::DUChainChangeSet(const ReferencedTopDUContext& topContext) :
    m_topContext(topContext)
{
}

DUChainChangeSet::~DUChainChangeSet()
{
    qDeleteAll(m_objectRefs);
}

DUChainChangeSet& DUChainChangeSet::operator<<(DUChainChangeSet& rhs)
{
    //Avoid merging into self
    if (this == &rhs)
        return *this;

    Q_ASSERT(m_topContext == rhs.m_topContext);
    qCDebug(LANGUAGE) << "Merging ChangeSets for context:" << m_topContext.data()->url().str();

    m_objectRefs << rhs.m_objectRefs;
    rhs.m_objectRefs.clear();

#ifndef NDEBUG
    //check for possible duplicates
    std::sort(m_objectRefs.begin(), m_objectRefs.end());

    for (QList<DUChainRef*>::iterator i = m_objectRefs.begin(); i < m_objectRefs.end() - 1; ++i)
        Q_ASSERT(*i != *(i + 1));

#endif
    return *this;
}

QList<DUChainRef*> DUChainChangeSet::objectRefs() const
{
    return m_objectRefs;
}

const ReferencedTopDUContext& DUChainChangeSet::topDuContext() const
{
    return m_topContext;
}
}
