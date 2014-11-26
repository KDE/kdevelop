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

#include <util/path.h>
#include <language/backgroundparser/urlparselock.h>

#include <clang-c/Index.h>

using namespace KDevelop;

ClangIndex::ClangIndex()
    // NOTE: We don't exclude PCH declarations. That way we could retrieve imports manually, as clang_getInclusions returns nothing on reparse with CXTranslationUnit_PrecompiledPreamble flag.
    : m_index(clang_createIndex(0 /*Exclude PCH Decls*/, qgetenv("KDEV_CLANG_DISPLAY_DIAGS") == "1" /*Display diags*/))
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

    static const QString pchExt = QStringLiteral(".pch");

    if (QFile::exists(pchInclude.toLocalFile() + pchExt)) {
        QReadLocker lock(&m_pchLock);
        auto pch = m_pch.constFind(pchInclude);
        if (pch != m_pch.constEnd()) {
            return pch.value();
        }
    }

    QSharedPointer<const ClangPCH> pch(new ClangPCH(environment, this));
    QWriteLocker lock(&m_pchLock);
    m_pch.insert(pchInclude, pch);
    return pch;
}

ClangIndex::~ClangIndex()
{
    clang_disposeIndex(m_index);
}
