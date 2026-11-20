/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmodulerepository.h"
#include "clangmodulegraph.h"

#include <language/duchain/appendedlist.h>
#include <serialization/itemrepository.h>

#include <QMutex>
#include <QMutexLocker>

using namespace KDevelop;

namespace {

DEFINE_LIST_MEMBER_HASH(ClangModuleRepositoryItem, dependencies, KDevelop::IndexedString)

class ClangModuleRepositoryItem
{
public:
    ClangModuleRepositoryItem()
    {
        initializeAppendedLists();
    }
    ClangModuleRepositoryItem(const ClangModuleRepositoryItem& rhs, bool dynamic = true)
        : moduleName(rhs.moduleName)
        , sourceFile(rhs.sourceFile)
        , workingDir(rhs.workingDir)
        , sourceModificationTime(rhs.sourceModificationTime)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }
    ~ClangModuleRepositoryItem()
    {
        freeAppendedLists();
    }
    ClangModuleRepositoryItem& operator=(const ClangModuleRepositoryItem&) = delete;

    unsigned int hash() const
    {
        return moduleName.index();
    }
    uint itemSize() const
    {
        return dynamicSize();
    }
    uint classSize() const
    {
        return sizeof(ClangModuleRepositoryItem);
    }

    IndexedString moduleName;
    IndexedString sourceFile;
    IndexedString workingDir;
    qint64 sourceModificationTime = 0;

    START_APPENDED_LISTS(ClangModuleRepositoryItem);
    APPENDED_LIST_FIRST(ClangModuleRepositoryItem, IndexedString, dependencies);
    END_APPENDED_LISTS(ClangModuleRepositoryItem, dependencies);
};

class ClangModuleRepositoryRequest
{
public:
    explicit ClangModuleRepositoryRequest(const ClangModuleRepositoryItem& item)
        : m_item(item)
    {
    }
    enum { AverageSize = 64 };

    unsigned int hash() const
    {
        return m_item.hash();
    }
    uint itemSize() const
    {
        return m_item.itemSize();
    }
    void createItem(ClangModuleRepositoryItem* item) const
    {
        new (item) ClangModuleRepositoryItem(m_item, false);
    }
    static void destroy(ClangModuleRepositoryItem* item, AbstractItemRepository&)
    {
        item->~ClangModuleRepositoryItem();
    }
    static bool persistent(const ClangModuleRepositoryItem*)
    {
        return true;
    }
    bool equals(const ClangModuleRepositoryItem* item) const
    {
        return m_item.moduleName == item->moduleName;
    }

    const ClangModuleRepositoryItem& m_item;
};

using ModuleItemRepo = ItemRepository<ClangModuleRepositoryItem, ClangModuleRepositoryRequest>;

DEFINE_LIST_MEMBER_HASH(ClangFileScanItem, requiredModules, KDevelop::IndexedString)
DEFINE_LIST_MEMBER_HASH(ClangFileScanItem, compileFlags, KDevelop::IndexedString)
DEFINE_LIST_MEMBER_HASH(ClangFileScanItem, moduleArguments, KDevelop::IndexedString)

class ClangFileScanItem
{
public:
    ClangFileScanItem()
    {
        initializeAppendedLists();
    }
    ClangFileScanItem(const ClangFileScanItem& rhs, bool dynamic = true)
        : sourceFile(rhs.sourceFile)
        , sourceModificationTime(rhs.sourceModificationTime)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }
    ~ClangFileScanItem()
    {
        freeAppendedLists();
    }
    ClangFileScanItem& operator=(const ClangFileScanItem&) = delete;

    unsigned int hash() const
    {
        return sourceFile.index();
    }
    uint itemSize() const
    {
        return dynamicSize();
    }
    uint classSize() const
    {
        return sizeof(ClangFileScanItem);
    }

    IndexedString sourceFile;
    qint64 sourceModificationTime = 0;

    START_APPENDED_LISTS(ClangFileScanItem);
    APPENDED_LIST_FIRST(ClangFileScanItem, IndexedString, requiredModules);
    APPENDED_LIST(ClangFileScanItem, IndexedString, compileFlags, requiredModules);
    APPENDED_LIST(ClangFileScanItem, IndexedString, moduleArguments, compileFlags);
    END_APPENDED_LISTS(ClangFileScanItem, moduleArguments);
};

class ClangFileScanRequest
{
public:
    explicit ClangFileScanRequest(const ClangFileScanItem& item)
        : m_item(item)
    {
    }
    enum { AverageSize = 128 };

