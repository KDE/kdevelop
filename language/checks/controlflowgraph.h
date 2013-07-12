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

#ifndef KDEVPLATFORM_CONTROLFLOWGRAPH_H
#define KDEVPLATFORM_CONTROLFLOWGRAPH_H

#include <QVector>
#include <QMap>
#include <language/languageexport.h>

namespace KDevelop {

class Declaration;
class ControlFlowNode;

/**
 * @brief The ControlFlowGraph describes the way a code interacts with the current state of a system
 *
 * This class will store the information regarding how is the code flow going to change depending
 * on what current state we have in our system. It will tell us what different code paths we have
 * available by listing them in different ways and it will let us know what those paths depend on
 * so that we can analyze it.
 */

class KDEVPLATFORMLANGUAGE_EXPORT ControlFlowGraph
{
  public:
    /** Creates an empty graph. */
    ControlFlowGraph();
    ~ControlFlowGraph();

    /** Adds an entry @p n to the graph. The graph takes the ownership of @p n */
    void addEntry(KDevelop::ControlFlowNode* n);

    /** Adds an entry @p n to the graph given @p decl declaration. The graph takes the ownership of @p n */
    void addEntry(KDevelop::Declaration* d, KDevelop::ControlFlowNode* n);

    /** Adds a node that does belong to the graph but that can't be accessed by any means. The graph takes the ownership of @p n */
    void addDeadNode(ControlFlowNode* n);

    /** Clears the current graph as if it was just constructed */
    void clear();

    /** @returns all declarations that have a node attached to */
    QList<KDevelop::Declaration*> declarations() const;

    /** @returns  the node attached to the declaration @p d*/
    ControlFlowNode* nodeForDeclaration(KDevelop::Declaration* d) const;

    /** @returns all root nodes in the graph */
    QList<ControlFlowNode*> rootNodes() const;

    /** @returns all dead nodes in the graph */
    QVector<ControlFlowNode*> deadNodes() const;
  private:
    ControlFlowGraph(const ControlFlowGraph&);
    class Private;
    Private* d;
};

}

#endif
