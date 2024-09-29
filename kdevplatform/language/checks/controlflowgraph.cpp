/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "controlflowgraph.h"
#include "controlflownode.h"

#include <util/algorithm.h>

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
    if (!node || !Algorithm::insert(deleted, node).inserted) {
        return;
    }

    clearNodeRecursively(node->next(), deleted);
    clearNodeRecursively(node->alternative(), deleted);

    delete node;
}

void ControlFlowGraph::clear()
{
    Q_D(ControlFlowGraph);

    QSet<ControlFlowNode*> deleted;
    for (ControlFlowNode* node : std::as_const(d->m_funcNodes)) {
        clearNodeRecursively(node, deleted);
    }

    for (ControlFlowNode* node : std::as_const(d->m_nodes)) {
        clearNodeRecursively(node, deleted);
    }

    for (ControlFlowNode* node : std::as_const(d->m_deadNodes)) {
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