    unsigned int hash() const
    {
        return m_item.hash();
    }
    uint itemSize() const
    {
        return m_item.itemSize();
    }
    void createItem(ClangFileScanItem* item) const
    {
        new (item) ClangFileScanItem(m_item, false);
    }
    static void destroy(ClangFileScanItem* item, AbstractItemRepository&)
    {
        item->~ClangFileScanItem();
    }
    static bool persistent(const ClangFileScanItem*)
    {
        return true;
    }
    bool equals(const ClangFileScanItem* item) const
    {
        return m_item.sourceFile == item->sourceFile;
    }

    const ClangFileScanItem& m_item;
};

using FileScanItemRepo = ItemRepository<ClangFileScanItem, ClangFileScanRequest>;

} // namespace

class ClangModuleRepository::Private
{
public:
    explicit Private(const QString& name)
        : repository(name, &mutex)
        , fileRepository(name + QStringLiteral("_files"), &mutex)
    {
    }

    mutable QMutex mutex;
    ModuleItemRepo repository;
    FileScanItemRepo fileRepository;
};

ClangModuleRepository::ClangModuleRepository(const QString& repositoryName)
    : d(std::make_unique<Private>(repositoryName))
{
}

ClangModuleRepository::~ClangModuleRepository() = default;

ClangModuleRepository& ClangModuleRepository::self()
{
    static ClangModuleRepository s_repo;
    return s_repo;
}

void ClangModuleRepository::insert(const ModuleMetadata& metadata)
{
    if (metadata.moduleName.isEmpty()) {
        return;
    }

    ClangModuleRepositoryItem item;
    item.moduleName = metadata.moduleName;
    item.sourceFile = metadata.sourceFile;
    item.workingDir = metadata.workingDir;
    item.sourceModificationTime = metadata.sourceModificationTime;

    for (const auto& dep : metadata.dependencies) {
        if (!dep.isEmpty()) {
            item.dependenciesList().append(dep);
        }
    }

    ClangModuleRepositoryRequest request(item);
    QMutexLocker lock(&d->mutex);
    const uint existingIndex = d->repository.findIndex(request);
    if (existingIndex) {
        d->repository.deleteItem(existingIndex);
    }
    d->repository.index(request);
}

std::optional<ClangModuleRepository::ModuleMetadata>
ClangModuleRepository::find(const IndexedString& moduleName) const
{
    if (moduleName.isEmpty()) {
        return std::nullopt;
    }

    ClangModuleRepositoryItem queryItem;
    queryItem.moduleName = moduleName;
    ClangModuleRepositoryRequest request(queryItem);

    QMutexLocker lock(&d->mutex);
    const uint index = d->repository.findIndex(request);
    if (!index) {
        return std::nullopt;
    }

    const ClangModuleRepositoryItem* stored = d->repository.itemFromIndex(index);
    if (!stored) {
        return std::nullopt;
    }

    ModuleMetadata meta;
    meta.moduleName = stored->moduleName;
    meta.sourceFile = stored->sourceFile;
    meta.workingDir = stored->workingDir;
    meta.sourceModificationTime = stored->sourceModificationTime;

    const uint count = stored->dependenciesSize();
    const IndexedString* deps = stored->dependencies();
    meta.dependencies.reserve(count);
    for (uint i = 0; i < count; ++i) {
        meta.dependencies.append(deps[i]);
    }

    return meta;
}

bool ClangModuleRepository::contains(const IndexedString& moduleName) const
{
    if (moduleName.isEmpty()) {
        return false;
    }

    ClangModuleRepositoryItem queryItem;
    queryItem.moduleName = moduleName;
    ClangModuleRepositoryRequest request(queryItem);

    QMutexLocker lock(&d->mutex);
    return d->repository.findIndex(request) != 0;
}

void ClangModuleRepository::remove(const IndexedString& moduleName)
{
    if (moduleName.isEmpty()) {
        return;
    }

    ClangModuleRepositoryItem queryItem;
    queryItem.moduleName = moduleName;
    ClangModuleRepositoryRequest request(queryItem);

    QMutexLocker lock(&d->mutex);
    const uint index = d->repository.findIndex(request);
    if (index) {
        d->repository.deleteItem(index);
    }
}

