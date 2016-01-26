/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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


#ifndef PROBLEMSTORE_H
#define PROBLEMSTORE_H

#include <QObject>
#include <QList>
#include <shell/shellexport.h>
#include <serialization/indexedstring.h>
#include <interfaces/iproblem.h>

struct ProblemStorePrivate;

namespace KDevelop
{

class WatchedDocumentSet;
class ProblemStoreNode;

/**
 * @brief Stores and handles problems. Does no ordering or filtering, those should be done in subclasses.
 *
 * Used to store problems that are ordered, filtered somewhere else. For example: DUChain problems gathered by ProblemReporter.
 * Stores the problems in ProblemStoreNodes.
 * When implementing a subclass, first and foremost the rebuild method needs to be implemented, which is called every time there's a change in scope and severity filter.
 * If grouping is desired then also the setGrouping method must be implemented.
 * ProblemStore depending on settings uses CurrentDocumentSet, OpenDocumentSet, CurrentProjectSet, or AllProjectSet for scope support (NOTE: Filtering still has to be implemented in either a subclass, or somewhere else).
 * When the scope changes it emits the changed() signal.
 *
 * Scope set / query methods:
 * \li setScope()
 * \li scope()
 *
 * Valid scope settings:
 * \li CurrentDocument
 * \li OpenDocuments
 * \li CurrentProject
 * \li AllProjects
 * \li BypassScopeFilter
 *
 * Usage example:
 * @code
 * QVector<IProblem::Ptr> problems;
 * // Add 4 problems
 * ...
 * ProblemStore *store = new ProblemStore();
 * store->setProblems(problems);
 * store->count(); // Returns 4
 *
 * ProblemStoreNode *node = store->findNode(0); // Returns the node with the first problem
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemStore : public QObject
{
    Q_OBJECT
public:
    explicit ProblemStore(QObject *parent = nullptr);
    ~ProblemStore() override;

    /// Adds a problem
    virtual void addProblem(const IProblem::Ptr &problem);

    /// Clears the current problems, and adds new ones from a list
    virtual void setProblems(const QVector<IProblem::Ptr> &problems);

    /// Finds the specified node
    virtual const ProblemStoreNode* findNode(int row, ProblemStoreNode *parent = nullptr) const;

    /// Returns the number of problems
    virtual int count(ProblemStoreNode *parent = nullptr) const;

    /// Clears the problems
    virtual void clear();

    /// Rebuild the problems list, if applicable. It does nothing in the base class.
    virtual void rebuild();

    /// Specifies the severity filter
    virtual void setSeverity(int severity);///old-style severity access

    virtual void setSeverities(KDevelop::IProblem::Severities severities);///new-style severity access

    /// Retrives the severity filter settings
    int severity() const;///old-style severity access

    KDevelop::IProblem::Severities severities() const;//new-style severity access

    /// Retrieves the currently watched document set
    WatchedDocumentSet* documents() const;

    /// Sets the scope filter
    void setScope(int scope);

    /// Returns the current scope
    int scope() const;

    /// Sets the grouping method
    virtual void setGrouping(int grouping);

    /// Sets whether we should bypass the scope filter
    virtual void setBypassScopeFilter(bool bypass);

    /// Sets the currently shown document (in the editor, it's triggered by the IDE)
    void setCurrentDocument(const IndexedString &doc);

    /// Retrives the path of the current document
    const KDevelop::IndexedString& currentDocument() const;

signals:
    /// Emitted when the problems change
    void changed();

    /// Emitted before the problemlist is rebuilt
    void beginRebuild();

    /// Emitted once the problemlist has been rebuilt
    void endRebuild();

private slots:
    /// Triggered when the watched document set changes. E.g.:document closed, new one added, etc
    virtual void onDocumentSetChanged();

protected:
    ProblemStoreNode* rootNode();

private:
    QScopedPointer<ProblemStorePrivate> d;
};

}

#endif

