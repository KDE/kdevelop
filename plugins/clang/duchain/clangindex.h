/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLANGINDEX_H
#define CLANGINDEX_H

#include "clanghelpers.h"

#include "clangprivateexport.h"
#include <serialization/indexedstring.h>

#include <util/path.h>

#include <QReadWriteLock>
#include <QSharedPointer>

#include <clang-c/Index.h>

class ClangParsingEnvironment;
class ClangPCH;

class KDEVCLANGPRIVATE_EXPORT ClangIndex
{
public:
    ClangIndex();
    ~ClangIndex();

    CXIndex index() const;

    /**
     * @returns the existing ClangPCH for @p environment
     *
     * The PCH is created using @p environment if it doesn't exist
     * This function is thread safe.
     */
    QSharedPointer<const ClangPCH> pch(const ClangParsingEnvironment& environment);

    /**
     * Gets the currently pinned TU for @p url
     *
     * If the currently pinned TU does not import @p url, @p url is returned
     */
    KDevelop::IndexedString translationUnitForUrl(const KDevelop::IndexedString& url);

    /**
     * Pin @p tu as the translation unit to use when parsing @p url
     */
    void pinTranslationUnitForUrl(const KDevelop::IndexedString& tu, const KDevelop::IndexedString& url);

    /**
     * Unpin any translation unit currently pinned for @p url
     */
    void unpinTranslationUnitForUrl(const KDevelop::IndexedString& url);

private:
    CXIndex m_index;

    QReadWriteLock m_pchLock;
    QHash<KDevelop::Path, QSharedPointer<const ClangPCH>> m_pch;

    QMutex m_mappingMutex;
    QHash<KDevelop::IndexedString, KDevelop::IndexedString> m_tuForUrl;
};

#endif //CLANGINDEX_H
