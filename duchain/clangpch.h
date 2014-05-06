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

#ifndef CLANGPCH_H
#define CLANGPCH_H

#include <language/duchain/topducontext.h>
#include <util/path.h>

#include "parsesession.h"
#include "clanghelpers.h"

class KDEVCLANGDUCHAIN_EXPORT ClangPCH
{
public:
    ClangPCH(const KDevelop::Path& pchInclude, const KDevelop::Path::List& includePaths,
             const QHash<QString, QString>& defines, ClangIndex* index);

    IncludeFileContexts mapIncludes(CXTranslationUnit tu) const;

    CXFile mapFile(CXTranslationUnit tu) const;

    KDevelop::ReferencedTopDUContext context() const;

private:
    Q_DISABLE_COPY(ClangPCH);

    IncludeFileContexts m_includes;
    KDevelop::ReferencedTopDUContext m_context;
    KSharedPtr<ParseSession> m_session;
};

#endif //CLANGPCH_H
