/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "controlflowgraph.h"
#include "controlflownode.h"

using namespace KDevelop;

class KDevelop::ControlFlowGraphPrivate
{
public:
    QList<ControlFlowNode*> m_nodes;
    QMap<KDevelop::Declaration*, ControlFlowNode*> m_funcNodes;
    QVector<ControlFlowNode*> m_deadNodes;
};

ControlFlowGraph::ControlFlowGraph()
    : d_ptr(new ControlFlowGraphPrivate)
{}

ControlFlowGraph::~ControlFlowGraph()
{
    clear();
}

void ControlFlowGraph::addEntry(ControlFlowNode* n)
{
    Q_D(ControlFlowGraph);

    d->m_nodes += n;
}

void ControlFlowGraph::addEntry(Declaration* decl, ControlFlowNode* n)
{
    Q_D(ControlFlowGraph);

    Q_ASSERT(d);
    Q_ASSERT(decl);
    d->m_funcNodes.insert(decl, n);
}

void ControlFlowGraph::addDeadNode(ControlFlowNode* n)
{
    Q_D(ControlFlowGraph);

    d->m_deadNodes += n;
}

void clearNodeRecursively(ControlFlowNode* node, QSet<ControlFlowNode*>& deleted)
{
    if (!node || deleted.contains(node))
        return;

    deleted += node;

    clearNodeRecursively(node->next(), deleted);
    clearNodeRecursively(node->alternative(), deleted);

    delete node;
}

void ControlFlowGraph::clear()
{
    Q_D(ControlFlowGraph);

    QSet<ControlFlowNode*> deleted;
    for (ControlFlowNode* node : qAsConst(d->m_funcNodes)) {
        clearNodeRecursively(node, deleted);
    }

    for (ControlFlowNode* node : qAsConst(d->m_nodes)) {
        clearNodeRecursively(node, deleted);
    }

    for (ControlFlowNode* node : qAsConst(d->m_deadNodes)) {
        clearNodeRecursively(node, deleted);
    }

    d->m_nodes.clear();
    d->m_funcNodes.clear();
    d->m_deadNodes.clear();
}

QList<ControlFlowNode*> ControlFlowGraph::rootNodes() const
{
    Q_D(const ControlFlowGraph);

    return d->m_funcNodes.values() + d->m_nodes;
}

QVector<ControlFlowNode*> ControlFlowGraph::deadNodes() const
{
    Q_D(const ControlFlowGraph);

    return d->m_deadNodes;
}

QList<Declaration*> ControlFlowGraph::declarations() const
{
    Q_D(const ControlFlowGraph);

    return d->m_funcNodes.keys();
}

ControlFlowNode* ControlFlowGraph::nodeForDeclaration(Declaration* decl) const
{
    Q_D(const ControlFlowGraph);

    return d->m_funcNodes.value(decl);
}
