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

#include "../ast/astnode.h"

using namespace KDevelop;

AstNodeRef::AstNodeRef(AstChangeSet* set)
    : m_changeSet(set)
    , m_node(0)
    , m_nodeRef(0)
    , m_newNode(false)
{
}

AstNodeRef::AstNodeRef(AstChangeSet* set, AstNode* node, bool newNode)
    : m_changeSet(set)
    , m_node(node)
    , m_nodeRef(0)
    , m_newNode(newNode)
{
}

AstNodeRef::AstNodeRef(AstChangeSet* set, AstNodeRef* original)
    : m_changeSet(set)
    , m_node(0)
    , m_nodeRef(original)
    , m_newNode(false)
{
}

AstNodeRef::~AstNodeRef()
{
    qDeleteAll(m_changes);

    if (m_newNode)
        delete m_node;
}

const AstNode* AstNodeRef::node() const
{
    return m_newNode ? 0 : m_node;
}

AstNodeRef* AstNodeRef::nodeRef() const
{
    return m_nodeRef;
}

AstNode* AstNodeRef::newNode() const
{
    return m_newNode ? m_node : 0;
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
    qDeleteAll(m_nodeRefs);
}

AstNodeRef* AstChangeSet::registerNewNode(AstNode* node)
{
    AstNodeRef* newRef = new AstNodeRef(this, node, true);
    m_nodeRefs.append(newRef);
    return newRef;
}

AstNodeRef* AstChangeSet::registerNewRef(AstNodeRef* ref)
{
    m_nodeRefs.append(ref);
    return ref;
}

AstNodeRef* AstChangeSet::copyNode(AstNode* source)
{
    AstNodeRef* newRef = new AstNodeRef(this, source, false);
    m_nodeRefs.append(newRef);
    return newRef;
}
