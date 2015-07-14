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

#include "./filteredproblemstore.h"
#include "./problem.h"
#include "./watcheddocumentset.h"
#include "./problemstorenode.h"

#include <KLocalizedString>

using namespace KDevelop;

namespace
{

// Base class for grouping strategy classes
// These classes build the problem tree based on the respective strategies
class GroupingStrategy
{
public:
    GroupingStrategy( ProblemStoreNode *root )
        : m_rootNode(root)
        , m_groupedRootNode(new ProblemStoreNode())
    {
    }

    virtual ~GroupingStrategy(){
    }

    // Add a problem to the appropriate group
    virtual void addProblem(const IProblem::Ptr &problem) = 0;

    // Find the specified noe
    const ProblemStoreNode* findNode(int row, ProblemStoreNode *parent = nullptr) const
    {
        if (parent == nullptr)
            return m_groupedRootNode->child(row);
        else
            return parent->child(row);
    }

    // Returns the number of children nodes
    int count(ProblemStoreNode *parent = nullptr)
    {
        if (parent == nullptr)
            return m_groupedRootNode->count();
        else
            return parent->count();
    }

    // Clears the problems
    virtual void clear()
    {
        m_groupedRootNode->clear();
    }

protected:
    ProblemStoreNode *m_rootNode;
    QScopedPointer<ProblemStoreNode> m_groupedRootNode;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Implements no grouping strategy, that is just stores the problems without any grouping
class NoGroupingStrategy final : public GroupingStrategy
{
public:
    NoGroupingStrategy(ProblemStoreNode *root)
        : GroupingStrategy(root)
    {
    }

    void addProblem(const IProblem::Ptr &problem) override
    {
        m_groupedRootNode->addChild(new ProblemNode(m_groupedRootNode.data(), problem));
    }

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Implements grouping based on path
class PathGroupingStrategy final : public GroupingStrategy
{
public:
    PathGroupingStrategy(ProblemStoreNode *root)
        : GroupingStrategy(root)
    {
    }

    void addProblem(const IProblem::Ptr &problem) override
    {
        QString path = problem->finalLocation().document.str();

        // See if we already have this path
        ProblemStoreNode *parent = nullptr;
        foreach (ProblemStoreNode *node, m_groupedRootNode->children()) {
            if (node->label() == path) {
                parent = node;
                break;
            }
        }

        // If not add it!
        if (parent == nullptr) {
            parent = new LabelNode(m_groupedRootNode.data(), path);
            m_groupedRootNode->addChild(parent);
        }

        parent->addChild(new ProblemNode(parent, problem));
    }

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Implements grouping based on severity
class SeverityGroupingStrategy final : public GroupingStrategy
{
public:
    enum SeverityGroups
    {
        GroupError          = 0,
        GroupWarning        = 1,
        GroupHint           = 2
    };

    SeverityGroupingStrategy(ProblemStoreNode *root)
        : GroupingStrategy(root)
    {
        // Create the groups on construction, so there's no need to search for them on addition
        m_groupedRootNode->addChild(new LabelNode(m_groupedRootNode.data(), i18n("Error")));
        m_groupedRootNode->addChild(new LabelNode(m_groupedRootNode.data(), i18n("Warning")));
        m_groupedRootNode->addChild(new LabelNode(m_groupedRootNode.data(), i18n("Hint")));
    }

    void addProblem(const IProblem::Ptr &problem) override
    {
        ProblemStoreNode *parent = nullptr;

        switch (problem->severity()) {
            case IProblem::Error: parent = m_groupedRootNode->child(GroupError); break;
            case IProblem::Warning: parent = m_groupedRootNode->child(GroupWarning); break;
            case IProblem::Hint: parent = m_groupedRootNode->child(GroupHint); break;
        }

        parent->addChild(new ProblemNode(m_groupedRootNode.data(), problem));
    }

    void clear()
    {
        m_groupedRootNode->child(GroupError)->clear();
        m_groupedRootNode->child(GroupWarning)->clear();
        m_groupedRootNode->child(GroupHint)->clear();
    }
};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace KDevelop
{

FilteredProblemStore::FilteredProblemStore(QObject *parent)
    : ProblemStore(parent)
    , m_grouping(NoGrouping)
    , m_strategy(new NoGroupingStrategy(rootNode()))
{
}

FilteredProblemStore::~FilteredProblemStore()
{
}

void FilteredProblemStore::addProblem(const IProblem::Ptr &problem)
{
    ProblemStore::addProblem(problem);

    if (match(problem))
        m_strategy->addProblem(problem);
}

const ProblemStoreNode* FilteredProblemStore::findNode(int row, ProblemStoreNode *parent) const
{
    return m_strategy->findNode(row, parent);
}

int FilteredProblemStore::count(ProblemStoreNode *parent) const
{
    return m_strategy->count(parent);
}

void FilteredProblemStore::clear()
{
    m_strategy->clear();
    ProblemStore::clear();
}

void FilteredProblemStore::rebuild()
{
    emit beginRebuild();

    m_strategy->clear();

    foreach (ProblemStoreNode *node, rootNode()->children()) {
        IProblem::Ptr problem = node->problem();
        if (match(problem)) {
            m_strategy->addProblem(problem);
        }
    }

    emit endRebuild();
}

void FilteredProblemStore::setGrouping(int grouping)
{
    GroupingMethod g = GroupingMethod(grouping);
    if(g == m_grouping)
        return;

    m_grouping = g;

    switch (g) {
        case NoGrouping: m_strategy.reset(new NoGroupingStrategy(rootNode())); break;
        case PathGrouping: m_strategy.reset(new PathGroupingStrategy(rootNode())); break;
        case SeverityGrouping: m_strategy.reset(new SeverityGroupingStrategy(rootNode())); break;
    }

    rebuild();
    emit changed();
}


bool FilteredProblemStore::match(const IProblem::Ptr &problem) const
{
    // If the problem is less severe than our filter criterion then it's discarded
    if(problem->severity() > severity())
        return false;

    // If the problem isn't in a file that's in the watched document set, it's discarded
    const WatchedDocumentSet::DocumentSet &docs = documents()->get();
    if(!docs.contains(problem->finalLocation().document))
        return false;

    return true;
}

}

