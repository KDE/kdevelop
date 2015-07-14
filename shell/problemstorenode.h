/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef PROBLEMNODE_H
#define PROBLEMNODE_H

#include <QString>
#include <QList>
#include <interfaces/iproblem.h>

namespace KDevelop
{

// Base class for ProblemStore node classes
// These nodes make up the problem tree for the problem store / problem model,
// that is then displayed in the problem tree view
class ProblemStoreNode
{
public:
    ProblemStoreNode(ProblemStoreNode *parent = nullptr)
    {
        m_parent = parent;
    }

    virtual ~ProblemStoreNode()
    {
        clear();
    }

    // Clear the children nodes
    void clear()
    {
        qDeleteAll(m_children);
        m_children.clear();
    }

    // Tells if the node is a root node.
    // A node is considered a root node (in this context), when it has no parent
    bool isRoot() const
    {
        if(!m_parent)
            return true;
        else
            return false;
    }

    // Returns the index of this node in the parent's child list.
    int index()
    {
        if(!m_parent)
            return -1;

        const QVector<ProblemStoreNode*> &children = m_parent->children();
        return children.indexOf(this);
    }

    // Returns the parent of this node
    ProblemStoreNode* parent() const{
        return m_parent;
    }

    // Sets the parent of this node
    void setParent(ProblemStoreNode *parent)
    {
        m_parent = parent;
    }

    // Returns the number of children nodes
    int count() const
    {
        return m_children.count();
    }

    // Returns a particular child node
    ProblemStoreNode* child(int row) const
    {
        return m_children[row];
    }

    // Returns the list of children nodes
    const QVector<ProblemStoreNode*>& children() const{
        return m_children;
    }

    // Adds a child node, and reparents the child
    void addChild(ProblemStoreNode *child)
    {
        m_children.push_back(child);
        child->setParent(this);
    }

    // Returns the label of this node, if there's one
    virtual QString label() const{
        return QString();
    }

    // Returns the node's stored problem, if there's such
    virtual IProblem::Ptr problem() const{
        return IProblem::Ptr(nullptr);
    }

private:
    // The parent node
    ProblemStoreNode *m_parent;

    // Children nodes
    QVector<ProblemStoreNode*> m_children;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////

// Label node, basically instead of storing a problem, it stores just a label, and used as parent node for problems
// E.g.: Path, Severity for grouping
class LabelNode : public ProblemStoreNode
{
public:
    LabelNode(ProblemStoreNode *parent = nullptr, const QString &l = QString())
        : ProblemStoreNode(parent)
        , m_label(l)
    {
    }

    ~LabelNode()
    {
    }

    QString label() const{ return m_label; }

    // Sets the label
    void setLabel(const QString &s){ m_label = s; }

private:
    // The label
    QString m_label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Problem node, stores problems
class ProblemNode : public ProblemStoreNode
{
public:
    ProblemNode(ProblemStoreNode *parent = nullptr, const IProblem::Ptr &problem = IProblem::Ptr(nullptr))
        : ProblemStoreNode(parent)
        , m_problem(problem)
    {
    }

    ~ProblemNode()
    {
    }

    IProblem::Ptr problem() const{
        return m_problem;
    }

    // Sets the problem
    void setProblem(const IProblem::Ptr &problem){
        m_problem = problem;
    }


private:
    // The problem
    IProblem::Ptr m_problem;
};

}

#endif

