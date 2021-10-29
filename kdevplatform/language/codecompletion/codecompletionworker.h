/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODECOMPLETIONWORKER_H
#define KDEVPLATFORM_CODECOMPLETIONWORKER_H

#include <QList>

#include <language/languageexport.h>
#include "../duchain/duchainpointer.h"
#include "../codecompletion/codecompletioncontext.h"

class QMutex;

namespace KTextEditor {
class Range;
class View;
class Cursor;
}

namespace KDevelop {
class CompletionTreeElement;
class CodeCompletionModel;

class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionWorker
    : public QObject
{
    Q_OBJECT

public:
    explicit CodeCompletionWorker(CodeCompletionModel* model);
    ~CodeCompletionWorker() override;

    virtual void abortCurrentCompletion();

    void setFullCompletion(bool);
    bool fullCompletion() const;

    KDevelop::CodeCompletionModel* model() const;

    ///When this is called, the result is shown in the completion-list.
    ///Call this from within your code
    void foundDeclarations(const QList<QExplicitlySharedDataPointer<CompletionTreeElement>>&,
                           const CodeCompletionContext::Ptr& completionContext);

Q_SIGNALS:

    ///Internal connections into the foreground completion model
    void foundDeclarationsReal(const QList<QExplicitlySharedDataPointer<CompletionTreeElement>>&,
                               const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext);

protected:

    virtual void computeCompletions(const DUContextPointer& context, const KTextEditor::Cursor& position,
                                    const QString& followingText, const KTextEditor::Range& contextRange,
                                    const QString& contextText);
    ///This can be overridden to compute an own grouping in the completion-list.
    ///The default implementation groups items in a way that improves the efficiency of the completion-model, thus the default-implementation should be preferred.
    virtual QList<QExplicitlySharedDataPointer<CompletionTreeElement>> computeGroups(
        const QList<CompletionTreeItemPointer>& items,
        const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext);
    ///If you don't need to reimplement computeCompletions, you can implement only this.
    virtual KDevelop::CodeCompletionContext* createCompletionContext(const KDevelop::DUContextPointer& context,
                                                                     const QString& contextText,
                                                                     const QString& followingText,
                                                                     const CursorInRevision& position) const;

    ///Override this to change the text-range which is used as context-information for the completion context
    ///The foreground-lock and a DUChain read lock are held when this is called
    virtual void updateContextRange(KTextEditor::Range& contextRange, KTextEditor::View* view,
                                    const DUContextPointer& context) const;

    ///Can be used to retrieve and set the aborting flag(Enabling it is equivalent to calling abortCompletion())
    ///Is always reset from within computeCompletions
    bool& aborting();

    ///Emits foundDeclarations() with an empty list. Always call this when you abort the process of computing completions
    void failed();

public Q_SLOTS:
    ///Connection from the foreground thread within CodeCompletionModel
    void computeCompletions(const KDevelop::DUContextPointer& context, const KTextEditor::Cursor& position,
                            KTextEditor::View* view);
    ///This can be used to do special processing within the background, completely bypassing the normal computeCompletions(..) etc. system.
    ///It will be executed within the background when the model emits doSpecialProcessingInBackground
    virtual void doSpecialProcessing(uint data);

private:
    bool m_hasFoundDeclarations;
    QMutex* m_mutex;
    bool m_abort;
    bool m_fullCompletion;
    KDevelop::CodeCompletionModel* m_model;
};
}

#endif // KDEVPLATFORM_CODECOMPLETIONWORKER_H
