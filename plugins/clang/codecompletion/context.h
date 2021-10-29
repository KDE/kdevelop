/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGCODECOMPLETIONCONTEXT_H
#define CLANGCODECOMPLETIONCONTEXT_H

#include "duchain/parsesession.h"

#include <language/codecompletion/codecompletioncontext.h>

#include <clang-c/Index.h>

#include <memory>

#include "completionhelper.h"
#include "clangprivateexport.h"

class KDEVCLANGPRIVATE_EXPORT ClangCodeCompletionContext : public KDevelop::CodeCompletionContext
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
                               const QString& text,
                               const QString& followingText = {});
    ~ClangCodeCompletionContext() override;

    QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) override;

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
