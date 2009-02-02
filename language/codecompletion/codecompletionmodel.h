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

#ifndef KDEV_CODECOMPLETIONMODEL_H
#define KDEV_CODECOMPLETIONMODEL_H

#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <KDE/KTextEditor/CodeCompletionModel>
#include <KDE/KSharedPtr>

#include "../duchain/duchainpointer.h"
#include "../languageexport.h"
#include "codecompletioncontext.h"
#include "codecompletionitem.h"
#include <interfaces/icompletionsettings.h>

class QIcon;
class QString;
class QMutex;

namespace KDevelop
{
class DUContext;
class Declaration;
class CodeCompletionWorker;
class CompletionWorkerThread;

class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionModel : public KTextEditor::CodeCompletionModel2
{
  Q_OBJECT

  public:
    CodeCompletionModel(QObject* parent);
    virtual ~CodeCompletionModel();
    
    ///This MUST be called after the creation of this completion-model.
    ///If you use use the KDevelop::CodeCompletion helper-class, that one cares about it.
    virtual void initialize();
    
    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
  
    void setCompletionContext(KSharedPtr<CodeCompletionContext> completionContext);
    KSharedPtr<CodeCompletionContext> completionContext() const;

    KDevelop::TopDUContextPointer currentTopContext() const;
    void setCurrentTopContext(KDevelop::TopDUContextPointer topContext);

    //Tracks navigation widget so they can be interactive with through the keyboard later on
    void addNavigationWidget(const CompletionTreeElement* element, QWidget* widget) const;

    ///Whether the completion should be fully detailed. If false, it should be simplifed, so no argument-hints,
    ///no expanding information, no type-information, etc.
    bool fullCompletion() const;
    
  Q_SIGNALS:
    void completionsNeeded(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private Q_SLOTS:
    void foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > item, KSharedPtr<CodeCompletionContext> completionContext);
    
  protected:
    virtual void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType, const KUrl& url) = 0;

    virtual void executeCompletionItem2(KTextEditor::Document* document, const KTextEditor::Range& word, const QModelIndex& index) const;
    KSharedPtr<CodeCompletionContext> m_completionContext;
    typedef QPair<KDevelop::DeclarationPointer, KSharedPtr<CodeCompletionContext> > DeclarationContextPair;

    mutable QMap<const CompletionTreeElement*, QPointer<QWidget> > m_navigationWidgets;
    QList< KSharedPtr<CompletionTreeElement> > m_completionItems;

    //Should create a completion-worker. The worker must be created right within this function, so it is assigned to the correct rhread.
    virtual CodeCompletionWorker* createCompletionWorker() = 0;
    friend class CompletionWorkerThread;
    
    CodeCompletionWorker* worker();
  private:
    bool m_fullCompletion;
    QMutex* m_mutex;
    CompletionWorkerThread* m_thread;
    KDevelop::TopDUContextPointer m_currentTopContext;
};

}

#endif

