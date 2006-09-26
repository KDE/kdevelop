/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KDEVSTANDARDTREE_H
#define KDEVSTANDARDTREE_H

#include <QList>

template <typename ValueType>
class KDevStandardNode;

template <typename ValueType>
class KDevStandardTree {
public:
    /**Creates the tree with the root node with value @p rootValue*/
    KDevStandardTree(ValueType rootValue);
    /**Clears up all memory occupied by the tree*/
    virtual ~KDevStandardTree();

    /**Clears the tree and all allocated memory*/
    virtual void clear();
    /** @return the root of the tree*/
    virtual KDevStandardNode<ValueType> *root();

protected:
    KDevStandardNode<ValueType> *m_root;

};

template <typename ValueType>
class KDevStandardNode {
public:
    /**Creates the tree node that holds some data*/
    KDevStandardNode(ValueType data);
    /**Destroys the node and all child nodes*/
    virtual ~KDevStandardNode();

    /** @return the data of the tree node*/
    virtual ValueType data() const;
    /** @return the list of child nodes*/
    virtual QList<KDevStandardNode<ValueType>*> nodes() const;

    /**Adds the @p node to the list of child nodes*/
    virtual void addChild(KDevStandardNode<ValueType> *node);

    /** @return the row index in the parent's list or 0 if the node has
    no parent*/
    virtual int row();

    virtual KDevStandardNode<ValueType> *parent() const;

private:
    /**Sets the parent of this node. This does not update the parent
    node so it's only for internal use.*/
    virtual void setParent(KDevStandardNode<ValueType> *node);

    ValueType m_data;
    QList<KDevStandardNode<ValueType>*> m_nodes;
    KDevStandardNode<ValueType> *m_parent;

};

template <typename Node>
class KDevStandardTreeOperator {
public:
    virtual ~KDevStandardTreeOperator() {}
    virtual void operator() (Node *node) { }
};

template <typename ValueType>
class KDevStandardTreeVisitor {
public:
    KDevStandardTreeVisitor() {}

    template <typename Operator>
    void visitTree(Operator op, KDevStandardTree<ValueType> *tree)
    {
        visitNode(op, tree->root());
    }

    template <typename Operator>
    void visitNode(Operator op, KDevStandardNode<ValueType> *node)
    {
        op(node);
        foreach (KDevStandardTree<ValueType> *child, node->nodes)
            visitNode(child);
    }

};

#include "kdevstandardtree_impl.h"

#endif
