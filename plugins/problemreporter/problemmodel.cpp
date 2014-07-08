/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "problemmodel.h"

#include <QTimer>
#include <klocale.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/topducontext.h>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "problemreporterplugin.h"
#include "watcheddocumentset.h"

using namespace KDevelop;

ProblemModel::ProblemModel(ProblemReporterPlugin * parent)
  : QAbstractItemModel(parent), m_plugin(parent), m_lock(QReadWriteLock::Recursive), m_showImports(false), m_severity(ProblemData::Hint), m_documentSet(0)
{
    m_minTimer = new QTimer(this);
    m_minTimer->setInterval(MinTimeout);
    m_minTimer->setSingleShot(true);
    connect(m_minTimer, SIGNAL(timeout()), SLOT(timerExpired()));
    m_maxTimer = new QTimer(this);
    m_maxTimer->setInterval(MaxTimeout);
    m_maxTimer->setSingleShot(true);
    connect(m_maxTimer, SIGNAL(timeout()), SLOT(timerExpired()));
    setScope(CurrentDocument);
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(setCurrentDocument(KDevelop::IDocument*)));
    // CompletionSettings include a list of todo markers we care for, so need to update
    connect(ICore::self()->languageController()->completionSettings(), SIGNAL(settingsChanged(ICompletionSettings*)), SLOT(forceFullUpdate()));

    if (ICore::self()->documentController()->activeDocument()) {
        setCurrentDocument(ICore::self()->documentController()->activeDocument());
    }
}

const int ProblemModel::MinTimeout = 1000;
const int ProblemModel::MaxTimeout = 5000;

ProblemModel::~ ProblemModel()
{
  m_problems.clear();
}

int ProblemModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_problems.count();

    return 0;
}

QString getDisplayUrl(const QString &url, const KUrl &base) {
    KUrl location(url);
    QString displayedUrl;
    if ( location.protocol() == base.protocol() &&
            location.user() == base.user() &&
            location.host() == base.host() ) {
        bool isParent;
        displayedUrl = KUrl::relativePath(base.path(), location.path(), &isParent );
        if ( !isParent ) {
            displayedUrl = location.pathOrUrl();
        }
    } else {
        displayedUrl = location.pathOrUrl();
    }
    return displayedUrl;
}

QVariant ProblemModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DUChainReadLocker lock;

    ProblemPointer p = problemForIndex(index);
    KUrl baseDirectory = m_currentDocument.upUrl();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case Source:
            return p->sourceString();
        case Error:
            return p->description();
        case File:
            return getDisplayUrl(p->finalLocation().document.str(), baseDirectory);
        case Line:
            if (p->finalLocation().isValid())
                return QString::number(p->finalLocation().start.line + 1);
            break;
        case Column:
            if (p->finalLocation().isValid())
                return QString::number(p->finalLocation().start.column + 1);
            break;
        }
        break;

    case Qt::ToolTipRole:
        return p->explanation();

    default:
        break;
    }

    return QVariant();
}

QModelIndex ProblemModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

QModelIndex ProblemModel::index(int row, int column, const QModelIndex & parent) const
{
    DUChainReadLocker lock(DUChain::lock());

    if (row < 0 || column < 0 || column >= LastColumn)
        return QModelIndex();

    if (parent.isValid()) {
        return QModelIndex();
    }

    if (row < m_problems.count())
        return createIndex(row, column);

    return QModelIndex();
}

int ProblemModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return LastColumn;
}

KDevelop::ProblemPointer ProblemModel::problemForIndex(const QModelIndex & index) const
{
    return m_problems.at(index.row());
}

ProblemReporterPlugin* ProblemModel::plugin()
{
    return m_plugin;
}

QVariant ProblemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case Source:
            return i18nc("@title:column source of problem", "Source");
        case Error:
            return i18nc("@title:column problem description", "Problem");
        case File:
            return i18nc("@title:column file where problem was found", "File");
        case Line:
            return i18nc("@title:column line number with problem", "Line");
        case Column:
            return i18nc("@title:column column number with problem", "Column");
    }

    return QVariant();
}

void ProblemModel::problemsUpdated(const KDevelop::IndexedString& url)
{
    QReadLocker locker(&m_lock);
    if (m_documentSet->get().contains(url)) {
        // m_minTimer will expire in MinTimeout unless some other parsing job finishes in this period.
        m_minTimer->start();
        // m_maxTimer will expire unconditionally in MaxTimeout
        if (!m_maxTimer->isActive()) {
            m_maxTimer->start();
        }
    }
}

