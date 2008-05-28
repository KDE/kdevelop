/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVCPPCODECOMPLETIONMODEL_H
#define KDEVCPPCODECOMPLETIONMODEL_H

#include <QPair>
#include <QMap>
#include <QPointer>
#include <ktexteditor/codecompletionmodel.h>
#include <ksharedptr.h>
#include <duchainpointer.h>
#include "codecompletioncontext.h"
#include "includeitem.h"
#include "completionitem.h"

class QIcon;
class QString;
class QMutex;

namespace KDevelop
{
class DUContext;
class Declaration;
}

namespace Cpp {
  class CodeCompletionContext;
  class NavigationWidget;
}

class CodeCompletionWorker;

class CompletionTreeElement;

class CppCodeCompletionModel : public KTextEditor::CodeCompletionModel2
{
  Q_OBJECT

  public:
    CppCodeCompletionModel(QObject* parent);
    virtual ~CppCodeCompletionModel();

    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    ///getData(..) for include-file completion
    QVariant getIncludeData(const QModelIndex& index, int role) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
  
    void setCompletionContext(KSharedPtr<Cpp::CodeCompletionContext> completionContext);
    KSharedPtr<Cpp::CodeCompletionContext> completionContext() const;

    KDevelop::TopDUContextPointer currentTopContext() const;

    //Tracks navigation widget so they can be interactive with through the keyboard later on
    void addNavigationWidget(const CompletionTreeElement* element, Cpp::NavigationWidget* widget) const;

  Q_SIGNALS:
    void completionsNeeded(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private Q_SLOTS:
    void foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > item, void* completionContext);
    
  private:
    virtual void executeCompletionItem2(KTextEditor::Document* document, const KTextEditor::Range& word, const QModelIndex& index) const;
    KSharedPtr<Cpp::CodeCompletionContext> m_completionContext;
    typedef QPair<KDevelop::DeclarationPointer, KSharedPtr<Cpp::CodeCompletionContext> > DeclarationContextPair;

    mutable QMap<const CompletionTreeElement*, QPointer<Cpp::NavigationWidget> > m_navigationWidgets;
    QList< KSharedPtr<CompletionTreeElement> > m_completionItems;

    QMutex* m_mutex;
    CodeCompletionWorker* m_worker;
    KDevelop::TopDUContextPointer m_currentTopContext;
};



#endif

