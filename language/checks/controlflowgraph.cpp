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

struct ControlFlowGraph::Private
{
    QList<ControlFlowNode*> m_nodes;
    QMap<KDevelop::DUContext*, ControlFlowNode*> m_funcNodes;
    QVector<ControlFlowNode*> m_deadNodes;
};

ControlFlowGraph::ControlFlowGraph()
    : d(new Private)
{}


ControlFlowGraph::~ControlFlowGraph()
{
  clear();
}

void ControlFlowGraph::addEntry(ControlFlowNode* n)
{
    d->m_nodes += n;
}

void ControlFlowGraph::addEntry(KDevelop::DUContext* ctx, ControlFlowNode* n)
{
  Q_ASSERT(ctx);
  d->m_funcNodes.insert(ctx, n);
}

void ControlFlowGraph::addDeadNode(ControlFlowNode* n)
{
  d->m_deadNodes += n;
}

void clearNodeRecursively(ControlFlowNode* node, QSet<ControlFlowNode*>& deleted)
{
  if(!node || deleted.contains(node))
    return;
  
  deleted += node;
  
  clearNodeRecursively(node->m_next, deleted);
  clearNodeRecursively(node->m_alternative, deleted);
  
  delete node;
}

void ControlFlowGraph::clear()
{
  QSet<ControlFlowNode*> deleted;
  foreach(ControlFlowNode* node, d->m_funcNodes)
    clearNodeRecursively(node, deleted);
  
  foreach(ControlFlowNode* node, d->m_nodes)
    clearNodeRecursively(node, deleted);
  
  foreach(ControlFlowNode* node, d->m_deadNodes)
    clearNodeRecursively(node, deleted);
  
  d->m_nodes.clear();
  d->m_funcNodes.clear();
  d->m_deadNodes.clear();
}

QList< ControlFlowNode* > ControlFlowGraph::graphNodes() const
{
    return d->m_funcNodes.values()+d->m_nodes;
}

QVector< ControlFlowNode* > ControlFlowGraph::deadNodes() const
{
    return d->m_deadNodes;
}

QList<DUContext*> ControlFlowGraph::contexts() const
{
    return d->m_funcNodes.keys();
}

ControlFlowNode* ControlFlowGraph::nodePerContext(DUContext* ctx)
{
    return d->m_funcNodes[ctx];
}
