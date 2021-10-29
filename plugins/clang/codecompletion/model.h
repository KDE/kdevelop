/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGCODECOMPLETIONMODEL_H
#define CLANGCODECOMPLETIONMODEL_H

#include <language/codecompletion/codecompletionmodel.h>

#include "clangprivateexport.h"

class ClangIndex;

class KDEVCLANGPRIVATE_EXPORT ClangCodeCompletionModel : public KDevelop::CodeCompletionModel
{
    Q_OBJECT

public:
    explicit ClangCodeCompletionModel(ClangIndex* index, QObject* parent);
    ~ClangCodeCompletionModel() override;

    bool shouldStartCompletion(KTextEditor::View* view, const QString& inserted,
                               bool userInsertion, const KTextEditor::Cursor& position) override;

    KTextEditor::Range completionRange(KTextEditor::View* view, const KTextEditor::Cursor& position) override;

    bool shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range& range, const QString& currentCompletion) override;

Q_SIGNALS:
    void requestCompletion(const QUrl &url, const KTextEditor::Cursor& cursor, const QString& text, const QString& followingText);

protected:
    KDevelop::CodeCompletionWorker* createCompletionWorker() override;

    void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                   InvocationType invocationType, const QUrl &url) override;

private:
    ClangIndex* m_index;
};

#endif // CLANGCODECOMPLETIONMODEL_H
