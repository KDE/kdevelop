/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
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

#ifndef CLANGCODECOMPLETIONCONTEXT_H
#define CLANGCODECOMPLETIONCONTEXT_H

#include <language/codecompletion/codecompletioncontext.h>

#include <clang-c/Index.h>

#include <memory>

#include "completionhelper.h"

class ParseSession;
class ClangCodeCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    ClangCodeCompletionContext(const ParseSession& session,
                               const KDevelop::SimpleCursor& position,
                               const QStringList& contents);
    ~ClangCodeCompletionContext();

    QList<KDevelop::CompletionTreeItemPointer> completionItems(const KDevelop::TopDUContext* const top,
                                                               const KDevelop::CursorInRevision& position);

    QList< KDevelop::CompletionTreeItemPointer > completionItems(bool& /*abort*/, bool /*fullCompletion*/ = true) override
    {
        // not used, see above
        return {};
    }

    QList<KDevelop::CompletionTreeElementPointer> ungroupedElements() override;
private:
    std::unique_ptr<CXCodeCompleteResults, void(*)(CXCodeCompleteResults*)> m_results;
    QList<KDevelop::CompletionTreeElementPointer> m_ungrouped;
    CompletionHelper m_completionHelper;
};

#endif // CLANGCODECOMPLETIONCONTEXT_H
