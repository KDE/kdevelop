/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef QMLJS_CONTEXT_H
#define QMLJS_CONTEXT_H

#include "codecompletionexport.h"
#include "items/completionitem.h"

#include <language/codecompletion/codecompletioncontext.h>
#include <language/duchain/ducontext.h>

namespace QmlJS {

class KDEVQMLJSCOMPLETION_EXPORT CodeCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    CodeCompletionContext(const KDevelop::DUContextPointer& context, const QString& text,
                          const KDevelop::CursorInRevision& position, int depth = 0);
    QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) override;

    KDevelop::AbstractType::Ptr typeToMatch() const;

    enum CompletionInContextFlag {
        CompletionOnlyLocal = 1,        /*!< @brief Don't list the items available in parent contexts */
        CompletionHideWrappers = 2,     /*!< @brief Filter out QML component wrappers (Q... C++ classes) */
    };
    Q_DECLARE_FLAGS(CompletionInContextFlags, CompletionInContextFlag)

private:
    /**
     * @brief Entry in the expression stack returned by expressionStack()
     *
     * Code-completion usually happens in partial expressions that cannot be
     * parsed using the QML/JS parser. expressionStack() can be used to build
     * a stack of unterminated partial expressions. This structure represents
     * an entry in this stack.
     *
     * Every expression stack entry contains information about where the
     * expression started (at the beginning of the string or just after a brace),
     * how many commas have been encountered in it (so that function call-tips
     * can know which is the current argument being edited), and where is the last
     * operator in the sub-expression. Normally, anything after the last operator
     * should be a valid sub-expression identifying a declaration.
     *
     * @code
     * foo(A, B, C
     * [   [   se
     * @endcode
     *
     * In the above snippet, the inner-most expression starts at A. However,
     * "A, B, C" is not a valid JS expression (or one that should be used for
     * code-completion). However, everything after the last comma, identified
     * by operatorStart and operatorEnd, is valid ("C" is a identifier). "s" is
     * operatorStart, and "e" is operatorEnd.
     */
    struct ExpressionStackEntry {
        int startPosition;
        int operatorStart;
        int operatorEnd;
        int commas;
    };

    QList<KDevelop::CompletionTreeItemPointer> normalCompletion();
    QList<KDevelop::CompletionTreeItemPointer> commentCompletion();
    QList<KDevelop::CompletionTreeItemPointer> importCompletion();
    QList<KDevelop::CompletionTreeItemPointer> nodeModuleCompletions();

    QList<KDevelop::CompletionTreeItemPointer> functionCallTips();
    QList<KDevelop::CompletionTreeItemPointer> completionsFromImports(CompletionInContextFlags flags);
    QList<KDevelop::CompletionTreeItemPointer> completionsFromNodeModule(CompletionInContextFlags flags,
                                                                         const QString& module);
    QList<KDevelop::CompletionTreeItemPointer> completionsInContext(const KDevelop::DUContextPointer& context,
                                                                    CompletionInContextFlags flags,
                                                                    CompletionItem::Decoration decoration);
    QList<KDevelop::CompletionTreeItemPointer> fieldCompletions(const QString &expression,
                                                                CompletionItem::Decoration decoration);

    KDevelop::Stack<ExpressionStackEntry> expressionStack(const QString& expression);    /*!< @see ExpressionStackEntry */
    KDevelop::DeclarationPointer declarationAtEndOfString(const QString& expression);

private:
    enum CompletionKind {
        NoCompletion,           /*!< @brief No code-completion at all */
        NormalCompletion,       /*!< @brief Completion in a code context */
        CommentCompletion,      /*!< @brief Completion in comments */
        StringCompletion,       /*!< @brief Completion in strings */
        ImportCompletion,       /*!< @brief Completion for import statements */
        NodeModulesCompletion   /*!< @brief Completion for node.js module names */
    };

    CompletionKind m_completionKind;
    KDevelop::AbstractType::Ptr m_typeToMatch;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CodeCompletionContext::CompletionInContextFlags)

}

#endif // QMLJS_CONTEXT_H
