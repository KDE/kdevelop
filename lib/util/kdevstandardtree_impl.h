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

#include "kdevstandardtree.h"

/* ======================================================================= */

template <typename ValueType>
KDevStandardTree<ValueType>::KDevStandardTree(ValueType rootValue)
{
    m_root = new KDevStandardNode<ValueType>(rootValue);
}

template <typename ValueType>
KDevStandardTree<ValueType>::~KDevStandardTree()
{
    clear();
}

template <typename ValueType>
void KDevStandardTree<ValueType>::clear()
{
    delete m_root;
}

template <typename ValueType>
KDevStandardNode<ValueType> *KDevStandardTree<ValueType>::root()
{
    return m_root;
}


/* ======================================================================= */

template <typename ValueType>
KDevStandardNode<ValueType>::KDevStandardNode(ValueType data)
    :m_parent(0)
{
    m_data = data;
}

template <typename ValueType>
KDevStandardNode<ValueType>::~KDevStandardNode()
{
    foreach (KDevStandardNode<ValueType> *node, m_nodes)
        delete node;
}

template <typename ValueType>
ValueType KDevStandardNode<ValueType>::data() const
{
    return m_data;
}

template <typename ValueType>
int KDevStandardNode<ValueType>::row()
{
    if (!m_parent)
        return 0;
    return m_parent->nodes().indexOf(this);
}

template <typename ValueType>
QList<KDevStandardNode<ValueType>*> KDevStandardNode<ValueType>::nodes() const
{
    return m_nodes;
}

template <typename ValueType>
void KDevStandardNode<ValueType>::addChild(KDevStandardNode<ValueType> *node)
{
    m_nodes << node;
    node->setParent(this);
}

template <typename ValueType>
KDevStandardNode<ValueType> *KDevStandardNode<ValueType>::parent() const
{
    return m_parent;
}

template <typename ValueType>
void KDevStandardNode<ValueType>::setParent(KDevStandardNode<ValueType> *node)
{
    m_parent = node;
}

/* ======================================================================= */
