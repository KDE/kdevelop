/*
 * KDevelop Problem Reporter
 *
 * Copyright 2010 Dmitry Risenberg <dmitry.risenberg@gmail.com>
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

    void addDocument(const IndexedString& doc, ActionFlags flags = {})
    {
        if (m_documents.contains(doc))
            return;

        m_documents.insert(doc);
        doUpdate(flags);
    }

    void delDocument(const IndexedString& doc, ActionFlags flags = {})
    {
        const auto documentIt = m_documents.find(doc);
        if (documentIt == m_documents.end())
            return;

        m_documents.erase(documentIt);
        doUpdate(flags);
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
        for (const IndexedString& doc : qAsConst(m_documents)) {
            TopDUContext* ctx = DUChain::self()->chainForDocument(doc);
            getImportsFromDU(ctx, visitedContexts);
            visitedContexts.remove(ctx);
        }

        for (TopDUContext* ctx : qAsConst(visitedContexts)) {
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

ProblemScope OpenDocumentSet::scope() const
{
    return OpenDocuments;
}

ProjectSet::ProjectSet(QObject* parent)
    : WatchedDocumentSet(parent)
{
}

void ProjectSet::fileAdded(ProjectFileItem* file)
{
    Q_D(WatchedDocumentSet);

    d->addDocument(IndexedString(file->indexedPath()), DoUpdate | DoEmit);
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
    d->addDocument(IndexedString(newFile->indexedPath()), DoUpdate | DoEmit);
}

void ProjectSet::trackProjectFiles(const IProject* project)
{
    if (project) {
        // The implementation should derive from QObject somehow
        auto* fileManager = dynamic_cast<QObject*>(project->projectFileManager());
        if (fileManager) {
            // can't use new signal/slot syntax here, IProjectFileManager is no a QObject
            connect(fileManager, SIGNAL(fileAdded(ProjectFileItem*)),
                    this, SLOT(fileAdded(ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRemoved(ProjectFileItem*)),
                    this, SLOT(fileRemoved(ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRenamed(Path,ProjectFileItem*)),
                    this, SLOT(fileRenamed(Path,ProjectFileItem*)));
        }
    }
}

CurrentProjectSet::CurrentProjectSet(const IndexedString& document, QObject* parent)
    : ProjectSet(parent)
    , m_currentProject(nullptr)
{
    setCurrentDocumentInternal(document);
}

void CurrentProjectSet::setCurrentDocument(const IndexedString& url)
{
    setCurrentDocumentInternal(url);
}

void CurrentProjectSet::setCurrentDocumentInternal(const IndexedString& url)
{
    Q_D(WatchedDocumentSet);

    IProject* projectForUrl = ICore::self()->projectController()->findProjectForUrl(url.toUrl());
    if (projectForUrl && projectForUrl != m_currentProject) {
        m_currentProject = projectForUrl;
        d->setDocuments(m_currentProject->fileSet());
        d->addDocument(IndexedString(m_currentProject->path().toLocalFile()), DoUpdate | DoEmit);
        trackProjectFiles(m_currentProject);
    }
}

ProblemScope CurrentProjectSet::scope() const
{
    return CurrentProject;
}

AllProjectSet::AllProjectSet(QObject* parent)
    : ProjectSet(parent)
{
    Q_D(WatchedDocumentSet);

    const auto projects = ICore::self()->projectController()->projects();
    for (const IProject* project : projects) {
        const auto fileSet = project->fileSet();
        for (const IndexedString& indexedString : fileSet) {
            d->addDocument(indexedString);
        }
        d->addDocument(IndexedString(project->path().toLocalFile()));
        trackProjectFiles(project);
    }
    d->updateImports();
    emit changed();
}

ProblemScope AllProjectSet::scope() const
{
    return AllProjects;
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
