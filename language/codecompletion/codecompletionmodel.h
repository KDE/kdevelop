/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_CODECOMPLETIONMODEL_H
#define KDEVPLATFORM_CODECOMPLETIONMODEL_H

#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QExplicitlySharedDataPointer>
#include <QUrl>

#include "../duchain/duchainpointer.h"
#include <language/languageexport.h>
#include "codecompletioncontext.h"
#include "codecompletionitem.h"
#include <interfaces/icompletionsettings.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

class QIcon;
class QString;
class QMutex;

namespace KDevelop
{
class DUContext;
class Declaration;
class CodeCompletionWorker;
class CompletionWorkerThread;

class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionModel : public KTextEditor::CodeCompletionModel
                                                      , public KTextEditor::CodeCompletionModelControllerInterface
{
  Q_OBJECT
  Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

  public:
    CodeCompletionModel(QObject* parent);
    virtual ~CodeCompletionModel();

    ///This MUST be called after the creation of this completion-model.
    ///If you use use the KDevelop::CodeCompletion helper-class, that one cares about it.
    virtual void initialize();

    ///Entry-point for code-completion. This determines ome settings, clears the model, and then calls completionInvokedInternal for further processing.
    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, KTextEditor::CodeCompletionModel::InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;

    ///Use this to set whether the code-completion widget should wait for this model until it's shown
    ///This makes sense when the model takes some time but not too much time, to make the UI less flickering and
    ///annoying.
    ///The default is false
    ///@todo Remove this option, make it true by default, and make sure in CodeCompletionWorker that the whole thing cannot break
    void setForceWaitForModel(bool wait);

    bool forceWaitForModel();

    ///Convenience-storage for use by the inherited completion model
    void setCompletionContext(QExplicitlySharedDataPointer<CodeCompletionContext> completionContext);
    QExplicitlySharedDataPointer<CodeCompletionContext> completionContext() const;

    ///Convenience-storage for use by the inherited completion model
    KDevelop::TopDUContextPointer currentTopContext() const;
    void setCurrentTopContext(KDevelop::TopDUContextPointer topContext);

    ///Tracks navigation widget so they can be interactive with through the keyboard later on
    void addNavigationWidget(const CompletionTreeElement* element, QWidget* widget) const;

    ///Whether the completion should be fully detailed. If false, it should be simplifed, so no argument-hints,
    ///no expanding information, no type-information, etc.
    bool fullCompletion() const;

    virtual MatchReaction matchingItem(const QModelIndex& matched);

    virtual QString filterString(KTextEditor::View* view, const KTextEditor::Range& range, const KTextEditor::Cursor& position);

    void clear();

    ///Returns the tree-element that belogns to the index, or zero
    QExplicitlySharedDataPointer<CompletionTreeElement> itemForIndex(QModelIndex index) const;

  Q_SIGNALS:
    ///Connection from this completion-model into the background worker thread. You should emit this from within completionInvokedInternal.
    void completionsNeeded(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);
    ///Additional signal that allows directly stepping into the worker-thread, bypassing computeCompletions(..) etc.
    ///doSpecialProcessing(data) will be executed in the background thread.
    void doSpecialProcessingInBackground(uint data);

  protected Q_SLOTS:
    ///Connection from the background-thread into the model: This is called when the background-thread is ready
    virtual void foundDeclarations(QList<QExplicitlySharedDataPointer<CompletionTreeElement> > item, QExplicitlySharedDataPointer<CodeCompletionContext> completionContext);

  protected:
    ///Eventually override this, determine the context or whatever, and then emit completionsNeeded(..) to continue processing in the background tread.
    ///The default-implementation does this completely, so if you don't need to do anything special, you can just leave it.
    virtual void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, KTextEditor::CodeCompletionModel::InvocationType invocationType, const QUrl& url);

    virtual void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& word, const QModelIndex& index) const;

    QExplicitlySharedDataPointer<CodeCompletionContext> m_completionContext;
    typedef QPair<KDevelop::DeclarationPointer, QExplicitlySharedDataPointer<CodeCompletionContext> > DeclarationContextPair;

    mutable QMap<const CompletionTreeElement*, QPointer<QWidget> > m_navigationWidgets;
    QList< QExplicitlySharedDataPointer<CompletionTreeElement> > m_completionItems;

    /// Should create a completion-worker. The worker must have no parent object,
    /// because else its thread-affinity can not be changed.
    virtual CodeCompletionWorker* createCompletionWorker() = 0;
    friend class CompletionWorkerThread;

    CodeCompletionWorker* worker() const;
  private:
    bool m_forceWaitForModel;
    bool m_fullCompletion;
    QMutex* m_mutex;
    CompletionWorkerThread* m_thread;
    QString m_filterString;
    KDevelop::TopDUContextPointer m_currentTopContext;
};

}

#endif

