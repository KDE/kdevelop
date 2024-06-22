/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemstore.h"

#include <language/editor/documentrange.h>
#include <shell/problem.h>
#include <shell/watcheddocumentset.h>
#include "problemstorenode.h"

namespace KDevelop
{

class ProblemStorePrivate
{
public:
    ProblemStorePrivate()
        : m_severities(KDevelop::IProblem::Error | KDevelop::IProblem::Warning | KDevelop::IProblem::Hint)
        , m_rootNode(new KDevelop::ProblemStoreNode())
    {
    }

    /// Watched document set. Only problems that are in files in this set are stored.
    KDevelop::WatchedDocumentSet *m_documents = nullptr;

    /// The severity filter setting
    KDevelop::IProblem::Severities m_severities;

    /// The problems list
    KDevelop::ProblemStoreNode *m_rootNode;

    /// Path of the currently open document
    KDevelop::IndexedString m_currentDocument;

    /// Path for the DocumentsInPath scope
    QString m_pathForDocumentsInPathScope;

    /// All stored problems
    QVector<KDevelop::IProblem::Ptr> m_allProblems;
};


ProblemStore::ProblemStore(QObject *parent)
    : QObject(parent),
      d_ptr(new ProblemStorePrivate)
{
    setScope(BypassScopeFilter);
}

ProblemStore::~ProblemStore()
{
    Q_D(ProblemStore);

    clear();

    delete d->m_rootNode;
}

void ProblemStore::addProblem(const IProblem::Ptr &problem)
{
    Q_D(ProblemStore);

    auto* node = new ProblemNode(d->m_rootNode);
    node->setProblem(problem);
    d->m_rootNode->addChild(node);

    d->m_allProblems += problem;
    emit problemsChanged();
}

void ProblemStore::setProblems(const QVector<IProblem::Ptr> &problems)
{
    Q_D(ProblemStore);

    int oldSize = d->m_allProblems.size();

    // set signals block to prevent problemsChanged() emitting during clean
    {
        QSignalBlocker blocker(this);
        clear();
    }

    for (const IProblem::Ptr& problem : problems) {
        d->m_rootNode->addChild(new ProblemNode(d->m_rootNode, problem));
    }

    rebuild();

    if (d->m_allProblems.size() != oldSize || d->m_allProblems != problems) {
        d->m_allProblems = problems;
        emit problemsChanged();
    }
}

QVector<IProblem::Ptr> ProblemStore::problems(const KDevelop::IndexedString& document) const
{
    Q_D(const ProblemStore);

    QVector<IProblem::Ptr> documentProblems;

    for (auto& problem : std::as_const(d->m_allProblems)) {
        if (problem->finalLocation().document == document)
            documentProblems += problem;
    }

    return documentProblems;
}

const ProblemStoreNode* ProblemStore::findNode(int row, ProblemStoreNode *parent) const
{
    Q_D(const ProblemStore);

    Q_UNUSED(parent);
    return d->m_rootNode->child(row);
}

int ProblemStore::count(ProblemStoreNode *parent) const
{
    Q_D(const ProblemStore);

    if(parent)
        return parent->count();
    else
        return d->m_rootNode->count();
}

void ProblemStore::clear()
{
    Q_D(ProblemStore);

    d->m_rootNode->clear();

    if (!d->m_allProblems.isEmpty()) {
        d->m_allProblems.clear();
        emit problemsChanged();
    }
}

void ProblemStore::rebuild()
{
}

void ProblemStore::setSeverity(int severity)
{
    switch (severity)
    {
        case KDevelop::IProblem::Error:
            setSeverities(KDevelop::IProblem::Error);
            break;
        case KDevelop::IProblem::Warning:
            setSeverities(KDevelop::IProblem::Error | KDevelop::IProblem::Warning);
            break;
        case KDevelop::IProblem::Hint:
            setSeverities(KDevelop::IProblem::Error | KDevelop::IProblem::Warning | KDevelop::IProblem::Hint);
            break;
    }
}

void ProblemStore::setSeverities(KDevelop::IProblem::Severities severities)
{
    Q_D(ProblemStore);

    if(severities != d->m_severities)
    {
        d->m_severities = severities;
        rebuild();
        emit changed();
    }
}

int ProblemStore::severity() const
{
    Q_D(const ProblemStore);

    if (d->m_severities.testFlag(KDevelop::IProblem::Hint))
        return KDevelop::IProblem::Hint;
    if (d->m_severities.testFlag(KDevelop::IProblem::Warning))
        return KDevelop::IProblem::Warning;
    if (d->m_severities.testFlag(KDevelop::IProblem::Error))
        return KDevelop::IProblem::Error;
    return 0;
}

KDevelop::IProblem::Severities ProblemStore::severities() const
{
    Q_D(const ProblemStore);

    return d->m_severities;
}

WatchedDocumentSet* ProblemStore::documents() const
{
    Q_D(const ProblemStore);

    return d->m_documents;
}

void ProblemStore::setScope(ProblemScope scope)
{
    Q_D(ProblemStore);

    bool showImports = false;

    if (d->m_documents) {
        if (scope == d->m_documents->scope())
            return;

        showImports = d->m_documents->showImports();
        delete d->m_documents;
    }

    switch (scope) {
    case CurrentDocument:
        d->m_documents = new CurrentDocumentSet(d->m_currentDocument, this);
        break;
    case OpenDocuments:
        d->m_documents = new OpenDocumentSet(this);
        break;
    case CurrentProject:
        d->m_documents = new CurrentProjectSet(d->m_currentDocument, this);
        break;
    case AllProjects:
        d->m_documents = new AllProjectSet(this);
        break;
    case DocumentsInPath:
        d->m_documents = new DocumentsInPathSet(d->m_pathForDocumentsInPathScope, this);
        break;
    case DocumentsInCurrentPath:
        d->m_documents = new DocumentsInCurrentPathSet(d->m_currentDocument, this);
        break;
    case BypassScopeFilter:
        d->m_documents = new BypassSet(this);
        break;
    }

    d->m_documents->setShowImports(showImports);

    rebuild();

    connect(d->m_documents, &WatchedDocumentSet::changed, this, &ProblemStore::onDocumentSetChanged);

    emit changed();
}

ProblemScope ProblemStore::scope() const
{
    Q_D(const ProblemStore);

    Q_ASSERT(d->m_documents);

    return d->m_documents->scope();
}

void ProblemStore::setGrouping(int grouping)
{
    Q_UNUSED(grouping);
}

void ProblemStore::setShowImports(bool showImports)
{
    Q_D(ProblemStore);

    d->m_documents->setShowImports(showImports);
}

int ProblemStore::showImports() const
{
    Q_D(const ProblemStore);

    return d->m_documents->showImports();
}

void ProblemStore::setCurrentDocument(const IndexedString &doc)
{
    Q_D(ProblemStore);

    d->m_currentDocument = doc;
    d->m_documents->setCurrentDocument(doc);
}


const KDevelop::IndexedString& ProblemStore::currentDocument() const
{
    Q_D(const ProblemStore);

    return d->m_currentDocument;
}

void ProblemStore::setPathForDocumentsInPathScope(const QString& path)
{
    Q_D(ProblemStore);

    d->m_pathForDocumentsInPathScope = path;

    if (d->m_documents->scope() == DocumentsInPath) {
        static_cast<DocumentsInPathSet*>(d->m_documents)->setPath(path);
    }
}

QString ProblemStore::pathForDocumentsInPathScope() const
{
    Q_D(const ProblemStore);

    return d->m_pathForDocumentsInPathScope;
}

void ProblemStore::onDocumentSetChanged()
{
    rebuild();

    emit changed();
}

ProblemStoreNode* ProblemStore::rootNode() const
{
    Q_D(const ProblemStore);

    return d->m_rootNode;
}

}

#include "moc_problemstore.cpp"
