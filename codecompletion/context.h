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

#include "duchain/parsesession.h"

#include <language/codecompletion/codecompletioncontext.h>

#include <clang-c/Index.h>

#include <memory>

#include "completionhelper.h"

class ClangCodeCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    enum ContextFilter
    {
        NoFilter        = 0,        ///< Show everything
        NoBuiltins      = 1 << 0,   ///< Hide builtin completion items
        NoMacros        = 1 << 1,   ///< Hide macro completion items
        NoDeclarations  = 1 << 2,   ///< Hide declaration completion items
        NoClangCompletion = NoBuiltins | NoMacros | NoDeclarations
    };
    Q_DECLARE_FLAGS(ContextFilters, ContextFilter)

    ClangCodeCompletionContext(const KDevelop::DUContextPointer& context,
                               const ParseSessionData::Ptr& sessionData,
                               const QUrl& url,
                               const KTextEditor::Cursor& position,
                               const QString& text);
    ~ClangCodeCompletionContext();

    virtual QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) override;

    QList<KDevelop::CompletionTreeElementPointer> ungroupedElements() override;

    ContextFilters filters() const;
    void setFilters(const ContextFilters& filters);

private:
    void addOverwritableItems();
    void addImplementationHelperItems();

    /// Creates the group named @p name and adds it to m_ungrouped if items @p items is not empty
    void eventuallyAddGroup(const QString& name, int priority, const QList<KDevelop::CompletionTreeItemPointer>& items);

    /// Returns whether the we are at a valid completion-position
    bool isValidPosition(CXTranslationUnit unit, CXFile file) const;

    std::unique_ptr<CXCodeCompleteResults, void(*)(CXCodeCompleteResults*)> m_results;
    QList<KDevelop::CompletionTreeElementPointer> m_ungrouped;
    CompletionHelper m_completionHelper;
    ParseSessionData::Ptr m_parseSessionData;
    ContextFilters m_filters = NoFilter;
};

#endif // CLANGCODECOMPLETIONCONTEXT_H
