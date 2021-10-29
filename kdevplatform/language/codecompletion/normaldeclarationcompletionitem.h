/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_NORMALDECLARATIONCOMPLETIONITEM_H
#define KDEVPLATFORM_NORMALDECLARATIONCOMPLETIONITEM_H

#include "codecompletionitem.h"
#include "codecompletioncontext.h"
#include <language/languageexport.h>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT NormalDeclarationCompletionItem
    : public CompletionTreeItem
{
public:
    explicit NormalDeclarationCompletionItem(
        const KDevelop::DeclarationPointer& decl = KDevelop::DeclarationPointer(),
        const QExplicitlySharedDataPointer<CodeCompletionContext>& context = QExplicitlySharedDataPointer<CodeCompletionContext>(),
        int inheritanceDepth = 0);
    KDevelop::DeclarationPointer declaration() const override;
    QExplicitlySharedDataPointer<CodeCompletionContext> completionContext() const;
    int inheritanceDepth() const override;
    int argumentHintDepth() const override;
    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;

    void execute(KTextEditor::View* document, const KTextEditor::Range& word) override;

protected:
    virtual QString declarationName() const;
    virtual QWidget* createExpandingWidget(const KDevelop::CodeCompletionModel* model) const;
    virtual bool createsExpandingWidget() const;
    virtual QString shortenedTypeString(const KDevelop::DeclarationPointer& decl, int desiredTypeLength) const;

    /**
     * Called after execute, you may insert additional chars for this declaration (eg. parens)
     * Default implementation does nothing
     */
    virtual void executed(KTextEditor::View* view, const KTextEditor::Range& word);

    QExplicitlySharedDataPointer<CodeCompletionContext> m_completionContext;
    KDevelop::DeclarationPointer m_declaration;
    int m_inheritanceDepth; //Inheritance-depth: 0 for local functions(within no class), 1 for within local class, 1000+ for global items.

    static const int normalBestMatchesCount;
    static const bool shortenArgumentHintReturnValues;
    static const int maximumArgumentHintReturnValueLength;
    static const int desiredTypeLength;
};
}
#endif
