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

#ifndef FILTEREDPROBLEMSTORE_H
#define FILTEREDPROBLEMSTORE_H

#include "problemstore.h"
#include "problemconstants.h"

namespace
{
class GroupingStrategy;
}

namespace KDevelop
{

// ProblemStore subclass that can filter and group problems
// Filtering is based on severity and source file criteria, see match() for details
// Grouping is done by grouping strategies, see GroupingStrategy and it's subclasses
class KDEVPLATFORMSHELL_EXPORT FilteredProblemStore : public ProblemStore
{
    Q_OBJECT
public:
    explicit FilteredProblemStore(QObject *parent = nullptr);
    ~FilteredProblemStore();

    // Adds a problem, which is then filtered and also added to the filtered problem list if it matches the filters
    void addProblem(const IProblem::Ptr &problem) override;

    // Retrieves the specified node
    const ProblemStoreNode* findNode(int row, ProblemStoreNode *parent = nullptr) const;

    // Retrieves the number of filtered problems
    int count(ProblemStoreNode *parent = nullptr) const;

    // Clears the problems
    void clear();

    // Rebuilds the filtered problem list
    void rebuild();

    // Set the grouping method. See the GroupingMethod enum.
    void setGrouping(int grouping);

    // Sets whether we should bypass the scope filter
    void setBypassScopeFilter(bool bypass) override;

private:
    // Tells if the problem matches the filters
    bool match(const IProblem::Ptr &problem) const;

    GroupingMethod m_grouping;
    QScopedPointer<GroupingStrategy> m_strategy;
    bool m_bypassScopeFilter;
};

}

#endif
