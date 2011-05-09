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

#ifndef CONTROLFLOWGRAPH_H
#define CONTROLFLOWGRAPH_H

#include <QVector>
#include <language/languageexport.h>

namespace KDevelop {

class ControlFlowNode;

class KDEVPLATFORMLANGUAGE_EXPORT ControlFlowGraph
{
  public:
    typedef QVector<ControlFlowNode*>::const_iterator const_iterator;
    ~ControlFlowGraph();
    
    void addEntry(ControlFlowNode* n);
    void addDeadNode(ControlFlowNode* n);
    void clear();
    
    const_iterator constBegin() const { return m_graphNodes.constBegin(); }
    const_iterator constEnd() const { return m_graphNodes.constEnd(); }
    
    QVector<ControlFlowNode*> deadNodes() const { return m_deadNodes; }
  private:
    QVector<ControlFlowNode*> m_graphNodes;
    QVector<ControlFlowNode*> m_deadNodes;
};

}

#endif
