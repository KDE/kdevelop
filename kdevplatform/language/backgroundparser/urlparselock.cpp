/*
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "urlparselock.h"

#include <QHash>
#include <QMutexLocker>
#include <QRecursiveMutex>

using namespace KDevelop;

namespace {
struct PerUrlData
{
    // TODO: make this non-recursive
    QRecursiveMutex mutex;
    // how many people are (trying to) parse this url
    // we use this to delete the entry once no-one needs it anymore
    uint ref = 0;
};

// this mutex protects the parsingUrls
// NOTE: QBasicMutex is safe to initialize statically
QBasicMutex parsingUrlsMutex;

// Hash of urls that are currently being parsed and their protection data
using ParsingUrls = QHash<IndexedString, PerUrlData*>;
ParsingUrls& parsingUrls()
{
    // delay initialization of the hash until it's needed
    static ParsingUrls parsingUrls;
    return parsingUrls;
}
}

UrlParseLock::UrlParseLock(const IndexedString& url)
    : m_url(url)
{
    QMutexLocker lock(&parsingUrlsMutex);

    // NOTE: operator[] default-initializes the ptr to zero for us when not available
    auto& perUrlData = parsingUrls()[url];
    if (!perUrlData) {
        // if that was the case, we are the first to parse this url, create an entry
        perUrlData = new PerUrlData;
    }

    // always increment the refcount
    ++perUrlData->ref;

    // now lock the url, but don't do so while blocking the global mutex
    auto& mutex = perUrlData->mutex;
    lock.unlock();

    mutex.lock();
}

UrlParseLock::~UrlParseLock()
{
    QMutexLocker lock(&parsingUrlsMutex);

    // find the entry for this url
    auto& urls = parsingUrls();
    auto it = urls.find(m_url);
    Q_ASSERT(it != urls.end()); // it must exist
    auto& perUrlData = it.value();

    // unlock the per-url mutex
    perUrlData->mutex.unlock();

    // decrement the refcount
    --perUrlData->ref;
    if (perUrlData->ref == 0) {
        // and cleanup, if possible
        delete perUrlData;
        urls.erase(it);
    }
}
