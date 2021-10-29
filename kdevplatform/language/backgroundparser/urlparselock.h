/*
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_URLPARSELOCK_H
#define KDEVPLATFORM_URLPARSELOCK_H

#include <language/languageexport.h>

#include <serialization/indexedstring.h>

namespace KDevelop {
/**
 * This is used to prevent the background parser from updating the duchain for a specific file.
 * It can be used to prevent changes while working on the duchain.
 *
 * Every language-specific parse-job has to lock this before updating a TopDUContext.
 *
 * @warning No other mutex must be locked when this lock is acquired, to prevent deadlocks
 */
class KDEVPLATFORMLANGUAGE_EXPORT UrlParseLock
{
public:
    explicit UrlParseLock(const IndexedString& url);
    ~UrlParseLock();

private:
    Q_DISABLE_COPY(UrlParseLock)

    IndexedString m_url;
};
}

#endif
