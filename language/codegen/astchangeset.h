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

#ifndef ASTCHANGESET_H
#define ASTCHANGESET_H

#include <QVariant>

#include "../languageexport.h"

namespace KDevelop {

class AstChangeSet;
class AstChange;
class AstNode;

/**
 * \short A reference to an existing read-only AST node.
 *
 * This class represents an AST node, and allows changes to be planned
 * for that node.
 *
 * \warning you must not create cyclic references.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT AstNodeRef
{
    friend class AstChangeSet;

public:
    virtual ~AstNodeRef();

    virtual const AstNode* node() const;
    virtual AstNodeRef* nodeRef() const;

    virtual AstNode* newNode() const;

    const QList<AstChange*>& changes() const;

    /// Adds a change to this node reference. Takes ownership of the \a change.
    AstChange* addChange(AstChange* change);
    /// Removes a change from this node reference, and deletes it.
    void deleteChange(AstChange* change);

protected:
    /// Default constructor. \todo is this needed?
    AstNodeRef(AstChangeSet* set);
    /// Constructor.  Either takes an existing \a node (\a newNode = false), or a newly created \a node (\a newNode = true)
    AstNodeRef(AstChangeSet* set, AstNode* node, bool newNode);
    /// Constructor.  Takes another node reference.
    AstNodeRef(AstChangeSet* set, AstNodeRef* original);

private:
    AstChangeSet* m_changeSet;
    AstNode* m_node;
    AstNodeRef* m_nodeRef;
    bool m_newNode;

    QList<AstChange*> m_changes;
};

typedef QList<AstNodeRef*> AstNodeList;

/**
 * \short Container class for a change to an AST node.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT AstChange
{
public:
    enum ChangeTypes {
        ListInsert,
        ListRemove,
        ListClear,
        ItemReplace,
        ItemMove
    };

    AstChange(ChangeTypes t);

    ChangeTypes type;
    // The index of the item in the node to be changed
    int nodeIndex;

    // The new node to occupy this position, if relevant
    AstNodeRef* newNode;
    // The list of nodes to occupy this position, if relevant
    AstNodeList newList;
    // The position to apply the node(s) in the list, if relevant
    int listOffset;
    // The value of the position, if relevant
    QVariant newValue;
};

/**
 * \short A set of changes to an AST.
 *
 * This class holds a set of all changes to an AST.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT AstChangeSet
{
public:
    /**
     * Constructor.
     *
     * \param topNode the top node of the read-only Ast to modify, or set to null if creating
     *                a new Ast from scratch.
     */
    AstChangeSet(const AstNode* topNode = 0);

    /**
     * Destructor, deletes all nodes owned by this change set.
     */
    virtual ~AstChangeSet();

    /**
     * Register a new node that you have created to insert at some point in this Ast.
     * You may modify this node directly.  The change set takes ownership, so that
     * the new node will be deleted when the change set is no longer needed.
     *
     * \returns the new node that has been registered.
     */
    AstNodeRef* registerNewNode(AstNode* node);

    /**
     * Create a blank reference to a node.
     *
     * The change set takes ownership, so that
     * the new node will be deleted when the change set is no longer needed.
     *
     * \returns the new node reference
     */
    AstNodeRef* registerNewRef(AstNodeRef* ref);

    /**
    * Copy an existing node (whether from the Ast or from the change set).
    *
    * You may then modify this reference, and the modifications will be applied to the node when the change set is finalised.
    *
    * \returns a copy of \a source, which you may modify directly.
    */
    AstNodeRef* copyNode(AstNode* source);

private:
    const AstNode* m_topNode;
    QList<AstNodeRef*> m_nodeRefs;
};

}

#endif // ASTCHANGESET_H
