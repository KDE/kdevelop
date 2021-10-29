/*
    SPDX-FileCopyrightText: 2014 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangpch.h"

#include <language/duchain/duchain.h>

#include "clanghelpers.h"
#include "util/clangtypes.h"
#include "clangparsingenvironment.h"

using namespace KDevelop;

namespace {

//Map a file from one translation unit to another
inline CXFile mapFile(CXFile file, CXTranslationUnit tu)
{
    return clang_getFile(tu, ClangString(clang_getFileName(file)).c_str());
}

}

ClangPCH::ClangPCH(const ClangParsingEnvironment& environment, ClangIndex* index)
    : m_session({})
{
    const auto& pchInclude = environment.pchInclude();
    Q_ASSERT(pchInclude.isValid());

    const TopDUContext::Features pchFeatures = TopDUContext::AllDeclarationsContextsUsesAndAST;
    const IndexedString doc(pchInclude.pathOrUrl());

    ClangParsingEnvironment pchEnv;
    pchEnv.setPchInclude(Path());
    pchEnv.setTranslationUnitUrl(doc);
    m_session.setData(ParseSessionData::Ptr(new ParseSessionData({}, index, pchEnv, ParseSessionData::PrecompiledHeader)));

    if (!m_session.unit()) {
        return;
    }

    auto imports = ClangHelpers::tuImports(m_session.unit());
    m_context = ClangHelpers::buildDUChain(m_session.mainFile(), imports, m_session, pchFeatures, m_includes, {}, {});
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
    return ::mapFile(m_session.mainFile(), tu);
}

ReferencedTopDUContext ClangPCH::context() const
{
    return m_context;
}
