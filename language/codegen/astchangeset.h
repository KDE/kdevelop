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

#ifndef KDEVPLATFORM_ASTCHANGESET_H
#define KDEVPLATFORM_ASTCHANGESET_H

#include <QtCore/QVariant>

namespace KDevelop {

template <typename AstNode>
class AstChangeSet;


class AstChange;

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
template <typename AstNode>
class AstNodeRef
{
    friend class AstChangeSet<AstNode>;

public:
    /// Destructor.
    virtual ~AstNodeRef()
    {
        qDeleteAll(m_changes);

        if (m_newNode)
            delete m_node;
    }

    typedef QList<AstNodeRef*> AstNodeList;

    /**
     * \short Container class for a change to an AST node.
     *
     * \author Hamish Rodda <rodda@kde.org>
     */
    class AstChange
    {
    public:
        enum ChangeTypes {
            ListRewrite,
            ListClear,
            ItemReplace,
            ItemMove
        };

        AstChange(ChangeTypes t)
            : type(t)
            , newNode(0)
            , listOffset(-1)
            , newValue(-1)
        {
        }

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

    virtual const AstNode* node() const
    {
        return m_newNode ? 0 : m_node;
    }

    virtual AstNodeRef* nodeRef() const
    {
        return m_nodeRef;
    }

    virtual AstNode* newNode() const
    {
        return m_newNode ? m_node : 0;
    }

    const QList<AstChange*>& changes() const
    {
        return m_changes;
    }

    /// Adds a change to this node reference. Takes ownership of the \a change.
    AstChange* newChange(AstChange* change)
    {
        m_changes.append(change);
        return change;
    }

    /// Removes a change from this node reference, and deletes it.
    void deleteChange(AstChange* change)
    {
        Q_ASSERT(m_changes.contains(change));
        m_changes.removeAll(change);
        delete change;
    }

protected:
    /// Default constructor. \todo is this needed?
    AstNodeRef(AstChangeSet<AstNode>* set)
        : m_changeSet(set)
        , m_node(0)
        , m_nodeRef(0)
        , m_newNode(false)
    {
    }

    /// Constructor.  Either takes an existing \a node (\a newNode = false), or a newly created \a node (\a newNode = true)
    AstNodeRef(AstChangeSet<AstNode>* set, AstNode* node, bool newNode)
        : m_changeSet(set)
        , m_node(node)
        , m_nodeRef(0)
        , m_newNode(newNode)
    {
    }

    /// Constructor.  Takes another node reference.
    AstNodeRef(AstChangeSet<AstNode>* set, AstNodeRef* original)
        : m_changeSet(set)
        , m_node(0)
        , m_nodeRef(original)
        , m_newNode(false)
    {
    }

    AstNode* m_nodeChanges;

private:
    AstChangeSet<AstNode>* m_changeSet;
    AstNode* m_node;
    AstNodeRef* m_nodeRef;
    bool m_newNode;

    QList<AstChange*> m_changes;
};

/**
 * \short A set of changes to an AST.
 *
 * This class holds a set of all changes to an AST.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
template <typename AstNode>
class AstChangeSet
{
public:
    /**
     * Constructor.
     *
     * \param topNode the top node of the read-only Ast to modify, or set to null if creating
     *                a new Ast from scratch.
     */
    AstChangeSet(const AstNode* topNode = 0)
        : m_topNode(topNode)
    {
    }

    /**
     * Destructor, deletes all nodes owned by this change set.
     */
    virtual ~AstChangeSet()
    {
        qDeleteAll(m_nodeRefs);
    }

    /**
     * Register a new node that you have created to insert at some point in this Ast.
     * You may modify this node directly.  The change set takes ownership, so that
     * the new node will be deleted when the change set is no longer needed.
     *
     * \returns the new node that has been registered.
     */
    AstNodeRef<AstNode>* registerNewNode(AstNode* node)
    {
        AstNodeRef<AstNode>* newRef = new AstNodeRef<AstNode>(this, node, true);
        m_nodeRefs.append(newRef);
        return newRef;
    }

    /**
     * Create a blank reference to a node.
     *
     * The change set takes ownership, so that
     * the new node will be deleted when the change set is no longer needed.
     *
     * \returns the new node reference
     */
    AstNodeRef<AstNode>* registerNewRef(AstNodeRef<AstNode>* ref)
    {
        m_nodeRefs.append(ref);
        return ref;
    }

    /**
    * Copy an existing node (whether from the Ast or from the change set).
    *
    * You may then modify this reference, and the modifications will be applied to the node when the change set is finalised.
    *
    * \returns a copy of \a source, which you may modify directly.
    */
    AstNodeRef<AstNode>* copyNode(AstNode* source)
    {
        AstNodeRef<AstNode>* newRef = new AstNodeRef<AstNode>(this, source, false);
        m_nodeRefs.append(newRef);
        return newRef;
    }

private:
    const AstNode* m_topNode;
    QList<AstNodeRef<AstNode>*> m_nodeRefs;
};

}

#endif // KDEVPLATFORM_ASTCHANGESET_H
