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


#ifndef KDEVPLATFORM_PROBLEMSTORENODE_H
#define KDEVPLATFORM_PROBLEMSTORENODE_H

#include <QString>
#include <interfaces/iproblem.h>
#include <shell/shellexport.h>

namespace KDevelop
{

/**
 * @brief Base class for ProblemStoreNode classes, which together make up a tree structure with label or problem leaves.
 *
 * When adding a child the node is automatically reparented.
 *
 * Usage:
 * @code
 * ProblemStoreNode *root = new ProblemStoreNode();
 * root->addChild(new ProblemStoreNode());
 * root->addChild(new ProblemStoreNode());
 * root->addChild(new ProblemStoreNode());
 * root->count(); // Returns 3
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemStoreNode
{
public:
    explicit ProblemStoreNode(ProblemStoreNode *parent = nullptr)
    {
        m_parent = parent;
    }

    virtual ~ProblemStoreNode()
    {
        clear();
    }

    /// Clear the children nodes
    void clear()
    {
        qDeleteAll(m_children);
        m_children.clear();
    }

    /// Tells if the node is a root node.
    /// A node is considered a root node (in this context), when it has no parent
    bool isRoot() const
    {
        if(!m_parent)
            return true;
        else
            return false;
    }

    /// Returns the index of this node in the parent's child list.
    int index()
    {
        if(!m_parent)
            return -1;

        const QVector<ProblemStoreNode*> &children = m_parent->children();
        return children.indexOf(this);
    }

    /// Returns the parent of this node
    ProblemStoreNode* parent() const{
        return m_parent;
    }

    /// Sets the parent of this node
    void setParent(ProblemStoreNode *parent)
    {
        m_parent = parent;
    }

    /// Returns the number of children nodes
    int count() const
    {
        return m_children.count();
    }

    /// Returns a particular child node
    ProblemStoreNode* child(int row) const
    {
        return m_children[row];
    }

    /// Returns the list of children nodes
    const QVector<ProblemStoreNode*>& children() const{
        return m_children;
    }

    /// Adds a child node, and reparents the child
    void addChild(ProblemStoreNode *child)
    {
        m_children.push_back(child);
        child->setParent(this);
    }

    /// Returns the label of this node, if there's one
    virtual QString label() const{
        return QString();
    }

    /// Returns the node's stored problem, if there's such
    virtual IProblem::Ptr problem() const{
        return IProblem::Ptr(nullptr);
    }

private:
    /// The parent node
    ProblemStoreNode *m_parent;

    /// Children nodes
    QVector<ProblemStoreNode*> m_children;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief A ProblemStoreNode that contains a label. For example: Label for severity or path grouping of problem nodes.
 *
 * Usage:
 * @code
 * ProblemStoreNode *root = new ProblemStoreNode();
 * ...
 * root->addChild(new LabelNode(root, QStringLiteral("ERROR")));
 * root->children().last()->label(); // "ERROR"
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT LabelNode : public ProblemStoreNode
{
public:
    explicit LabelNode(ProblemStoreNode *parent = nullptr, const QString &l = QString())
        : ProblemStoreNode(parent)
        , m_label(l)
    {
    }

    ~LabelNode()
    {
    }

    QString label() const{ return m_label; }

    /// Sets the label
    void setLabel(const QString &s){ m_label = s; }

private:
    /// The label
    QString m_label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief A ProblemStoreNode that contains a problem. For example: as part of a problem list in a severity or path group.
 *
 * Usage:
 * @code
 * IProblem::Ptr problem1(new DetectedProblem());
 * ...
 * label->addChild(new ProblemNode(label, problem1));
 * label->children().last()->problem(); // Provides problem1
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemNode : public ProblemStoreNode
{
public:
    explicit ProblemNode(ProblemStoreNode *parent = nullptr, const IProblem::Ptr &problem = IProblem::Ptr(nullptr))
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

    /// Sets the problem
    void setProblem(const IProblem::Ptr &problem){
        m_problem = problem;
    }


private:
    /// The problem
    IProblem::Ptr m_problem;
};

}

#endif

