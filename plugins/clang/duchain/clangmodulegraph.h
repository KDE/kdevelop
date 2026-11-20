/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGMODULEGRAPH_H
#define CLANGMODULEGRAPH_H

#include "clangprivateexport.h"

#include <serialization/indexedstring.h>

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

class KDEVCLANGPRIVATE_EXPORT ClangModuleGraph
{
public:
    struct ModuleNode
    {
        KDevelop::IndexedString name;
        KDevelop::IndexedString sourceFile;
        QList<KDevelop::IndexedString> dependencies;
    };

    struct TopologicalSortResult
    {
        bool success = false;
        QList<KDevelop::IndexedString> order;
        QList<KDevelop::IndexedString> cycle;
    };

    ClangModuleGraph() = default;
    ~ClangModuleGraph() = default;

    /**
     * Add or update a module node in the graph.
     */
    void addModule(const KDevelop::IndexedString& name,
                   const QList<KDevelop::IndexedString>& dependencies,
                   const KDevelop::IndexedString& sourceFile = {});

    /**
     * Remove a module and its edges from the graph.
     */
    void removeModule(const KDevelop::IndexedString& name);

    /**
     * Clear all nodes and edges from the graph.
     */
    void clear();

    /**
     * Check if a module exists in the graph.
     */
    bool contains(const KDevelop::IndexedString& name) const;

    /**
     * Returns the node information for a module, or empty node if not found.
     */
    ModuleNode moduleNode(const KDevelop::IndexedString& name) const;

    /**
     * Direct dependencies required by @p name.
     */
    QList<KDevelop::IndexedString> dependenciesOf(const KDevelop::IndexedString& name) const;

    /**
     * Direct dependents (reverse dependencies) that import @p name.
     */
    QList<KDevelop::IndexedString> dependentsOf(const KDevelop::IndexedString& name) const;

    /**
     * All registered module names in the graph.
     */
    QList<KDevelop::IndexedString> allModules() const;

    /**
     * Total number of modules in the graph.
     */
    int size() const;

    /**
     * Check whether the graph contains any circular dependencies.
     */
    bool hasCycle() const;

    /**
     * Find a circular dependency chain if one exists (e.g. [A, B, C, A]).
     * Returns an empty list if the graph is a valid DAG.
     */
    QList<KDevelop::IndexedString> findCycle() const;

    /**
     * Computes a topological sort of the entire module graph.
     * Dependencies appear before the modules that require them.
     * If a cycle is detected, returns success = false and the cycle chain.
     */
    TopologicalSortResult topologicalSort() const;

    /**
     * Computes a topological sort of all transitive dependencies required
     * to compile @p targetModule, with @p targetModule at the end.
     */
    TopologicalSortResult dependenciesFor(const KDevelop::IndexedString& targetModule) const;

    /**
     * Computes a topological sort of all transitive dependencies required
     * to compile @p targetModules, with dependencies appearing before dependents.
     */
    TopologicalSortResult dependenciesFor(const QList<KDevelop::IndexedString>& targetModules) const;

    /**
     * Format a cycle chain for user diagnostic reporting:
     * e.g. "A -> B -> C -> A"
     */
    static QString formatCycleString(const QList<KDevelop::IndexedString>& cycle);

private:
    QHash<KDevelop::IndexedString, ModuleNode> m_nodes;
    QHash<KDevelop::IndexedString, QSet<KDevelop::IndexedString>> m_dependents;
};

#endif // CLANGMODULEGRAPH_H
