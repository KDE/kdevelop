/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#include "astchangeset.h"

using namespace KDevelop;

AstNodeRef::AstNodeRef(AstChangeSet* set)
    : m_changeSet(set)
    , m_original(0)
    , m_originalRef(0)
{
}

AstNodeRef::AstNodeRef(AstChangeSet* set, AstNode* original)
    : m_changeSet(set)
    , m_original(original)
    , m_originalRef(0)
{
}

AstNodeRef::AstNodeRef(AstChangeSet* set, AstNodeRef* original)
    : m_changeSet(set)
    , m_original(0)
    , m_originalRef(original)
{
}

AstNodeRef::~AstNodeRef()
{
    qDeleteAll(m_changes);
}

AstNode* AstNodeRef::originalNode() const
{
    return m_original;
}

AstNodeRef* AstNodeRef::originalNodeRef() const
{
    return m_originalRef;
}

const QList<AstChange*>& AstNodeRef::changes() const
{
    return m_changes;
}

AstChange* AstNodeRef::addChange(AstChange* change)
{
    m_changes.append(change);
    return change;
}

void AstNodeRef::deleteChange(AstChange* change)
{
    Q_ASSERT(m_changes.contains(change));
    m_changes.removeAll(change);
    delete change;
}

AstChange::AstChange(ChangeTypes t)
    : type(t)
    , newNode(0)
    , listOffset(-1)
    , newValue(-1)
{
}

AstChangeSet::AstChangeSet(const AstNode* topNode)
    : m_topNode(topNode)
{
}

AstChangeSet::~AstChangeSet()
{
    qDeleteAll(m_localNodes);
    qDeleteAll(m_nodeRefs);
}

AstNode* AstChangeSet::registerNewNode(AstNode* node)
{
    m_localNodes.append(node);
    return node;
}

AstNodeRef* AstChangeSet::registerNewRef(AstNodeRef* ref)
{
    m_nodeRefs.append(ref);
    return ref;
}

AstNodeRef* AstChangeSet::copyNode(AstNode* source)
{
    AstNodeRef* newRef = new AstNodeRef(this, source);
    m_nodeRefs.append(newRef);
    return newRef;
}