void ProblemModel::timerExpired()
{
    m_minTimer->stop();
    m_maxTimer->stop();
    rebuildProblemList();
}

QList<ProblemPointer> ProblemModel::getProblems(const IndexedString& url, bool showImports)
{
    QList<ProblemPointer> result;
    QSet<TopDUContext*> visitedContexts;
    DUChainReadLocker lock;
    getProblemsInternal(DUChain::self()->chainForDocument(url), showImports, visitedContexts, result);
    return result;
}

QList< ProblemPointer > ProblemModel::getProblems(QSet< IndexedString > urls, bool showImports)
{
    QList<ProblemPointer> result;
    QSet<TopDUContext*> visitedContexts;
    DUChainReadLocker lock;
    foreach(const IndexedString& url, urls) {
        getProblemsInternal(DUChain::self()->chainForDocument(url), showImports, visitedContexts, result);
    }
    return result;
}

void ProblemModel::getProblemsInternal(TopDUContext* context, bool showImports, QSet<TopDUContext*>& visitedContexts, QList<KDevelop::ProblemPointer>& result)
{
    if (!context || visitedContexts.contains(context)) {
        return;
    }
    foreach(ProblemPointer p, context->problems()) {
        if (p && p->severity() <= m_severity) {
            result.append(p);
        }
    }
    visitedContexts.insert(context);
    if (showImports) {
        bool isProxy = context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->isProxyContext();
        foreach(const DUContext::Import &ctx, context->importedParentContexts()) {
            if(!ctx.indexedContext().indexedTopContext().isLoaded())
                continue;
            TopDUContext* topCtx = dynamic_cast<TopDUContext*>(ctx.context(0));
            if(topCtx) {
                //If we are starting at a proxy-context, only recurse into other proxy-contexts,
                //because those contain the problems.
                if(!isProxy || (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext()))
                    getProblemsInternal(topCtx, showImports, visitedContexts, result);
            }
        }
    }
}

void ProblemModel::rebuildProblemList()
{
    // No locking here, because it may be called from an already locked context
    beginResetModel();
    m_problems = getProblems(m_documentSet->get(), m_showImports);
    endResetModel();
}

void ProblemModel::setCurrentDocument(KDevelop::IDocument* document)
{
    QWriteLocker locker(&m_lock);
    beginResetModel();
    m_currentDocument = document->url();
    m_documentSet->setCurrentDocument(IndexedString(m_currentDocument));
    endResetModel();
}

void ProblemModel::setShowImports(bool showImports)
{
    if (m_showImports != showImports) {
        QWriteLocker locker(&m_lock);
        m_showImports = showImports;
        rebuildProblemList();
    }
}

void ProblemModel::setScope(int scope)
{
    Scope cast_scope = static_cast<Scope>(scope);
    QWriteLocker locker(&m_lock);
    if (!m_documentSet || m_documentSet->getScope() != cast_scope) {
        if (m_documentSet) {
            delete m_documentSet;
        }
        switch (cast_scope) {
        case CurrentDocument:
            m_documentSet = new CurrentDocumentSet(IndexedString(m_currentDocument), this);
            break;
        case OpenDocuments:
            m_documentSet = new OpenDocumentSet(this);
            break;
        case CurrentProject:
            m_documentSet = new CurrentProjectSet(IndexedString(m_currentDocument), this);
            break;
        case AllProjects:
            m_documentSet = new AllProjectSet(this);
            break;
        }
        connect(m_documentSet, SIGNAL(changed()), this, SLOT(documentSetChanged()));
        rebuildProblemList();
    }
}

void ProblemModel::setSeverity(int severity)
{
    ProblemData::Severity cast_severity = static_cast<ProblemData::Severity>(severity);
    if (m_severity != cast_severity) {
        QWriteLocker locker(&m_lock);
        m_severity = cast_severity;
        rebuildProblemList();
    }
}

void ProblemModel::documentSetChanged()
{
    rebuildProblemList();
}

void ProblemModel::forceFullUpdate()
{
    m_lock.lockForRead();
    QSet<IndexedString> documents = m_documentSet->get();
    m_lock.unlock();
    DUChainReadLocker lock(DUChain::lock());
    foreach(const IndexedString& document, documents) {
        if (document.isEmpty())
            continue;

        TopDUContext::Features updateType = TopDUContext::ForceUpdate;
        if(documents.size() == 1)
            updateType = TopDUContext::ForceUpdateRecursive;
        DUChain::self()->updateContextForUrl(document, (TopDUContext::Features)(updateType | TopDUContext::VisibleDeclarationsAndContexts));
    }
}
