/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FILTEREDPROBLEMSTORE_H
#define FILTEREDPROBLEMSTORE_H

#include "problemstore.h"
#include "problemconstants.h"


namespace KDevelop
{

class FilteredProblemStorePrivate;

/**
 * @brief ProblemStore subclass that can group by severity, and path, and filter by scope, and severity.
 *
 * Internally grouping is implemented using a tree structure.
 * When grouping is on, the top level nodes are the groups, and their children are the nodes containing the problems that belong into that node.
 * If the problems have diagnostics, then the diagnostics are added as children nodes as well. This was implemented so they can be browsed in a model/view architecture.
 * When grouping is off, the top level nodes are the problem nodes.
 *
 * Grouping can be set and queried using the following methods
 * \li setGrouping();
 * \li grouping();
 *
 * Valid grouping settings:
 * \li NoGrouping
 * \li PathGrouping
 * \li SeverityGrouping
 *
 * Severity filter can be set and queried using the following methods
 * \li setSeverity()
 * \li severity()
 *
 * Valid severity setting:
 * \li IProblem::Error
 * \li IProblem::Warning
 * \li IProblem::Hint
 *
 * When changing the grouping or filtering method the following signals are emitted in order:
 * \li beginRebuild()
 * \li endRebuild()
 * \li changed()
 *
 * Usage example:
 * @code
 * IProblem::Ptr problem(new DetectedProblem);
 * problem->setSeverity(IProblem::Error);
 * ...
 * FilteredProblemStore *store = new FilteredProblemStore();
 * store->setGrouping(SeverityGrouping);
 * store->addProblem(problem);
 * ProblemStoreNode *label = store->findNode(0); // Label node with the label "Error"
 * ProblemNode *problemNode = dynamic_cast<ProblemNode*>(label->child(0)); // the node with the problem
 * problemNode->problem(); // The problem we added
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT FilteredProblemStore : public ProblemStore
{
    Q_OBJECT
public:
    explicit FilteredProblemStore(QObject *parent = nullptr);
    ~FilteredProblemStore() override;

    /// Adds a problem, which is then filtered and also added to the filtered problem list if it matches the filters
    void addProblem(const IProblem::Ptr &problem) override;

    /// Retrieves the specified node
    const ProblemStoreNode* findNode(int row, ProblemStoreNode *parent = nullptr) const override;

    /// Retrieves the number of filtered problems
    int count(ProblemStoreNode *parent = nullptr) const override;

    /// Clears the problems
    void clear() override;

    /// Rebuilds the filtered problem list
    void rebuild() override;

    /// Set the grouping method. See the GroupingMethod enum.
    void setGrouping(int grouping) override;

    /// Tells which grouping strategy is currently in use
    int grouping() const;

private:
    friend class FilteredProblemStorePrivate;
    const QScopedPointer<class FilteredProblemStorePrivate> d_ptr;
    Q_DECLARE_PRIVATE(FilteredProblemStore)
};

}

#endif
