/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmodulegraph.h"

#include <functional>

using namespace KDevelop;

void ClangModuleGraph::addModule(const IndexedString& name,
                                 const QList<IndexedString>& dependencies,
                                 const IndexedString& sourceFile)
{
    if (name.isEmpty()) {
        return;
    }

    if (m_nodes.contains(name)) {
        const auto& oldNode = m_nodes.value(name);
        for (const auto& dep : oldNode.dependencies) {
            auto it = m_dependents.find(dep);
            if (it != m_dependents.end()) {
                it.value().remove(name);
                if (it.value().isEmpty()) {
                    m_dependents.erase(it);
                }
            }
        }
    }

    ModuleNode node;
    node.name = name;
    node.sourceFile = sourceFile;
    node.dependencies = dependencies;
    m_nodes.insert(name, node);

    for (const auto& dep : dependencies) {
        if (!dep.isEmpty()) {
            m_dependents[dep].insert(name);
        }
    }
}

void ClangModuleGraph::removeModule(const IndexedString& name)
{
    auto nodeIt = m_nodes.find(name);
    if (nodeIt == m_nodes.end()) {
        return;
    }

    for (const auto& dep : nodeIt.value().dependencies) {
        auto it = m_dependents.find(dep);
        if (it != m_dependents.end()) {
            it.value().remove(name);
            if (it.value().isEmpty()) {
                m_dependents.erase(it);
            }
        }
    }

    m_nodes.erase(nodeIt);
    m_dependents.remove(name);
}

void ClangModuleGraph::clear()
{
    m_nodes.clear();
    m_dependents.clear();
}

bool ClangModuleGraph::contains(const IndexedString& name) const
{
    return m_nodes.contains(name);
}

ClangModuleGraph::ModuleNode ClangModuleGraph::moduleNode(const IndexedString& name) const
{
    return m_nodes.value(name);
}

QList<IndexedString> ClangModuleGraph::dependenciesOf(const IndexedString& name) const
{
    return m_nodes.value(name).dependencies;
}

QList<IndexedString> ClangModuleGraph::dependentsOf(const IndexedString& name) const
{
    const auto set = m_dependents.value(name);
    return set.values();
}

QList<IndexedString> ClangModuleGraph::allModules() const
{
    return m_nodes.keys();
}

int ClangModuleGraph::size() const
{
    return m_nodes.size();
}

bool ClangModuleGraph::hasCycle() const
{
    return !findCycle().isEmpty();
}

QList<IndexedString> ClangModuleGraph::findCycle() const
{
    QSet<IndexedString> visited;
    QSet<IndexedString> visiting;
    QList<IndexedString> path;
    QList<IndexedString> cycle;

    std::function<bool(const IndexedString&)> dfs = [&](const IndexedString& current) -> bool {
        visiting.insert(current);
        path.append(current);

        const auto node = m_nodes.value(current);
        for (const auto& dep : node.dependencies) {
            if (visiting.contains(dep)) {
                const int cycleStart = path.indexOf(dep);
                if (cycleStart != -1) {
                    cycle = path.mid(cycleStart);
                    cycle.append(dep);
                } else {
                    cycle = {dep, current, dep};
                }
                return true;
            }

            if (!visited.contains(dep) && m_nodes.contains(dep)) {
                if (dfs(dep)) {
                    return true;
                }
            }
        }

        path.removeLast();
        visiting.remove(current);
        visited.insert(current);
        return false;
    };

    for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it) {
        if (!visited.contains(it.key())) {
            if (dfs(it.key())) {
                return cycle;
            }
        }
    }

    return {};
}

ClangModuleGraph::TopologicalSortResult ClangModuleGraph::topologicalSort() const
{
    TopologicalSortResult result;
    QSet<IndexedString> visited;
    QSet<IndexedString> visiting;
    QList<IndexedString> path;

    std::function<bool(const IndexedString&)> dfs = [&](const IndexedString& current) -> bool {
        visiting.insert(current);
        path.append(current);

        const auto node = m_nodes.value(current);
        for (const auto& dep : node.dependencies) {
            if (visiting.contains(dep)) {
                const int cycleStart = path.indexOf(dep);
                if (cycleStart != -1) {
                    result.cycle = path.mid(cycleStart);
                    result.cycle.append(dep);
                } else {
                    result.cycle = {dep, current, dep};
                }
                result.success = false;
                return false;
            }

            if (!visited.contains(dep) && m_nodes.contains(dep)) {
                if (!dfs(dep)) {
                    return false;
                }
            }
        }

        path.removeLast();
        visiting.remove(current);
        visited.insert(current);
        result.order.append(current);
        return true;
    };

    for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it) {
        if (!visited.contains(it.key())) {
            if (!dfs(it.key())) {
                return result;
            }
        }
    }

    result.success = true;
    return result;
}

ClangModuleGraph::TopologicalSortResult ClangModuleGraph::dependenciesFor(const IndexedString& targetModule) const
{
    return dependenciesFor(QList<IndexedString>{targetModule});
}

ClangModuleGraph::TopologicalSortResult ClangModuleGraph::dependenciesFor(const QList<IndexedString>& targetModules) const
{
    TopologicalSortResult result;
    if (targetModules.isEmpty()) {
        result.success = true;
        return result;
    }

    QSet<IndexedString> visited;
    QSet<IndexedString> visiting;
    QList<IndexedString> path;

    std::function<bool(const IndexedString&)> dfs = [&](const IndexedString& current) -> bool {
        visiting.insert(current);
        path.append(current);

        const auto node = m_nodes.value(current);
        for (const auto& dep : node.dependencies) {
            if (visiting.contains(dep)) {
                const int cycleStart = path.indexOf(dep);
                if (cycleStart != -1) {
                    result.cycle = path.mid(cycleStart);
                    result.cycle.append(dep);
                } else {
                    result.cycle = {dep, current, dep};
                }
                result.success = false;
                return false;
            }

            if (!visited.contains(dep) && m_nodes.contains(dep)) {
                if (!dfs(dep)) {
                    return false;
                }
            }
        }

        path.removeLast();
        visiting.remove(current);
        visited.insert(current);
        result.order.append(current);
        return true;
    };

    for (const auto& target : targetModules) {
        if (!visited.contains(target) && m_nodes.contains(target)) {
            if (!dfs(target)) {
                return result;
            }
        }
    }

    result.success = true;
    return result;
}

QString ClangModuleGraph::formatCycleString(const QList<IndexedString>& cycle)
{
    if (cycle.isEmpty()) {
        return {};
    }

    QStringList parts;
    parts.reserve(cycle.size());
    for (const auto& node : cycle) {
        parts.append(node.str());
    }
    return parts.join(QStringLiteral(" -> "));
}
