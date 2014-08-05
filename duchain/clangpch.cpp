/*
 * Copyright 2014  Olivier de Gaalon <olivier.jg@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clangpch.h"

#include <language/duchain/duchain.h>

#include "clanghelpers.h"
#include "util/clangtypes.h"

using namespace KDevelop;

namespace {

//Map a file from one translation unit to another
inline CXFile mapFile(CXFile file, CXTranslationUnit tu)
{
    return clang_getFile(tu, ClangString(clang_getFileName(file)).c_str());
}

}

ClangPCH::ClangPCH(const Path& pchInclude, const Path::List& includePaths, const QHash<QString, QString>& defines, ClangIndex* index)
    : m_session({})
{
    Q_ASSERT(pchInclude.isValid());

    const TopDUContext::Features pchFeatures = TopDUContext::AllDeclarationsContextsUsesAndAST;
    const IndexedString doc(pchInclude.pathOrUrl());

    m_session.setData(ParseSessionData::Ptr(new ParseSessionData(doc, QByteArray(), index, includePaths, Path(),
                                            defines, ParseSessionData::PrecompiledHeader)));

    if (!m_session.unit()) {
        return;
    }

    auto imports = ClangHelpers::tuImports(m_session.unit());
    m_context = ClangHelpers::buildDUChain(m_session.file(), imports, m_session.data(), pchFeatures, m_includes);
}

IncludeFileContexts ClangPCH::mapIncludes(CXTranslationUnit tu) const
{
    IncludeFileContexts mapped;
    mapped.reserve(m_includes.size());
    for (auto it = m_includes.constBegin(); it != m_includes.constEnd(); ++it) {
        mapped.insert(::mapFile(it.key(), tu), it.value());
    }
    return mapped;
}

CXFile ClangPCH::mapFile(CXTranslationUnit tu) const
{
    return ::mapFile(m_session.file(), tu);
}

ReferencedTopDUContext ClangPCH::context() const
{
    return m_context;
}
