/*
    SPDX-FileCopyrightText: 2014 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGPCH_H
#define CLANGPCH_H

#include <language/duchain/topducontext.h>
#include <util/path.h>

#include "parsesession.h"
#include "clanghelpers.h"

class ClangParsingEnvironment;

class KDEVCLANGPRIVATE_EXPORT ClangPCH
{
public:
    ClangPCH(const ClangParsingEnvironment& environment, ClangIndex* index);

    IncludeFileContexts mapIncludes(CXTranslationUnit tu) const;

    CXFile mapFile(CXTranslationUnit tu) const;

    KDevelop::ReferencedTopDUContext context() const;

private:
    Q_DISABLE_COPY(ClangPCH)

    IncludeFileContexts m_includes;
    KDevelop::ReferencedTopDUContext m_context;
    ParseSession m_session;
};

#endif //CLANGPCH_H
