/*
 * This file is part of KDevelop
 *
 * Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2016 Milian Wolff <mail@milianw.de>
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
#include "urlparselock.h"

#include <QHash>
#include <QMutex>

using namespace KDevelop;

namespace {
struct PerUrlData
{
    PerUrlData()
    // TODO: make this non-recursive
        : mutex(QMutex::Recursive)
    {}
    QMutex mutex;
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
