/*
   Copyright 2008 Ramón Zarazúa <killerfox512+kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "duchainchangeset.h"

namespace KDevelop
{

//DUChainRef::DUChainRef(DUChainChangeSet* set, DUChainBase* object, bool newObject) :
//    m_changeSet(set), m_object(object), m_objectRef(0), m_newObject(newObject)
//{
//}

DUChainChangeSet::DUChainChangeSet(ReferencedTopDUContext topContext) :
    m_topContext(topContext)
{
}

DUChainChangeSet::~DUChainChangeSet()
{
    foreach(DUChainRef * reference, m_objectRefs)
        delete reference;
}

DUChainChangeSet & DUChainChangeSet::operator<<(DUChainChangeSet & rhs)
{
    //Avoid merging into self
    if(this == &rhs)
        return *this;
    
    Q_ASSERT(m_topContext == rhs.m_topContext);
    kDebug() << "Merging ChangeSets for context:" << m_topContext.data()->url().str();
    
    m_objectRefs << rhs.m_objectRefs;
    rhs.m_objectRefs.clear();
    
#ifndef NDEBUG
    //check for possible duplicates
    std::sort(m_objectRefs.begin(), m_objectRefs.end());
    
    for(QList<DUChainRef *>::iterator i = m_objectRefs.begin(); i < m_objectRefs.end() - 1; ++i  )
        Q_ASSERT(*i != *(i + 1));
#endif
    return *this;
}

QList<DUChainRef *> DUChainChangeSet::objectRefs() const
{
    return m_objectRefs;
}

const ReferencedTopDUContext & DUChainChangeSet::topDuContext() const
{
    return m_topContext;
}

}
