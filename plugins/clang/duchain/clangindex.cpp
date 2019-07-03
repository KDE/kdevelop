/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#include "clangindex.h"

#include "clangpch.h"
#include "clangparsingenvironment.h"
#include "documentfinderhelpers.h"

#include <util/path.h>
#include <util/clangtypes.h>
#include <util/clangdebug.h>
#include <language/backgroundparser/urlparselock.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

#include <clang-c/Index.h>

using namespace KDevelop;

ClangIndex::ClangIndex()
    // NOTE: We don't exclude PCH declarations. That way we could retrieve imports manually, as clang_getInclusions returns nothing on reparse with CXTranslationUnit_PrecompiledPreamble flag.
    : m_index(clang_createIndex(0 /*Exclude PCH Decls*/, qEnvironmentVariableIsSet("KDEV_CLANG_DISPLAY_DIAGS") /*Display diags*/))
{
    // demote the priority of the clang parse threads to reduce potential UI lockups
    // but the code completion threads still retain their normal priority to return
    // the results as quickly as possible
    clang_CXIndex_setGlobalOptions(m_index, clang_CXIndex_getGlobalOptions(m_index)
        | CXGlobalOpt_ThreadBackgroundPriorityForIndexing);
}

CXIndex ClangIndex::index() const
{
    return m_index;
}

QSharedPointer<const ClangPCH> ClangIndex::pch(const ClangParsingEnvironment& environment)
{
    const auto& pchInclude = environment.pchInclude();
    if (!pchInclude.isValid()) {
        return {};
    }

    UrlParseLock pchLock(IndexedString(pchInclude.pathOrUrl()));

    static const QString pchExt = QStringLiteral(".pch");

    if (QFile::exists(pchInclude.toLocalFile() + pchExt)) {
        QReadLocker lock(&m_pchLock);
        auto pch = m_pch.constFind(pchInclude);
        if (pch != m_pch.constEnd()) {
            return pch.value();
        }
    }

    auto pch = QSharedPointer<ClangPCH>::create(environment, this);
    QWriteLocker lock(&m_pchLock);
    m_pch.insert(pchInclude, pch);
    return pch;
}

ClangIndex::~ClangIndex()
{
    clang_disposeIndex(m_index);
}

IndexedString ClangIndex::translationUnitForUrl(const IndexedString& url)
{
    { // try explicit pin data first
        QMutexLocker lock(&m_mappingMutex);
        auto tu = m_tuForUrl.find(url);
        if (tu != m_tuForUrl.end()) {
            if (!QFile::exists(tu.value().str())) {
                // TU doesn't exist, unpin
                m_tuForUrl.erase(tu);
                return url;
            }
            return tu.value();
        }
    }
    // if no explicit pin data is available, follow back the duchain import chain
    {
        DUChainReadLocker lock;
        TopDUContext* top = DUChain::self()->chainForDocument(url);
        if (top) {
            TopDUContext* tuTop = top;
            QSet<TopDUContext*> visited;
            while(true) {
                visited.insert(tuTop);
                TopDUContext* next = nullptr;
                const auto importers = tuTop->indexedImporters();
                for (IndexedDUContext ctx : importers) {
                    if (ctx.data()) {
                        next = ctx.data()->topContext();
                        break;
                    }
                }
                if (!next || visited.contains(next)) {
                    break;
                }
                tuTop = next;
            }
            if (tuTop != top) {
                return tuTop->url();
            }
        }
    }

    // otherwise, fallback to a simple buddy search for headers
    if (ClangHelpers::isHeader(url.str())) {
        const auto buddies = DocumentFinderHelpers::potentialBuddies(url.toUrl(), false);
        for (const QUrl& buddy : buddies) {
            const QString buddyPath = buddy.toLocalFile();
            if (QFile::exists(buddyPath)) {
                return IndexedString(buddyPath);
            }
        }
    }
    return url;
}

void ClangIndex::pinTranslationUnitForUrl(const IndexedString& tu, const IndexedString& url)
{
    QMutexLocker lock(&m_mappingMutex);
    m_tuForUrl.insert(url, tu);
}

void ClangIndex::unpinTranslationUnitForUrl(const IndexedString& url)
{
    QMutexLocker lock(&m_mappingMutex);
    m_tuForUrl.remove(url);
}
