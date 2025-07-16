/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clangindex.h"

#include "clangpch.h"
#include "clangparsingenvironment.h"
#include "documentfinderhelpers.h"

#include <interfaces/icompletionsettings.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include <util/path.h>
#include <util/clangtypes.h>
#include <util/clangdebug.h>
#include <language/backgroundparser/urlparselock.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

#include <clang-c/Index.h>

using namespace KDevelop;

namespace {

CXIndex createIndex()
{
    // NOTE: We don't exclude PCH declarations. That way we could retrieve imports manually,
    // as clang_getInclusions returns nothing on reparse with CXTranslationUnit_PrecompiledPreamble flag.
    const auto excludeDeclarationsFromPCH = false;
    const auto displayDiagnostics = qEnvironmentVariableIsSet("KDEV_CLANG_DISPLAY_DIAGS");

    CXIndex index;
#if CINDEX_VERSION_MINOR >= 64
    const bool storePreamblesInMemory =
        ICore::self()->languageController()->completionSettings()->precompiledPreambleStorage()
        == ICompletionSettings::PrecompiledPreambleStorage::Memory;
    // When KDevelop crashes, libclang leaves preamble-*.pch files in its temporary directory.
    // These files occupy from tens of megabytes to gigabytes and are not automatically removed
    // until system restart. Set the preamble storage path to the active session's temporary
    // directory in order to remove all PCH files for the active session on KDevelop start.
    // See also https://github.com/llvm/llvm-project/issues/51847
    const auto preambleStoragePath = ICore::self()->sessionTemporaryDirectoryPath().toUtf8();

    CXIndexOptions options = {sizeof(CXIndexOptions)};
    // Demote the priority of the clang parse threads to reduce potential UI lockups.
    // The code completion threads still retain their normal priority to return the results as quickly as possible.
    options.ThreadBackgroundPriorityForIndexing = CXChoice_Enabled;
    options.ExcludeDeclarationsFromPCH = excludeDeclarationsFromPCH;
    options.DisplayDiagnostics = displayDiagnostics;
    options.StorePreamblesInMemory = storePreamblesInMemory;
    options.PreambleStoragePath = preambleStoragePath.constData();

    // TODO: remove the following workaround line as well as a complementary workaround
    //       that hides the related configuration UI in languagepreferences.cpp after fixing
    //       the crash reported in https://bugs.kde.org/show_bug.cgi?id=486932 properly.
    options.StorePreamblesInMemory = false;

    index = clang_createIndexWithOptions(&options);
    if (index) {
        return index;
    }
    qCWarning(KDEV_CLANG) << "clang_createIndexWithOptions() failed. CINDEX_VERSION_MINOR =" << CINDEX_VERSION_MINOR
                          << ", sizeof(CXIndexOptions) =" << options.Size;
    // Fall back to using older API. Configuring the preamble storage is not essential.
#endif
    index = clang_createIndex(excludeDeclarationsFromPCH, displayDiagnostics);
    // Demote the priority of the clang parse threads to reduce potential UI lockups.
    // The code completion threads still retain their normal priority to return the results as quickly as possible.
    clang_CXIndex_setGlobalOptions(
        index, clang_CXIndex_getGlobalOptions(index) | CXGlobalOpt_ThreadBackgroundPriorityForIndexing);
    return index;
}

} // unnamed namespace

ClangIndex::ClangIndex()
    : m_index(createIndex())
{
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

    if (QFile::exists(pchInclude.toLocalFile() + QLatin1String(".pch"))) {
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
