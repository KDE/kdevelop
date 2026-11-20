/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGMODULEREPOSITORY_H
#define CLANGMODULEREPOSITORY_H

#include "clangprivateexport.h"

#include <serialization/indexedstring.h>

#include <QList>
#include <QString>
#include <memory>
#include <optional>

class ClangModuleGraph;

class KDEVCLANGPRIVATE_EXPORT ClangModuleRepository
{
public:
    struct ModuleMetadata
    {
        KDevelop::IndexedString moduleName;
        KDevelop::IndexedString sourceFile;
        KDevelop::IndexedString workingDir;
        qint64 sourceModificationTime = 0;
        QList<KDevelop::IndexedString> dependencies;
    };

    struct FileScanMetadata
    {
        KDevelop::IndexedString sourceFile;
        qint64 sourceModificationTime = 0;
        QList<KDevelop::IndexedString> requiredModules;
        QList<KDevelop::IndexedString> compileFlags;
        QList<KDevelop::IndexedString> moduleArguments;
    };

    explicit ClangModuleRepository(const QString& repositoryName = QStringLiteral("ClangModuleRepository"));
    ~ClangModuleRepository();

    static ClangModuleRepository& self();

    /**
     * Store or update module metadata in the persistent repository.
     */
    void insert(const ModuleMetadata& metadata);

    /**
     * Look up module metadata by logical module name.
     */
    std::optional<ModuleMetadata> find(const KDevelop::IndexedString& moduleName) const;

    /**
     * Check whether a module is present in the repository.
     */
    bool contains(const KDevelop::IndexedString& moduleName) const;

    /**
     * Remove module metadata from the repository.
     */
    void remove(const KDevelop::IndexedString& moduleName);

    /**
     * Clear all module metadata from the repository.
     */
    void clear();

    /**
     * Returns all indexed module names.
     */
    QList<KDevelop::IndexedString> allModules() const;

    /**
     * Populate a ClangModuleGraph with all module entries from the repository.
     */
    void loadIntoGraph(ClangModuleGraph& graph) const;

    /**
     * Store or update per-file scan results (required modules, compile flags,
     * and resolved module arguments) in the persistent repository.
     */
    void insertFileScan(const FileScanMetadata& metadata);

    /**
     * Look up cached scan results for @p sourceFile.
     */
    std::optional<FileScanMetadata> findFileScan(const KDevelop::IndexedString& sourceFile) const;

    /**
     * Remove cached scan results for @p sourceFile.
     */
    void removeFileScan(const KDevelop::IndexedString& sourceFile);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // CLANGMODULEREPOSITORY_H
