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

#include "problemreporterplugin.h"
#include "watcheddocumentset.h"

using namespace KDevelop;

ProblemModel::ProblemModel(ProblemReporterPlugin * parent)
  : QAbstractItemModel(parent), m_plugin(parent), m_showImports(false), m_documentSet(0)
{
    setScope(CurrentDocument);
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(setCurrentDocument(KDevelop::IDocument*)));
}

ProblemModel::~ ProblemModel()
{
  m_problems.clear();
}

int ProblemModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_problems.count();

    if (parent.internalId() && parent.column() == 0)
        return m_problems.at(parent.row())->locationStack().count();

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

//     Locking the duchain here leads to a deadlock, because kate triggers some paint to the outside while holding the smart-lock
//     DUChainReadLocker lock(DUChain::lock());

    ProblemPointer p = problemForIndex(index);
    KUrl baseDirectory = m_currentDocument.upUrl();

    if (!index.internalId()) {
        // Top level
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case Source:
                        return p->sourceString();
                        break;
                    case Error:
                        return p->description();
                    case File: {
                        return getDisplayUrl(p->finalLocation().document.str(), baseDirectory);
                    }
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

    } else {
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case Error:
                        return i18n("In file included from:");
                    case File: {
                        return getDisplayUrl(p->locationStack().at(index.row()).document.str(), baseDirectory);
                    } case Line:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start.line + 1);
                        break;
                    case Column:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start.column + 1);
                        break;
                }
                break;

            default:
                break;
        }
    }

    return QVariant();
}

QModelIndex ProblemModel::parent(const QModelIndex & index) const
{
    if (index.internalId())
        return createIndex(m_problems.indexOf(problemForIndex(index)), 0, 0);

    return QModelIndex();
}

QModelIndex ProblemModel::index(int row, int column, const QModelIndex & parent) const
{
    DUChainReadLocker lock(DUChain::lock());

    if (row < 0 || column < 0 || column >= LastColumn)
        return QModelIndex();

    if (parent.isValid()) {
        if (parent.internalId())
            return QModelIndex();

        if (parent.column() != 0)
            return QModelIndex();

        ProblemPointer problem = problemForIndex(parent);
        if (row >= problem->locationStack().count())
            return QModelIndex();
        ///@todo Make location-stack work again

        return createIndex(row, column, row);
    }

    if (row < m_problems.count())
        return createIndex(row, column, 0);

    return QModelIndex();
}

int ProblemModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return LastColumn;
}

KDevelop::ProblemPointer ProblemModel::problemForIndex(const QModelIndex & index) const
{
    if (index.internalId())
        return m_problems.at(index.internalId());
    else
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
            return i18n("Source");
        case Error:
            return i18n("Problem");
        case File:
            return i18n("File");
        case Line:
            return i18n("Line");
        case Column:
            return i18n("Column");
    }

    return QVariant();
}

void ProblemModel::updateProblems(const KDevelop::IndexedString& url, TopDUContext* context)
{
    if (!context) {
        return;
    }
    DUChainReadLocker chainLock;
    m_lock.lockForWrite();
    m_topProblems.remove(url);
    updateProblemsInternal(context, url);
    m_lock.unlock();
    chainLock.unlock();
    if (m_documentSet->get().contains(url)) {
        rebuildProblemList();
    }
}

void ProblemModel::updateProblemsInternal(KDevelop::TopDUContext* context, const KDevelop::IndexedString& parentUrl)
{
    KDevelop::IndexedString currentUrl = context->url();
    if (parentUrl == currentUrl) {
        m_topProblems[currentUrl].append(context->problems());
    } else {
        m_imports[parentUrl].insert(currentUrl);
        if (m_topProblems.contains(currentUrl)) {
            return;
        }
        m_topProblems[currentUrl] = context->problems();
    }
    bool isProxy = context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->isProxyContext();
    foreach(const DUContext::Import &ctx, context->importedParentContexts()) {
        if(!ctx.indexedContext().indexedTopContext().isLoaded())
            continue;
        TopDUContext* topCtx = dynamic_cast<TopDUContext*>(ctx.context(0));
        if(topCtx) {
            //If we are starting at a proxy-context, only recurse into other proxy-contexts,
            //because those contain the problems.
            if(!isProxy || (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext()))
                updateProblemsInternal(topCtx, currentUrl);
        }
    }
}

QList<ProblemPointer> ProblemModel::getProblems(IndexedString url, bool showImports)
{
    QList<ProblemPointer> result;
    QSet<IndexedString> visitedUrls;
    m_lock.lockForRead();
    getProblemsInternal(url, showImports, visitedUrls, result);
    m_lock.unlock();
    return result;
}

QList< ProblemPointer > ProblemModel::getProblems(QSet< IndexedString > urls, bool showImports)
{
    QList<ProblemPointer> result;
    QSet<IndexedString> visitedUrls;
    m_lock.lockForRead();
    foreach(const IndexedString& url, urls) {
        getProblemsInternal(url, showImports, visitedUrls, result);
    }
    m_lock.unlock();
    return result;
}

void ProblemModel::getProblemsInternal(IndexedString url, bool showImports, QSet< IndexedString >& visitedUrls, QList< ProblemPointer >& result)
{
    if (visitedUrls.contains(url)) {
        return;
    }
    result.append(m_topProblems[url]);
    visitedUrls.insert(url);
    if (showImports) {
        foreach(const IndexedString& importUrl, m_imports[url]) {
            getProblemsInternal(importUrl, showImports, visitedUrls, result);
        }
    }
}

void ProblemModel::rebuildProblemList()
{
    m_problems = getProblems(m_documentSet->get(), m_showImports);
    reset();
}

void ProblemModel::setCurrentDocument(KDevelop::IDocument* document)
{
    m_currentDocument = document->url();
    m_documentSet->setCurrentDocument(IndexedString(m_currentDocument));
    reset();
}

void ProblemModel::setShowImports(bool showImports)
{
    if (m_showImports != showImports) {
        m_showImports = showImports;
        rebuildProblemList();
    }
}

void ProblemModel::setScope(int scope)
{
    Scope cast_scope = static_cast<Scope>(scope);
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

void ProblemModel::documentSetChanged()
{
    rebuildProblemList();
}

void ProblemModel::forceFullUpdate()
{
    // TODO: implement
}
