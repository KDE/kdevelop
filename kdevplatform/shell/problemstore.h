/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROBLEMSTORE_H
#define PROBLEMSTORE_H

#include <QObject>
#include <shell/shellexport.h>
#include <interfaces/iproblem.h>

#include "problemconstants.h"

namespace KDevelop
{

class WatchedDocumentSet;
class ProblemStoreNode;
class IndexedString;
class ProblemStorePrivate;

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

    /// Retrieve problems for selected document
    QVector<IProblem::Ptr> problems(const KDevelop::IndexedString& document) const;

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

    /// Retrieves the severity filter settings
    int severity() const;///old-style severity access

    KDevelop::IProblem::Severities severities() const;//new-style severity access

    /// Retrieves the currently watched document set
    WatchedDocumentSet* documents() const;

    /// Sets the scope filter
    void setScope(ProblemScope scope);

    /// Returns the current scope
    ProblemScope scope() const;

    /// Sets the grouping method
    virtual void setGrouping(int grouping);

    /// Set 'show imports' filter value
    void setShowImports(bool showImports);

    /// Retrieve 'show imports' filter setting
    int showImports() const;

    /// Sets the currently shown document (in the editor, it's triggered by the IDE)
    void setCurrentDocument(const IndexedString &doc);

    /// Retrieves the path of the current document
    const KDevelop::IndexedString& currentDocument() const;

    /// Sets the path to a folder to be used by the ProblemScope::DocumentsInPath scope
    void setPathForDocumentsInPathScope(const QString& path);

    /// Retrieves the path to the folder to be used by the ProblemScope::DocumentsInPath scope
    QString pathForDocumentsInPathScope() const;

Q_SIGNALS:
    /// Emitted when any store setting (grouping, scope, severity, document) is changed
    void changed();

    /// Emitted when the stored problems are changed with clear(), addProblem() and setProblems()
    /// methods. This signal emitted only when internal problems storage is really changed:
    /// for example, it is not emitted when we call clear() method for empty storage.
    void problemsChanged();

    /// Emitted before the problemlist is rebuilt
    void beginRebuild();

    /// Emitted once the problemlist has been rebuilt
    void endRebuild();

private Q_SLOTS:
    /// Triggered when the watched document set changes. E.g.:document closed, new one added, etc
    virtual void onDocumentSetChanged();

protected:
    ProblemStoreNode* rootNode() const;

private:
    const QScopedPointer<class ProblemStorePrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProblemStore)
};

}

#endif

