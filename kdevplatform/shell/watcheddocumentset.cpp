/*
    SPDX-FileCopyrightText: 2010 Dmitry Risenberg <dmitry.risenberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "watcheddocumentset.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QFileInfo>

namespace KDevelop
{

enum ActionFlag {
    DoUpdate = 1,
    DoEmit = 2
};
Q_DECLARE_FLAGS(ActionFlags, ActionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ActionFlags)

class WatchedDocumentSetPrivate : public QObject
{
    Q_OBJECT

public:
    using DocumentSet = WatchedDocumentSet::DocumentSet;

    explicit WatchedDocumentSetPrivate(WatchedDocumentSet* documentSet)
        : m_documentSet(documentSet)
        , m_showImports(false)
    {
        connect(DUChain::self(), &DUChain::updateReady, this, &WatchedDocumentSetPrivate::updateReady);
    }

    inline bool showImports() const
    {
        return m_showImports;
    }

    void setShowImports(bool showImports)
    {
        if (m_showImports == showImports)
            return;

        DocumentSet oldImports = m_imports;

        m_showImports = showImports;
        updateImports();

        if (m_imports != oldImports)
            emit m_documentSet->changed();
    }

    inline const DocumentSet& documents() const
    {
        return m_documents;
    }

    inline const DocumentSet& imports() const
    {
        return m_imports;
    }

    inline void doUpdate(ActionFlags flags)
    {
        if (flags.testFlag(DoUpdate))
            updateImports();

        if (flags.testFlag(DoEmit))
            emit m_documentSet->changed();
    }

    void setDocuments(const DocumentSet& docs, ActionFlags flags = {})
    {
        m_documents = docs;
        doUpdate(flags);
    }

    bool addDocument(const IndexedString& doc, ActionFlags flags = {})
    {
        if (m_documents.contains(doc))
            return false;

        m_documents.insert(doc);
        doUpdate(flags);
        return true;
    }

    bool delDocument(const IndexedString& doc, ActionFlags flags = {})
    {
        const auto documentIt = m_documents.find(doc);
        if (documentIt == m_documents.end())
            return false;

        m_documents.erase(documentIt);
        doUpdate(flags);
        return true;
    }

    void renameDocument(const IndexedString& previousUrl, const IndexedString& currentUrl, ActionFlags flags = {})
    {
        const bool deleted = delDocument(previousUrl);
        const bool added = addDocument(currentUrl);
        if (deleted || added) {
            doUpdate(flags);
        }
    }

    void updateImports()
    {
        if (!m_showImports) {
            if (!m_imports.isEmpty()) {
                m_imports.clear();
                return;
            }
            return;
        }

        getImportsFromDUChain();
    }

    void clear()
    {
        m_documents.clear();
        m_imports.clear();
        emit m_documentSet->changed();
    }

private:
    void getImportsFromDU(TopDUContext* context, QSet<TopDUContext*>& visitedContexts)
    {
        if (!context || visitedContexts.contains(context))
            return;

        visitedContexts.insert(context);
        const auto importedParentContexts = context->importedParentContexts();
        for (const DUContext::Import& ctx : importedParentContexts) {
            auto* topCtx = dynamic_cast<TopDUContext*>(ctx.context(nullptr));

            if (topCtx)
                getImportsFromDU(topCtx, visitedContexts);
        }
    }

    void getImportsFromDUChain()
    {
        KDevelop::DUChainReadLocker lock;
        QSet<TopDUContext*> visitedContexts;

        m_imports.clear();
        for (const IndexedString& doc : std::as_const(m_documents)) {
            TopDUContext* ctx = DUChain::self()->chainForDocument(doc);
            getImportsFromDU(ctx, visitedContexts);
            visitedContexts.remove(ctx);
        }

        for (TopDUContext* ctx : std::as_const(visitedContexts)) {
            m_imports.insert(ctx->url());
        }
    }

    void updateReady(const IndexedString& doc, const ReferencedTopDUContext&)
    {
        if (!m_showImports || !m_documents.contains(doc))
            return;

        DocumentSet oldImports = m_imports;

        updateImports();
        if (m_imports != oldImports)
            emit m_documentSet->changed();
    }

    WatchedDocumentSet* m_documentSet;

    DocumentSet m_documents;
    DocumentSet m_imports;

    bool m_showImports;
};

WatchedDocumentSet::WatchedDocumentSet(QObject* parent)
    : QObject(parent)
    , d_ptr(new WatchedDocumentSetPrivate(this))
{
}

WatchedDocumentSet::~WatchedDocumentSet()
{
}

bool WatchedDocumentSet::showImports() const
{
    Q_D(const WatchedDocumentSet);

    return d->showImports();
}

void WatchedDocumentSet::setShowImports(bool showImports)
{
    Q_D(WatchedDocumentSet);

    d->setShowImports(showImports);
}

void WatchedDocumentSet::setCurrentDocument(const IndexedString&)
{
}

WatchedDocumentSet::DocumentSet WatchedDocumentSet::get() const
{
    Q_D(const WatchedDocumentSet);

    return d->documents();
}

WatchedDocumentSet::DocumentSet WatchedDocumentSet::imports() const
{
    Q_D(const WatchedDocumentSet);

    return d->imports();
}

CurrentDocumentSet::CurrentDocumentSet(const IndexedString& document, QObject* parent)
    : WatchedDocumentSet(parent)
{
    Q_D(WatchedDocumentSet);

    d->setDocuments({document}, DoUpdate);
}

void CurrentDocumentSet::setCurrentDocument(const IndexedString& url)
{
    Q_D(WatchedDocumentSet);

    d->setDocuments({url}, DoUpdate | DoEmit);
}

ProblemScope CurrentDocumentSet::scope() const
{
    return CurrentDocument;
}

OpenDocumentSet::OpenDocumentSet(QObject* parent)
    : WatchedDocumentSet(parent)
{
    Q_D(WatchedDocumentSet);

    const auto documents = ICore::self()->documentController()->openDocuments();
    for (IDocument* doc : documents) {
        d->addDocument(IndexedString(doc->url()));
    }
    d->updateImports();

    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this, &OpenDocumentSet::documentClosed);
    connect(ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this, &OpenDocumentSet::documentCreated);
    connect(ICore::self()->documentController(), &IDocumentController::documentUrlChanged, this,
            &OpenDocumentSet::documentUrlChanged);
}

void OpenDocumentSet::documentClosed(IDocument* doc)
{
    Q_D(WatchedDocumentSet);

    d->delDocument(IndexedString(doc->url()), DoUpdate | DoEmit);
}

void OpenDocumentSet::documentCreated(IDocument* doc)
{
    Q_D(WatchedDocumentSet);

    d->addDocument(IndexedString(doc->url()), DoUpdate | DoEmit);
}

void OpenDocumentSet::documentUrlChanged(IDocument* doc, const QUrl& previousUrl)
{
    Q_D(WatchedDocumentSet);

    if (doc->textDocument()) {
        d->renameDocument(IndexedString{previousUrl}, IndexedString{doc->url()}, DoUpdate | DoEmit);
    }
}

ProblemScope OpenDocumentSet::scope() const
{
    return OpenDocuments;
}

ProjectSet::ProjectSet(QObject* parent)
    : WatchedDocumentSet(parent)
{
    // Multiple projects can share a project file manager, e.g. CMakeManager. When a project P that
    // is being opened uses an already tracked project file manager, fileAdded() is invoked for each file
    // that belongs to P. If P contains thousands of files with many problems, KDevelop UI freezes,
    // because the ProblemReporterModel model is reset on each document set change.
    // Suspend adding files while a project is being opened. Derived classes connect to the signal
    // IProjectController::projectOpened and add all of the newly opened project's files at once, if needed.
    const auto* const projectController = ICore::self()->projectController();
    connect(projectController, &IProjectController::projectAboutToBeOpened, this, [this] {
        m_pauseAddingFiles = true;
    });
    const auto projectOpeningOver = [this] {
        m_pauseAddingFiles = false;
    };
    connect(projectController, &IProjectController::projectOpened, this, projectOpeningOver);
    connect(projectController, &IProjectController::projectOpeningAborted, this, projectOpeningOver);
}

void ProjectSet::fileAdded(ProjectFileItem* file)
{
    Q_D(WatchedDocumentSet);

    if (m_pauseAddingFiles) {
        return;
    }

    const auto path = IndexedString(file->indexedPath());
    if (include(path)) {
        d->addDocument(path, DoUpdate | DoEmit);
    }
}

void ProjectSet::fileRemoved(ProjectFileItem* file)
{
    Q_D(WatchedDocumentSet);

    d->delDocument(IndexedString(file->indexedPath()), DoUpdate | DoEmit);
}

void ProjectSet::fileRenamed(const Path& oldFile, ProjectFileItem* newFile)
{
    Q_D(WatchedDocumentSet);

    d->delDocument(IndexedString(oldFile.pathOrUrl()));
    fileAdded(newFile);
}

static const QObject* qobjectProjectFileManager(const IProjectFileManager* projectFileManager)
{
    // The implementation should derive from QObject somehow
    return dynamic_cast<const QObject*>(projectFileManager);
}

void ProjectSet::trackProjectFiles(const IProjectFileManager* projectFileManager)
{
    auto* const fileManager = qobjectProjectFileManager(projectFileManager);
    if (!fileManager) {
        return;
    }
    // can't use new signal/slot syntax here, IProjectFileManager is no a QObject
    connect(fileManager, SIGNAL(fileAdded(KDevelop::ProjectFileItem*)), this,
            SLOT(fileAdded(KDevelop::ProjectFileItem*)));
    connect(fileManager, SIGNAL(fileRemoved(KDevelop::ProjectFileItem*)), this,
            SLOT(fileRemoved(KDevelop::ProjectFileItem*)));
    connect(fileManager, SIGNAL(fileRenamed(KDevelop::Path, KDevelop::ProjectFileItem*)), this,
            SLOT(fileRenamed(KDevelop::Path, KDevelop::ProjectFileItem*)));
}

void ProjectSet::stopTrackingProjectFiles(const IProjectFileManager* projectFileManager)
{
    auto* const fileManager = qobjectProjectFileManager(projectFileManager);
    if (!fileManager) {
        return;
    }
    fileManager->disconnect(this);
}

CurrentProjectSet::CurrentProjectSet(const IndexedString& document, QObject* parent)
    : ProjectSet(parent)
    , m_currentDocumentUrl(document.toUrl())
{
    handleCurrentDocumentChange();

    // Opening or closing a project can change m_currentDocument's associated project.
    const auto* const projectController = ICore::self()->projectController();
    connect(projectController, &IProjectController::projectOpened, this,
            &CurrentProjectSet::handleCurrentDocumentChange);
    connect(projectController, &IProjectController::projectClosed, this,
            &CurrentProjectSet::handleCurrentDocumentChange);
}

void CurrentProjectSet::setCurrentDocument(const IndexedString& url)
{
    m_currentDocumentUrl = url.toUrl();
    handleCurrentDocumentChange();
}

void CurrentProjectSet::handleCurrentDocumentChange()
{
    Q_D(WatchedDocumentSet);

    const auto* const projectForUrl = ICore::self()->projectController()->findProjectForUrl(m_currentDocumentUrl);
    if (projectForUrl == m_currentProject) {
        return;
    }

    if (m_currentProject) {
        stopTrackingProjectFiles(m_currentProject->projectFileManager());
    }
    m_currentProject = projectForUrl;

    if (m_currentProject) {
        d->setDocuments(m_currentProject->fileSet(), DoUpdate | DoEmit);
        trackProjectFiles(m_currentProject->projectFileManager());
    } else {
        d->clear();
    }
}

bool CurrentProjectSet::include(const IndexedString& url) const
{
    return m_currentProject && m_currentProject->fileSet().contains(url);
}

ProblemScope CurrentProjectSet::scope() const
{
    return CurrentProject;
}

AllProjectSet::AllProjectSet(QObject* parent)
    : AllProjectSet(InitFlag::LoadOnInit, parent)
{
}

AllProjectSet::AllProjectSet(InitFlag initFlag, QObject* parent)
    : ProjectSet(parent)
{
    switch (initFlag) {
    case InitFlag::LoadOnInit:
        reload();
        break;
    case InitFlag::SkipLoadOnInit:
        break;
    }

    const auto* const projectController = ICore::self()->projectController();
    connect(projectController, &IProjectController::projectOpened, this, &AllProjectSet::projectOpened);
    // When a project is closed, simply reloading can be more efficient than removing the closed project's
    // files, especially if this was the only open project. Furthermore, a single file can belong to
    // multiple open projects. In this corner case removing the closed project's files is incorrect.
    connect(projectController, &IProjectController::projectClosed, this, &AllProjectSet::reload);
}

void AllProjectSet::reload()
{
    Q_D(WatchedDocumentSet);

    d->clear();

    QSet<const IProjectFileManager*> openProjectFileManagers;

    const auto projects = ICore::self()->projectController()->projects();
    for (const IProject* project : projects) {
        addProjectFiles(*project);
        openProjectFileManagers.insert(project->projectFileManager());
    }

    if (m_trackedProjectFileManagers != openProjectFileManagers) {
        const auto removedFileManagers = m_trackedProjectFileManagers - openProjectFileManagers;
        for (auto* f : removedFileManagers) {
            stopTrackingProjectFiles(f);
        }

        const auto addedFileManagers = openProjectFileManagers - m_trackedProjectFileManagers;
        for (auto* f : addedFileManagers) {
            trackProjectFiles(f);
        }

        m_trackedProjectFileManagers = openProjectFileManagers;
    }

    d->updateImports();
    emit changed();
}

void AllProjectSet::projectOpened(const IProject* project)
{
    Q_D(WatchedDocumentSet);

    addProjectFiles(*project);

    const auto* const projectFileManager = project->projectFileManager();
    if (!m_trackedProjectFileManagers.contains(projectFileManager)) {
        trackProjectFiles(projectFileManager);
        m_trackedProjectFileManagers.insert(projectFileManager);
    }

    d->updateImports();
    emit changed();
}

void AllProjectSet::addProjectFiles(const IProject& project)
{
    Q_D(WatchedDocumentSet);

    const auto fileSet = project.fileSet();
    for (const IndexedString& document : fileSet) {
        if (include(document)) {
            d->addDocument(document);
        }
    }
}

ProblemScope AllProjectSet::scope() const
{
    return AllProjects;
}

DocumentsInPathSet::DocumentsInPathSet(const QString& path, QObject* parent)
    : AllProjectSet(InitFlag::SkipLoadOnInit, parent)
    , m_path(path)
{
    reload();
}

ProblemScope DocumentsInPathSet::scope() const
{
    return DocumentsInPath;
}

void DocumentsInPathSet::setPath(const QString& path)
{
    if (m_path == path) {
        return;
    }

    m_path = path;
    reload();
}

bool DocumentsInPathSet::include(const IndexedString& path) const
{
    if (m_path.isEmpty()) {
        return true;
    }

    return path.str().contains(m_path, Qt::CaseInsensitive);
}

DocumentsInCurrentPathSet::DocumentsInCurrentPathSet(const IndexedString& document, QObject* parent)
    : DocumentsInPathSet(QFileInfo(document.str()).path(), parent)
{
}

ProblemScope DocumentsInCurrentPathSet::scope() const
{
    return DocumentsInCurrentPath;
}

void DocumentsInCurrentPathSet::setCurrentDocument(const IndexedString& document)
{
    setPath(QFileInfo(document.str()).path());
}

BypassSet::BypassSet(QObject* parent)
    : WatchedDocumentSet(parent)
{
}

ProblemScope BypassSet::scope() const
{
    return BypassScopeFilter;
}

}

#include "watcheddocumentset.moc"
#include "moc_watcheddocumentset.cpp"