void ClangModuleRepository::clear()
{
    const auto modules = allModules();
    for (const auto& mod : modules) {
        remove(mod);
    }

    QList<IndexedString> files;
    struct FileVisitor
    {
        QList<IndexedString>& result;
        bool operator()(const ClangFileScanItem* item)
        {
            if (item && !item->sourceFile.isEmpty()) {
                result.append(item->sourceFile);
            }
            return true;
        }
    } fileVisitor{files};

    {
        QMutexLocker lock(&d->mutex);
        d->fileRepository.visitAllItems(fileVisitor);
    }
    for (const auto& file : files) {
        removeFileScan(file);
    }
}

QList<IndexedString> ClangModuleRepository::allModules() const
{
    QList<IndexedString> modules;
    struct Visitor
    {
        QList<IndexedString>& result;
        bool operator()(const ClangModuleRepositoryItem* item)
        {
            if (item && !item->moduleName.isEmpty()) {
                result.append(item->moduleName);
            }
            return true;
        }
    } visitor{modules};

    QMutexLocker lock(&d->mutex);
    d->repository.visitAllItems(visitor);
    return modules;
}

void ClangModuleRepository::loadIntoGraph(ClangModuleGraph& graph) const
{
    struct GraphVisitor
    {
        ClangModuleGraph& g;
        bool operator()(const ClangModuleRepositoryItem* item)
        {
            if (!item || item->moduleName.isEmpty()) {
                return true;
            }

            QList<IndexedString> deps;
            const uint count = item->dependenciesSize();
            const IndexedString* depPtr = item->dependencies();
            deps.reserve(count);
            for (uint i = 0; i < count; ++i) {
                deps.append(depPtr[i]);
            }

            g.addModule(item->moduleName, deps, item->sourceFile);
            return true;
        }
    } visitor{graph};

    QMutexLocker lock(&d->mutex);
    d->repository.visitAllItems(visitor);
}

void ClangModuleRepository::insertFileScan(const FileScanMetadata& metadata)
{
    if (metadata.sourceFile.isEmpty()) {
        return;
    }

    ClangFileScanItem item;
    item.sourceFile = metadata.sourceFile;
    item.sourceModificationTime = metadata.sourceModificationTime;

    for (const auto& req : metadata.requiredModules) {
        if (!req.isEmpty()) {
            item.requiredModulesList().append(req);
        }
    }
    for (const auto& flag : metadata.compileFlags) {
        if (!flag.isEmpty()) {
            item.compileFlagsList().append(flag);
        }
    }
    for (const auto& arg : metadata.moduleArguments) {
        if (!arg.isEmpty()) {
            item.moduleArgumentsList().append(arg);
        }
    }

    ClangFileScanRequest request(item);
    QMutexLocker lock(&d->mutex);
    const uint existingIndex = d->fileRepository.findIndex(request);
    if (existingIndex) {
        d->fileRepository.deleteItem(existingIndex);
    }
    d->fileRepository.index(request);
}

std::optional<ClangModuleRepository::FileScanMetadata>
ClangModuleRepository::findFileScan(const IndexedString& sourceFile) const
{
    if (sourceFile.isEmpty()) {
        return std::nullopt;
    }

    ClangFileScanItem queryItem;
    queryItem.sourceFile = sourceFile;
    ClangFileScanRequest request(queryItem);

    QMutexLocker lock(&d->mutex);
    const uint index = d->fileRepository.findIndex(request);
    if (!index) {
        return std::nullopt;
    }

    const ClangFileScanItem* stored = d->fileRepository.itemFromIndex(index);
    if (!stored) {
        return std::nullopt;
    }

    FileScanMetadata meta;
    meta.sourceFile = stored->sourceFile;
    meta.sourceModificationTime = stored->sourceModificationTime;

    auto copyList = [](const IndexedString* ptr, uint count, QList<IndexedString>& out) {
        out.reserve(count);
        for (uint i = 0; i < count; ++i) {
            out.append(ptr[i]);
        }
    };

    copyList(stored->requiredModules(), stored->requiredModulesSize(), meta.requiredModules);
    copyList(stored->compileFlags(), stored->compileFlagsSize(), meta.compileFlags);
    copyList(stored->moduleArguments(), stored->moduleArgumentsSize(), meta.moduleArguments);

    return meta;
}

void ClangModuleRepository::removeFileScan(const IndexedString& sourceFile)
{
    if (sourceFile.isEmpty()) {
        return;
    }

    ClangFileScanItem queryItem;
    queryItem.sourceFile = sourceFile;
    ClangFileScanRequest request(queryItem);

    QMutexLocker lock(&d->mutex);
    const uint index = d->fileRepository.findIndex(request);
    if (index) {
        d->fileRepository.deleteItem(index);
    }
}
